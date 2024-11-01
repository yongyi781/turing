#pragma once

#include <algorithm>
#include <deque>
#include <euler/algorithm.hpp>
#include <string>
#include <vector>

#include <ansi.hpp>
#include <euler/it.hpp>

// trim from start (in place)
inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](char ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](char ch) { return !std::isspace(ch); }).base(), s.end());
}

namespace turing
{
using symbol_type = uint8_t;
using state_type = int8_t;

/// Turing state background color, according to bbchallenge.org (but a bit darker).
inline std::string getBgStyle(state_type state)
{
    switch (state)
    {
    case 0:
        return ansi::bg(128, 0, 0);
    case 1:
        return ansi::bg(128, 96, 0) + ansi::str(ansi::black);
    case 2:
        return ansi::bg(32, 64, 255);
    case 3:
        return ansi::bg(0, 128, 0);
    case 4:
        return ansi::bg(128, 0, 128);
    case 5:
        return ansi::bg(0, 128, 128);
    case -1:
        return ansi::str(ansi::invert);
    default:
        return "";
    }
}

/// Left or right.
enum class direction : uint8_t
{
    left,
    right
};

/// A Turing tape with up to 256 symbols, along with a head.
class Tape
{
  public:
    using container_type = std::vector<symbol_type>;
    static constexpr size_t defaultPrintWidth = 50;
    static constexpr char zeroChar = ' ';

    symbol_type &operator*() { return _data[_head + _offset]; }
    constexpr symbol_type operator*() const { return _data[_head + _offset]; }

    /// Gets the symbol at the given absolute position (zero being the initial position).
    constexpr symbol_type operator[](ptrdiff_t i) const
    {
        auto j = i + _offset;
        return j >= 0 && (size_t)j < _data.size() ? _data[j] : 0;
    }

    [[nodiscard]] const container_type &data() const { return _data; }
    /// Returns the absolute position of the head.
    [[nodiscard]] constexpr int64_t head() const { return _head; }
    [[nodiscard]] constexpr int64_t offset() const { return _offset; }

    /// The left edge of the tape.
    [[nodiscard]] constexpr int64_t leftEdge() const { return _leftEdge; }
    // The right edge of the tape.
    [[nodiscard]] constexpr int64_t rightEdge() const { return _data.size() - _offset - 1; }
    /// The size of the tape.
    [[nodiscard]] constexpr size_t size() const { return rightEdge() - leftEdge() + 1; }

    /// Returns whether the tape consists of all zeros.
    [[nodiscard]] constexpr bool blank() const
    {
        return _data[0] == 0 && std::ranges::equal(std::ranges::subrange(_data.begin(), _data.end() - 1),
                                                   std::ranges::subrange(_data.begin() + 1, _data.end()));
    }

    constexpr void step(symbol_type x, direction d)
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
        int64_t shift = width / 2;
        int64_t start = width * floorDiv((int64_t)(_head + shift), (int64_t)width) - shift;
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
    [[nodiscard]] std::string str1(size_t width = defaultPrintWidth, std::string_view headPrefix = ">",
                                   std::string_view headSuffix = "") const
    {
        std::string s1;
        std::string sHead;
        std::string s2;
        size_t c = 0;
        auto start = std::find_if(_data.begin(), _data.end(), [](auto x) { return x != 0; });
        if (start == _data.end())
            return std::string(headPrefix) + std::string(headSuffix);
        if (_data.begin() + _head + _offset < start)
            start = _data.begin() + _head + _offset;
        auto end = std::find_if(_data.rbegin(), _data.rend(), [](auto x) { return x != 0; }).base();
        if (_data.begin() + _head + _offset >= end)
            end = _data.begin() + _head + _offset + 1;
        for (auto it = start; it != end; ++it)
            if (it == _data.begin() + _head + _offset)
            {
                s1 += toStringHelper(c);
                c = 0;
                sHead += headPrefix;
                sHead += **this == 0 ? zeroChar : (char)('0' + **this);
                sHead += headSuffix;
            }
            else if (*it == 0)
            {
                (it < _data.begin() + _head + _offset ? s1 : s2) += toStringHelper(c);
                c = 0;
            }
            else
                ++c;
        if (c > 0)
            s2 += toStringHelper(c);
        return std::string(std::max(0, ((int)width - 1) / 2 - (int)s1.size()), ' ') + s1 + sHead + s2 +
               std::string(std::max(0, (int)width / 2 - (int)s2.size()), ' ');
    }

    /// Returns a string representation of this tape, in run length encoding.
    [[nodiscard]] std::string str2(size_t /*width*/ = defaultPrintWidth, std::string_view headPrefix = ">",
                                   std::string_view headSuffix = "") const
    {
        std::string s;
        auto curr = (symbol_type)-1;
        size_t c = 0;
        auto start = std::find_if(_data.begin(), _data.end(), [](auto x) { return x != 0; });
        if (start == _data.end())
            return std::string(headPrefix) + std::string(headSuffix);
        if (_data.begin() + _head + _offset < start)
            start = _data.begin() + _head + _offset;
        auto end = std::find_if(_data.rbegin(), _data.rend(), [](auto x) { return x != 0; }).base();
        if (_data.begin() + _head + _offset >= end)
            end = _data.begin() + _head + _offset + 1;
        for (auto it = start; it != end; ++it)
        {
            if (it == _data.begin() + _head + _offset)
            {
                if (c > 0)
                    s += toStringRLE(curr, c);
                curr = (symbol_type)-1;
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
    int64_t _leftEdge = 0;

    constexpr void moveLeft()
    {
        --_head;
        if (_head < _leftEdge)
            --_leftEdge;
        if (_head + _offset < 0)
        {
            size_t n = _data.size();
            _data.insert(_data.begin(), n, 0);
            _offset += n;
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

    static std::string toStringRLE(symbol_type x, size_t c)
    {
        if (x == (symbol_type)-1)
            return "";
        std::string s{1, (char)('0' + x)};
        return s + "^" + std::to_string(c) + " ";
    }
};

/// A Turing machine transition.
struct transition
{
    symbol_type symbol = 0;
    direction direction = turing::direction::left;
    state_type nextState = -1;
};

using turing_rule = vector2d<transition>;

/// Returns a string representation of the given Turing machine rule.
inline std::string to_string(const turing_rule &rule)
{
    std::string s;
    for (size_t i = 0; i < rule.rows(); ++i)
    {
        if (i != 0)
            s += "_";
        for (size_t j = 0; j < rule.columns(); ++j)
        {
            auto &&[symbol, dir, state] = rule[i, j];
            if (state == (state_type)-1)
                s += "---";
            else
            {
                s += (char)('0' + symbol);
                s += dir == turing::direction::left ? 'L' : 'R';
                s += (char)(state + 'A');
            }
        }
    }
    return s;
}

/// Doesn't work yet, but good enough for 4x2. Precondition: all defined states are reachable.
inline turing_rule lexicalNormalForm(const turing_rule &rule)
{
    if (rule.rows() <= 3) // nothing to do.
        return rule;
    // Don't worry about symbols yet
    auto statePerm = range((state_type)0, (state_type)(rule.rows() - 1));
    auto highestUsedState = rule[0, 0].nextState;
    for (state_type i = 0; i < (state_type)rule.rows(); ++i)
    {
        for (symbol_type j = 0; j < (symbol_type)rule.columns(); ++j)
        {
            if (i == 0 && j == 0)
                continue;
            auto k = rule[i, j].nextState;
            if (k < 0 || k >= (int)rule.rows())
                continue;
            if (k > highestUsedState + 1)
                std::swap(statePerm[k], statePerm[highestUsedState + 1]);
            highestUsedState = std::max(highestUsedState, k);
        }
    }
    turing_rule res(rule.rows(), rule.columns());
    for (state_type i = 0; i < (state_type)rule.rows(); ++i)
    {
        for (symbol_type j = 0; j < (symbol_type)rule.columns(); ++j)
        {
            auto &&[symbol, dir, state] = rule[i, j];
            if (state < 0 || state >= (int)rule.rows())
                continue;
            res[statePerm[i], j] = {symbol, dir, statePerm[state]};
        }
    }
    return res;
}

/// A Turing machine.
class TuringMachine
{
  public:
    using rule_type = turing_rule;

    struct step_info
    {
        // False if the machine was already in a halt state.
        bool success;
        // True if the tape grew in size as a result of the step.
        bool tapeExpanded;
    };

    TuringMachine() = default;

    constexpr TuringMachine(turing_rule rule, Tape tape = {}, state_type state = 0, size_t steps = 0)
        : _rule(std::move(rule)), _tape(std::move(tape)), _state(state), _steps(steps)
    {
    }

    /// Initializes a Turing machine from a code in TNF format.
    TuringMachine(std::string code)
    {
        ltrim(code);
        rtrim(code);
        auto v = it::split(std::move(code), '_')
                     .map([](auto &&x) {
                         return it::range(0, x.size() - 1, 3)
                             .map([&](auto &&i) {
                                 auto triple = x.substr(i, 3);
                                 if (triple == "---")
                                     triple = "1RZ";
                                 return transition{(symbol_type)(triple[0] - '0'),
                                                   triple[1] == 'R' ? direction::right : direction::left,
                                                   (state_type)(triple[2] - 'A')};
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
    [[nodiscard]] constexpr std::string ruleStr() const { return to_string(_rule); }
    [[nodiscard]] constexpr const Tape &tape() const { return _tape; }
    constexpr void tape(Tape newTape) { _tape = std::move(newTape); }
    [[nodiscard]] constexpr size_t steps() const { return _steps; }
    void steps(size_t newSteps) { _steps = newSteps; }
    [[nodiscard]] constexpr state_type state() const { return _state; }
    void state(state_type newState) { _state = newState; }

    /// Returns whether the Turing machine is halted, i.e. in the Z state.
    [[nodiscard]] constexpr bool halted() const { return _state < 0 || (size_t)_state >= numStates(); }
    [[nodiscard]] constexpr bool blank() const { return _tape.blank(); }

    [[nodiscard]] constexpr int64_t head() const { return _tape.head(); }
    [[nodiscard]] constexpr int64_t offset() const { return _tape.offset(); }

    /// Gets the transition that this machine will execute next.
    [[nodiscard]] constexpr transition peek() const { return _rule[_state, *_tape]; }

    /// Steps, and returns true if the machine advanced, false otherwise (for example, it was already halted).
    step_info step()
    {
        if (halted())
            return {false, false};
        auto &&[b, dir, s] = peek();
        auto sz = _tape.size();
        _tape.step(b, dir);
        _state = s;
        ++_steps;
        return {true, _tape.size() != sz};
    }

    void reset()
    {
        _tape = {};
        _state = 0;
        _steps = 0;
    }

    [[nodiscard]] std::string str(bool terminal = false, size_t width = Tape::defaultPrintWidth) const
    {
        return _tape.str(width, terminal ? getBgStyle(_state) : (char)(_state + 'A') + std::string(">"),
                         terminal ? ansi::str(ansi::reset) : "");
    }

    /// Returns a string representation of this Turing machine, by encoding runs of 1s. For example, 1_11_111 is 123,
    /// and 1__1 is 101.
    [[nodiscard]] std::string str1(bool terminal = false, size_t width = Tape::defaultPrintWidth) const
    {
        return _tape.str1(width, terminal ? getBgStyle(_state) : (char)(_state + 'A') + std::string(">"),
                          terminal ? ansi::str(ansi::reset) : "");
    }

    /// Returns a string representation of this Turing machine, in run length encoding.
    [[nodiscard]] std::string str2(bool terminal = false, size_t width = Tape::defaultPrintWidth) const
    {
        return _tape.str2(width, terminal ? getBgStyle(_state) : (char)(_state + 'A') + std::string(">"),
                          terminal ? ansi::str(ansi::reset) : "");
    }

  private:
    rule_type _rule;
    Tape _tape;
    state_type _state = 0;
    size_t _steps = 0;
};

struct tape_segment
{
    std::vector<symbol_type> data;
    /// Relative head position.
    int64_t head;
    state_type state;

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o,
                                                         const turing::tape_segment &ts)
    {
        if (ts.head == -1)
            o << turing::getBgStyle(ts.state) << ' ' << ansi::reset;
        for (size_t i = 0; i < ts.data.size(); ++i)
            if (ts.head == (int)i)
                o << turing::getBgStyle(ts.state) << (char)('0' + ts.data[i]) << ansi::reset;
            else
                o << (char)('0' + ts.data[i]);
        if (ts.head == (int)ts.data.size())
            o << turing::getBgStyle(ts.state) << ' ' << ansi::reset;
        return o;
    }
};

constexpr bool operator==(const tape_segment &a, const tape_segment &b)
{
    return a.data == b.data && a.head == b.head && a.state == b.state;
}

struct packed_transition
{
    tape_segment from;
    tape_segment to;
    size_t steps;

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o,
                                                         const turing::packed_transition &ts)
    {
        return o << ts.from << " → " << ts.to << " (" << ts.steps << ")";
    }
};

constexpr bool operator==(const packed_transition &a, const packed_transition &b)
{
    return a.from == b.from && a.to == b.to;
}

/// Gets the tape segment, inclusive.
inline tape_segment getTapeSegment(const Tape &tape, state_type state, int64_t start, int64_t stop)
{
    std::vector<symbol_type> v;
    for (int64_t i = start; i <= stop; ++i)
        v.push_back(tape[i]);
    return {v, tape.head() - start, state};
}

/// Returns whether the given spans of t1 and t2, relative to their head positions, are identical.
inline bool spansEqual(const Tape &t1, const Tape &t2, int64_t start, int64_t end)
{
    for (int64_t i = start; i <= end; ++i)
        if (t1[t1.head() + i] != t2[t2.head() + i])
            return false;
    return true;
}

/// Finds the period of the given Turing machine.
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

/// Hash
constexpr size_t hash_value(const turing::tape_segment &t)
{
    size_t seed = 0;
    boost::hash_combine(seed, t.data);
    boost::hash_combine(seed, t.head);
    boost::hash_combine(seed, t.state);
    return seed;
}

/// Hash
constexpr size_t hash_value(const turing::packed_transition &t)
{
    size_t seed = 0;
    boost::hash_combine(seed, t.from);
    boost::hash_combine(seed, t.to);
    return seed;
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

template <typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o, turing::direction d)
{
    return o << (d == turing::direction::right ? 'R' : 'L');
}
