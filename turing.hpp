#pragma once

#include <algorithm>
#include <deque>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include <ansi.hpp>
#include <boost/container_hash/hash.hpp>
#include <euler/algorithm.hpp>

namespace turing
{
using symbol_type = uint8_t;
using state_type = int8_t;

// Trim from start (in place)
inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::ranges::find_if(s, [](char ch) { return !std::isspace(ch); }));
}

// Trim from end (in place)
inline void rtrim(std::string &s)
{
    s.erase(std::ranges::find_if(std::ranges::reverse_view(s), [](char ch) { return !std::isspace(ch); }).base(),
            s.end());
}

/// Left or right.
enum class direction : bool
{
    left,
    right
};

/// A Turing machine transition.
struct transition
{
    symbol_type symbol = 0;
    direction direction = direction::left;
    state_type toState = -1;
};

constexpr size_t maxStates = 6;
constexpr size_t maxSymbols = 6;
class turing_rule
{
  public:
    constexpr turing_rule(size_t nStates = 0, size_t nSymbols = 0) : _nStates(nStates), _nSymbols(nSymbols) {}
    turing_rule(std::string code)
    {
        ltrim(code);
        rtrim(code);
        if (code.empty())
            return;
        std::istringstream ss(code);
        std::string token;
        const std::vector<std::string> rows;
        size_t i = 0;
        for (; std::getline(ss, token, '_') && i < maxStates; ++i)
        {
            if (token.empty() || token.size() % 3 != 0 || (_nSymbols != 0 && token.size() / 3 != _nSymbols))
            {
                _nSymbols = 0;
                return;
            }
            if (_nSymbols == 0)
                _nSymbols = token.size() / 3;
            for (size_t j = 0; j < _nSymbols; ++j)
            {
                auto triple = token.substr(3 * j, 3);
                if (triple[2] == '-')
                    _data[i][j] = transition{.symbol = 1, .direction = direction::right, .toState = -1};
                else
                {
                    const symbol_type symbol = triple[0] - '0';
                    if (symbol >= _nSymbols || (triple[1] != 'L' && triple[1] != 'R'))
                    {
                        _nSymbols = 0;
                        return;
                    }
                    _data[i][j] = transition{.symbol = (symbol_type)(triple[0] - '0'),
                                             .direction = triple[1] == 'R' ? direction::right : direction::left,
                                             .toState = (state_type)(triple[2] - 'A')};
                }
            }
        }
        _nStates = i;
    }

    [[nodiscard]] constexpr transition &operator[](size_t i, size_t j) { return _data[i][j]; }
    [[nodiscard]] constexpr const transition &operator[](size_t i, size_t j) const { return _data[i][j]; }

    [[nodiscard]] constexpr size_t numStates() const { return _nStates; }
    constexpr void numStates(size_t n) { _nStates = n; }
    [[nodiscard]] constexpr size_t numSymbols() const { return _nSymbols; }
    constexpr void numSymbols(size_t n) { _nSymbols = n; }
    [[nodiscard]] constexpr bool empty() const { return _nStates == 0 || _nSymbols == 0; }
    [[nodiscard]] constexpr bool filled() const
    {
        for (size_t i = 0; i < numStates(); ++i)
            for (size_t j = 0; j < numSymbols(); ++j)
                if (_data[i][j].toState == -1)
                    return false;
        return true;
    }

    [[nodiscard]] std::string str() const
    {
        std::string s;
        for (size_t i = 0; i < numStates(); ++i)
        {
            if (i != 0)
                s += "_";
            for (size_t j = 0; j < numSymbols(); ++j)
            {
                if ((*this)[i, j].toState == (state_type)-1)
                    s += "---";
                else
                {
                    s += (char)('0' + (*this)[i, j].symbol);
                    s += (*this)[i, j].direction == turing::direction::left ? 'L' : 'R';
                    s += (char)((*this)[i, j].toState + 'A');
                }
            }
        }
        return s;
    }

  private:
    std::array<std::array<transition, maxSymbols>, maxStates> _data{};
    size_t _nStates = 0;
    size_t _nSymbols = 0;
};

/// Turing state background color, following bbchallenge.org (but a bit darker).
inline std::string getBgStyle(state_type state)
{
    switch (state)
    {
    case 0:
        return ansi::bg(128, 0, 0);
    case 1:
        return ansi::bg(128, 96, 0);
    case 2:
        return ansi::bg(32, 64, 255);
    case 3:
        return ansi::bg(0, 128, 0);
    case 4:
        return ansi::bg(128, 0, 128);
    case 5:
        return ansi::bg(0, 128, 128);
    case -1:
    case 25:
        return ansi::bg(255, 0, 0);
    default:
        return "";
    }
}

/// Turing state foreground color, following bbchallenge.org (but a bit darker).
inline std::string getFgStyle(int index)
{
    switch (index)
    {
    case 0:
        return ansi::fg(255, 135, 155);
    case 1:
        return ansi::fg(255, 196, 88);
    case 2:
        return ansi::fg(124, 192, 255);
    case 3:
        return ansi::fg(176, 250, 160);
    case 4:
        return ansi::fg(219, 165, 255);
    case 5:
        return ansi::fg(79, 255, 235);
    case 6:
        return ansi::fg(195, 197, 79);
    case 7:
        return ansi::fg(255, 255, 255);
    default:
        return ansi::str(ansi::white);
    }
}

struct tape_segment
{
    state_type state = 0;
    std::vector<symbol_type> data;
    /// Relative head position.
    int64_t head = 0;

    constexpr friend bool operator==(const tape_segment &a, const tape_segment &b)
    {
        return a.data == b.data && a.head == b.head && a.state == b.state;
    }

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

struct macro_transition
{
    tape_segment from;
    tape_segment to;
    size_t steps = 0;

    constexpr friend bool operator==(const macro_transition &a, const macro_transition &b)
    {
        return a.from == b.from && a.to == b.to;
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o,
                                                         const turing::macro_transition &ts)
    {
        return o << ts.from << " → " << ts.to << " (" << ts.steps << ", " << ts.to.head - ts.from.head << ")";
    }
};

/// A Turing tape with up to 256 symbols, along with a head and a state.
class Tape
{
  public:
    using container_type = std::vector<symbol_type>;
    static constexpr size_t defaultPrintWidth = 50;

    /// Constructor for Tape.
    constexpr Tape(container_type data = {0}, int64_t head = 0) : _data(std::move(data)), _head(head) {}

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
    [[nodiscard]] constexpr size_t state() const { return _state; }

    /// Returns whether the tape consists of all zeros.
    [[nodiscard]] constexpr bool blank() const
    {
        return _data[0] == 0 && std::ranges::equal(std::ranges::subrange(_data.begin(), _data.end() - 1),
                                                   std::ranges::subrange(_data.begin() + 1, _data.end()));
    }

    /// Steps, and returns whether the tape expanded as a result of the step.
    constexpr bool step(const transition &tr)
    {
        **this = tr.symbol;
        _state = tr.toState;
        return tr.direction == direction::left ? moveLeft() : moveRight();
    }

    /// Returns a string representation of this tape.
    [[nodiscard]] constexpr std::string str(size_t width = defaultPrintWidth) const
    {
        return strAux(width, true, ">", {});
    }

    /// Returns a string representation of this tape, colored for the terminal.
    [[nodiscard]] constexpr std::string prettyStr(size_t width = defaultPrintWidth) const
    {
        return strAux(width, false, getBgStyle(_state), ansi::str(ansi::bgDefault));
    }

    /// @brief Gets the tape segment between `start` and `stop`, inclusive.
    /// @param start The start position (inclusive).
    /// @param stop The stop position (inclusive).
    [[nodiscard]] tape_segment getSegment(int64_t start, int64_t stop) const
    {
        std::vector<symbol_type> v(size_t(stop - start + 1));
        for (int64_t i = start; i <= stop; ++i)
            v[i - start] = (*this)[i];
        return {.state = _state, .data = v, .head = _head - start};
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o, const Tape &t)
    {
        return o << t.str();
    }

  private:
    container_type _data{0};
    int64_t _head = 0;
    int64_t _offset = 0;
    int64_t _leftEdge = 0;
    state_type _state = 0;

    constexpr bool moveLeft()
    {
        --_head;
        if (_head < _leftEdge)
        {
            --_leftEdge;
            if (_head + _offset < 0)
            {
                const size_t n = _data.size();
                _data.insert(_data.begin(), n, 0);
                _offset += n;
            }
            return true;
        }
        return false;
    }

    constexpr bool moveRight()
    {
        ++_head;
        if (_head + _offset >= (int64_t)_data.size())
        {
            _data.push_back(0);
            return true;
        }
        return false;
    }

    [[nodiscard]] constexpr std::string strAux(size_t width = defaultPrintWidth, bool printState = true,
                                               std::string_view headPrefix = ">",
                                               std::string_view headSuffix = "") const
    {
        std::string s;
        if (printState)
            s += (char)(_state + 'A');
        const int64_t shift = width / 2;
        const int64_t start = width * floorDiv((int64_t)(_head + shift), (int64_t)width) - shift;
        for (int64_t i = start; i < (int64_t)(start + width); ++i)
        {
            if (i == _head)
                s += headPrefix;
            const int64_t j = i + _offset;
            s += (i >= _leftEdge && i <= rightEdge() ? (char)('0' + _data[j]) : ' ');
            if (i == _head)
                s += headSuffix;
        }
        return s;
    }
};

/// Doesn't work yet, but good enough for 4x2. Precondition: all defined states are reachable.
inline turing_rule lexicalNormalForm(const turing_rule &rule)
{
    if (rule.numStates() <= 3) // nothing to do.
        return rule;
    // Don't worry about symbols yet
    auto statePerm = range((state_type)0, (state_type)(rule.numStates() - 1));
    auto highestUsedState = rule[0, 0].toState;
    for (state_type i = 0; i < (state_type)rule.numStates(); ++i)
    {
        for (symbol_type j = 0; j < (symbol_type)rule.numSymbols(); ++j)
        {
            if (i == 0 && j == 0)
                continue;
            auto k = rule[i, j].toState;
            if (k < 0 || k >= (int)rule.numStates())
                continue;
            if (k > highestUsedState + 1)
                std::swap(statePerm[k], statePerm[highestUsedState + 1]);
            highestUsedState = std::max(highestUsedState, k);
        }
    }
    turing_rule res(rule.numStates(), rule.numSymbols());
    for (state_type i = 0; i < (state_type)rule.numStates(); ++i)
    {
        for (symbol_type j = 0; j < (symbol_type)rule.numSymbols(); ++j)
        {
            if (rule[i, j].toState < 0 || rule[i, j].toState >= (int)rule.numStates())
                continue;
            res[statePerm[i], j] = {.symbol = rule[i, j].symbol,
                                    .direction = rule[i, j].direction,
                                    .toState = statePerm[rule[i, j].toState]};
        }
    }
    return res;
}

/// A Turing machine.
class TuringMachine
{
  public:
    struct step_result
    {
        // False if the machine was already in a halt state.
        bool success;
        // True if the tape grew in size as a result of the step.
        bool tapeExpanded;
    };

    constexpr TuringMachine(turing_rule rule = {}, Tape tape = {}, size_t steps = 0)
        : _rule(rule), _tape(std::move(tape)), _steps(steps)
    {
    }

    /// Initializes a Turing machine from a code in TNF format.
    TuringMachine(std::string code, Tape tape = {}, size_t steps = 0)
        : _rule(std::move(code)), _tape(std::move(tape)), _steps(steps)
    {
    }

    [[nodiscard]] constexpr size_t numStates() const { return _rule.numStates(); }
    [[nodiscard]] constexpr size_t numColors() const { return _rule.numSymbols(); }

    [[nodiscard]] constexpr const turing_rule &rule() const { return _rule; }
    [[nodiscard]] constexpr std::string ruleStr() const { return _rule.str(); }
    [[nodiscard]] constexpr const Tape &tape() const { return _tape; }
    constexpr void tape(Tape newTape) { _tape = std::move(newTape); }
    [[nodiscard]] constexpr size_t steps() const { return _steps; }
    void steps(size_t newSteps) { _steps = newSteps; }
    [[nodiscard]] constexpr state_type state() const { return _tape.state(); }

    /// Returns whether the Turing machine is halted, i.e. in the Z state.
    [[nodiscard]] constexpr bool halted() const { return state() < 0 || (size_t)state() >= numStates(); }
    [[nodiscard]] constexpr bool blank() const { return _tape.blank(); }

    [[nodiscard]] constexpr int64_t head() const { return _tape.head(); }
    [[nodiscard]] constexpr int64_t offset() const { return _tape.offset(); }

    /// Gets the transition that this machine will execute next.
    [[nodiscard]] constexpr const transition &peek() const { return _rule[state(), *_tape]; }

    /// Steps, and returns true if the machine advanced, false otherwise (for example, it was already halted).
    step_result step()
    {
        if (halted())
            return {.success = false, .tapeExpanded = false};
        ++_steps;
        return {.success = true, .tapeExpanded = _tape.step(peek())};
    }

    /// Resets this Turing machine to the given tape and step 0, but keeps the rule.
    void reset(Tape tape = {})
    {
        _tape = std::move(tape);
        _steps = 0;
    }

    /// Seeks to step number n.
    void seek(size_t n)
    {
        if (_steps == n)
            return;
        if (_steps > n)
        {
            std::cerr << "Generally try to avoid calling seek() backwards.\n";
            reset();
        }
        while (_steps < n)
            if (!step().success)
                break;
    }

    [[nodiscard]] std::string str(size_t width = Tape::defaultPrintWidth) const { return _tape.str(width); }
    [[nodiscard]] std::string prettyStr(size_t width = Tape::defaultPrintWidth) const { return _tape.prettyStr(width); }

  private:
    turing_rule _rule;
    Tape _tape;
    size_t _steps = 0;
};

/// Returns whether the given spans of t1 and t2, relative to their head positions, are identical.
inline bool spansEqual(const Tape &t1, const Tape &t2, int64_t start, int64_t end)
{
    for (int64_t i = start; i <= end; ++i)
        if (t1[t1.head() + i] != t2[t2.head() + i])
            return false;
    return true;
}

/// Parses a number, handling input like 1e8 correctly.
inline size_t parseNumber(const std::string &s)
{
    return s.contains('e') || s.contains('E') ? std::stod(s) : std::stoll(s);
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
constexpr size_t hash_value(const turing::macro_transition &t)
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
