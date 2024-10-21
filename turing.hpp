#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include <ansi.hpp>
#include <euler/it.hpp>

// trim from start (in place)
inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

namespace turing
{
/// Turing state background color, according to bbchallenge.org (but a bit darker).
inline std::string getBgStyle(char state)
{
    switch (state)
    {
    case 'A':
        return ansi::bg(128, 0, 0);
    case 'B':
        return ansi::bg(128, 96, 0) + ansi::str(ansi::black);
    case 'C':
        return ansi::bg(32, 64, 255);
    case 'D':
        return ansi::bg(0, 128, 0);
    case 'E':
        return ansi::bg(128, 0, 128);
    case 'F':
        return ansi::bg(0, 128, 128);
    case 'Z':
        return ansi::str(ansi::invert);
    default:
        return "";
    }
}

/// Left or right.
enum class direction : int8_t
{
    left,
    right
};

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o, direction d)
{
    return o << (d == direction::right ? 'R' : 'L');
}

/// A Turing tape with up to 256 symbols, along with a head.
class Tape
{
  public:
    using symbol = uint8_t;
    using container_type = std::vector<symbol>;
    static constexpr size_t defaultPrintWidth = 50;
    static constexpr char zeroChar = ' ';

    symbol &operator*() { return _data[_head + _offset]; }
    constexpr symbol operator*() const { return _data[_head + _offset]; }

    constexpr symbol operator[](ptrdiff_t i) const
    {
        auto j = _head + _offset + i;
        return j >= 0 && (size_t)j < _data.size() ? _data[j] : 0;
    }

    [[nodiscard]] const container_type &data() const { return _data; }
    /// Returns the absolute position of the head.
    [[nodiscard]] constexpr int64_t head() const { return _head; }
    [[nodiscard]] constexpr int64_t offset() const { return _offset; }

    [[nodiscard]] constexpr size_t size() const { return _data.size(); }

    /// Returns whether the tape consists of all zeros.
    [[nodiscard]] constexpr bool blank() const
    {
        return _data[0] == 0 && std::ranges::equal(std::ranges::subrange(_data.begin(), _data.end() - 1),
                                                   std::ranges::subrange(_data.begin() + 1, _data.end()));
    }

    constexpr void step(symbol x, direction d)
    {
        **this = x;
        if (d == direction::left)
            moveLeft();
        else
            moveRight();
    }

    /// This variant leaves the symbol unchanged.
    constexpr void step(direction dir) { step(**this, dir); }

    /// Returns a string representation of this tape.
    [[nodiscard]] constexpr std::string str(size_t width = defaultPrintWidth, std::string_view headPrefix = ">",
                                            std::string_view headSuffix = "") const
    {
        std::string s;
        int64_t start = (int64_t)_head - (int64_t)(_head % width);
        for (int64_t i = start; i < (int64_t)(start + width); ++i)
        {
            if (i == _head)
                s += headPrefix;
            int64_t j = i + _offset;
            s += (j >= 0 && j < (int64_t)_data.size() && _data[j] != 0 ? (char)('0' + _data[j]) : zeroChar);
            if (i == _head)
                s += headSuffix;
        }
        return s;
    }

    /// Returns a string representation of this tape, by encoding runs of 1s. For example, 1_11_111 is 123, and 1__1
    /// is 101.
    [[nodiscard]] constexpr std::string str1(size_t width = defaultPrintWidth, std::string_view headPrefix = ">",
                                             std::string_view headSuffix = "") const
    {
        std::string s1;
        std::string sHead;
        std::string s2;
        size_t c = 0;
        auto start = std::find_if(_data.begin(), _data.end(), [](auto x) { return x != 0; });
        if (start == _data.end())
            return std::string(headPrefix) + std::string(headSuffix);
        if (_data.begin() + _head < start)
            start = _data.begin() + _head;
        auto end = std::find_if(_data.rbegin(), _data.rend(), [](auto x) { return x != 0; }).base();
        if (_data.begin() + _head >= end)
            end = _data.begin() + _head + 1;
        for (auto it = start; it != end; ++it)
        {
            if (it == _data.begin() + _head)
            {
                if (c > 0)
                    s1 += toStringHelper(c);
                c = 0;
                sHead += headPrefix;
                sHead += **this == 0 ? zeroChar : (char)('0' + **this);
                sHead += headSuffix;
            }
            else if (*it == 0)
            {
                (it < _data.begin() + _head ? s1 : s2) += toStringHelper(c);
                c = 0;
            }
            else
                ++c;
        }
        if (c > 0)
            s2 += toStringHelper(c);
        return std::string(std::max(0, ((int)width - 1) / 2 - (int)s1.size()), ' ') + s1 + sHead + s2 +
               std::string(std::max(0, (int)width / 2 - (int)s2.size()), ' ');
    }

    /// Returns a string representation of this tape, in run length encoding.
    [[nodiscard]] constexpr std::string str2(size_t /*width*/ = defaultPrintWidth, std::string_view headPrefix = ">",
                                             std::string_view headSuffix = "") const
    {
        std::string s;
        auto curr = (uint8_t)-1;
        size_t c = 0;
        auto start = std::find_if(_data.begin(), _data.end(), [](auto x) { return x != 0; });
        if (start == _data.end())
            return std::string(headPrefix) + std::string(headSuffix);
        if (_data.begin() + _head < start)
            start = _data.begin() + _head;
        auto end = std::find_if(_data.rbegin(), _data.rend(), [](auto x) { return x != 0; }).base();
        if (_data.begin() + _head >= end)
            end = _data.begin() + _head + 1;
        for (auto it = start; it != end; ++it)
        {
            if (it == _data.begin() + _head)
            {
                if (c > 0)
                    s += toStringRLE(curr, c);
                curr = (uint8_t)-1;
                c = 0;
                s += headPrefix;
                s += (char)('0' + **this);
                s += headSuffix;
                s += ' ';
            }
            else if (*it == curr)
                ++c;
            else
            {
                if (c > 0)
                    s += toStringRLE(curr, c);
                curr = *it;
                c = 1;
            }
        }
        if (c > 0)
            s += toStringRLE(curr, c);
        return s;
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o, const Tape &t)
    {
        return o << t.str();
    }

  private:
    container_type _data{0};
    int64_t _head = _data.size() - 1;
    int64_t _offset = 0;

    constexpr void moveLeft()
    {
        --_head;
        if (_head + _offset < 0)
        {
            _data.insert(_data.begin(), 0);
            ++_offset;
        }
    }

    constexpr void moveRight()
    {
        ++_head;
        if (_head + _offset >= (int64_t)_data.size())
            _data.push_back(0);
    }

    static std::string toStringHelper(size_t c)
    {
        if (c == 0)
            return {zeroChar};
        if (c < 10)
            return std::to_string(c);
        return "[" + std::to_string(c) + "]";
    }

    static std::string toStringRLE(uint8_t x, size_t c)
    {
        if (x == (uint8_t)-1)
            return "";
        std::string s{1, (char)('0' + x)};
        return s + "^" + std::to_string(c) + " ";
    }
};

/// A Turing machine.
class TuringMachine
{
  public:
    using rule_type = vector2d<std::tuple<uint8_t, direction, char>>;

    constexpr TuringMachine(rule_type rule) : _rule(std::move(rule)) {}

    /// Initializes a Turing machine from a code in TNF format.
    TuringMachine(std::string_view code)
    {
        std::string s{code};
        ltrim(s);
        rtrim(s);
        auto v = it::split(s, '_')
                     .map([](auto &&s) {
                         return it::range(0, s.size() - 1, 3)
                             .map([&](auto &&i) {
                                 auto triple = s.substr(i, 3);
                                 if (triple == "---")
                                     triple = "1RZ";
                                 return std::tuple{uint8_t(triple[0] - '0'),
                                                   triple[1] == 'R' ? direction::right : direction::left, triple[2]};
                             })
                             .to();
                     })
                     .to();
        _rule.resize(v.size(), v[0].size());
        for (size_t i = 0; i < v.size(); ++i)
            for (size_t j = 0; j < v[i].size(); ++j)
                _rule[i, j] = v[i][j];
    }

    [[nodiscard]] constexpr size_t numStates() const { return _rule.rows(); }
    [[nodiscard]] constexpr size_t numColors() const { return _rule.columns(); }

    [[nodiscard]] constexpr const rule_type &rule() const { return _rule; }
    [[nodiscard]] constexpr const Tape &tape() const { return _tape; }
    [[nodiscard]] constexpr size_t steps() const { return _steps; }
    [[nodiscard]] constexpr char state() const { return _state; }

    /// Returns whether the Turing machine is halted, i.e. in the Z state.
    [[nodiscard]] constexpr bool halted() const { return _state < 'A' || (size_t)(_state - 'A') >= numStates(); }
    [[nodiscard]] constexpr bool blank() const { return _tape.blank(); }

    [[nodiscard]] constexpr int64_t head() const { return _tape.head(); }
    [[nodiscard]] constexpr int64_t offset() const { return _tape.offset(); }

    [[nodiscard]] std::string str(bool terminal = false, size_t width = Tape::defaultPrintWidth) const
    {
        return _tape.str(width, terminal ? getBgStyle(_state) : _state + std::string(">"),
                         terminal ? ansi::str(ansi::reset) : "");
    }

    /// Returns a string representation of this Turing machine, by encoding runs of 1s. For example, 1_11_111 is 123,
    /// and 1__1 is 101.
    [[nodiscard]] std::string str1(bool terminal = false, size_t width = Tape::defaultPrintWidth) const
    {
        return _tape.str1(width, terminal ? getBgStyle(_state) : _state + std::string(">"),
                          terminal ? ansi::str(ansi::reset) : "");
    }

    /// Returns a string representation of this Turing machine, in run length encoding.
    [[nodiscard]] std::string str2(bool terminal = false, size_t width = Tape::defaultPrintWidth) const
    {
        return _tape.str2(width, terminal ? getBgStyle(_state) : _state + std::string(">"),
                          terminal ? ansi::str(ansi::reset) : "");
    }

    /// Steps, and returns true if the machine advanced, false otherwise (for example, it was already halted).
    bool step()
    {
        if (halted())
            return false;
        auto &&[b, dir, s] = _rule[_state - 'A', *_tape];
        _tape.step(b, dir);
        _state = s;
        ++_steps;
        return true;
    }

    void reset()
    {
        _tape = {};
        _state = 'A';
        _steps = 0;
    }

  private:
    rule_type _rule;
    Tape _tape;
    size_t _steps = 0;
    char _state = 'A';
};

inline bool spansEqual(const Tape &t1, const Tape &t2, int64_t start, int64_t end)
{
    for (int64_t i = start; i <= end; ++i)
        if (t1[i] != t2[i])
            return false;
    return true;
}

inline int64_t findTranslatedCyclerPeriod(TuringMachine machine, size_t initialSteps = 100, size_t maxPeriod = 100)
{
    for (size_t i = 0; i < initialSteps; ++i)
    {
        machine.step();
        if (machine.halted())
            return {};
    }
    auto startTape = machine.tape();
    auto startState = machine.state();
    int64_t startHead = startTape.head();
    int64_t ld = 0;
    int64_t hd = 0;
    for (size_t p = 1; p <= maxPeriod; ++p)
    {
        machine.step();
        if (machine.halted())
            return {};
        if (machine.state() == startState)
        {
            auto l = ld;
            auto h = hd;
            if (machine.tape().head() < startHead)
                l = std::min(-startTape.offset() - startHead, -machine.tape().offset() - machine.tape().head());
            else if (machine.tape().head() > startHead)
                h = std::max(startTape.data().size() - startHead - startTape.offset(),
                             machine.tape().data().size() - machine.tape().head() - machine.tape().offset());
            if (spansEqual(startTape, machine.tape(), l, h))
                return p;
        }
        ld = std::min(ld, machine.tape().head() - startHead);
        hd = std::max(hd, machine.tape().head() - startHead);
    }
    return {};
}

namespace known
{
inline TuringMachine bb2Champion() { return {"1RB1LB_1LA1RZ"}; }
inline TuringMachine bb3Champion() { return {"1RB1RZ_1LB0RC_1LC1LA"}; }
inline TuringMachine bb4Champion() { return {"1RB1LB_1LA0LC_1RZ1LD_1RD0RA"}; }
/// BB(5) champion. `1RB1LC_1RC1RB_1RD0LE_1LA1LD_1RZ0LA`.
inline TuringMachine bb5Champion() { return {"1RB1LC_1RC1RB_1RD0LE_1LA1LD_1RZ0LA"}; }
/// BB(6) current champion. D is the rarest state by far. `1RB0LD_1RC0RF_1LC1LA_0LE1RZ_1LF0RB_0RC0RE`.
inline TuringMachine bb6Champion() { return {"1RB0LD_1RC0RF_1LC1LA_0LE1RZ_1LF0RB_0RC0RE"}; }
/// 4-state 2-color Blanking Beaver runner-up, 66345 steps. `1RB0LC_1LD0LA_1RC1RD_1LA0LD`.
inline TuringMachine bbb4RunnerUp() { return {"1RB0LC_1LD0LA_1RC1RD_1LA0LD"}; }
/// BBB4 champion. `1RB1LC_1RD1RB_0RD0RC_1LD1LA`.
inline TuringMachine bbb4Champion() { return {"1RB1LC_1RD1RB_0RD0RC_1LD1LA"}; }
/// BB(2, 3) champion. `1RB2LB1RZ_2LA2RB1LB`. 38 steps.
inline TuringMachine bb23Champion() { return {"1RB2LB1RZ_2LA2RB1LB"}; }
/// BB(3, 3) current champion. `0RB2LA1RA_1LA2RB1RC_1RZ1LB1LC`. 119,112,334,170,342,541 steps.
inline TuringMachine bb33Champion() { return {"0RB2LA1RA_1LA2RB1RC_1RZ1LB1LC"}; }
/// BB(3, 3) 8th top halter. `1RB2LA1RA_1LB1LA2RC_1RZ1LC2RB`. 1808669046 steps.
inline TuringMachine bb33_8th() { return {"1RB2LA1RA_1LB1LA2RC_1RZ1LC2RB"}; }
/// Lin recurrence, living nightmare. 158491 preperiod, 17620 period. `1RB0RC_1LB1LD_0RA0LD_1LA1RC`.
inline TuringMachine boydJohnson() { return {"1RB0RC_1LB1LD_0RA0LD_1LA1RC"}; }
/// Lin recurrence, living nightmare. 7170 preperiod, 29117 period. `1RB0RA_1RC0RB_1LD1LC_1RA0LC`.
inline TuringMachine boydJohnson2() { return {"1RB0RA_1RC0RB_1LD1LC_1RA0LC"}; }
/// Lin recurrence, living nightmare. 28812 preperiod, 5588 period. `1RB1RA_0RC0LB_0RD0RA_1LD0LA`.
inline TuringMachine boydJohnson3() { return {"1RB1RA_0RC0LB_0RD0RA_1LD0LA"}; }
/// Probviously nonhalting BB(6) Collatz-like Cryptid.
inline TuringMachine antihydra() { return {"1RB1RA_0LC1LE_1LD1LC_1LA0LB_1LF1RE_---0RA"}; }
/// Non-halting. State B reveals powers of 2 pattern. Not classified yet by bbchallenge.org.
inline TuringMachine id4446642() { return {"1RB0RC_0LC---_1RD1RC_0LE1RA_1RD1LE"}; }
} // namespace known
} // namespace turing
