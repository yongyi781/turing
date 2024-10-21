#include "../pch.hpp"

#include "../euler.hpp"

using namespace std;

enum class direction : uint8_t
{
    left,
    right
};

constexpr size_t initialTapeSize = 31;
bool debug = false;
size_t tmPrintWidth = 100;

array<byte, 1'000'000> buffer;
std::pmr::monotonic_buffer_resource mr{buffer.data(), buffer.size()};
pmr::polymorphic_allocator pa{&mr};

template <ranges::range Range> bool allZero(Range &&r)
{
    return r[0] == 0 &&
           ranges::equal(ranges::subrange(r.begin(), r.end() - 1), ranges::subrange(r.begin() + 1, r.end()));
}

class Tape
{
  public:
    using container_type = std::pmr::vector<uint8_t>;

    Tape(size_t numStates) { _stateHist.resize(numStates); }

    uint8_t operator*() const { return *_head; }
    uint8_t operator[](ptrdiff_t i) const
    {
        auto j = _head + i;
        return j >= _data.begin() && j < _data.end() ? *j : 0;
    }

    size_t operator()(uint8_t b, direction dir, char newState = '\0', bool print = debug)
    {
        return step(b, dir, newState, print);
    }

    size_t operator()(direction dir, char newState = '\0', bool print = debug) { return step(dir, newState, print); }

    size_t step(uint8_t b, direction dir, char newState = '\0', bool print = debug)
    {
        *_head = b;
        if (newState != '\0')
            _state = newState;
        if (_state != 'Z')
            ++_stateHist[_state - 'A'];

        if (dir == direction::left)
            moveLeft();
        else
            moveRight();
        ++_steps;
        if (print && (_statePrintFilter == '\0' || _state == _statePrintFilter))
        {
            cout << *this << '\n';
            // if (_steps == 66345)
            //     cin.get();
        }
        return _steps;
    }

    size_t step(direction dir, char newState = '\0', bool print = debug) { return step(*_head, dir, newState, print); }

    size_t halt() { return step(1, direction::right, 'Z'); }

    [[nodiscard]] size_t steps() const { return _steps; }
    [[nodiscard]] const container_type &data() const { return _data; }
    void steps(size_t newValue) { _steps = newValue; }
    void statePrintFilter(char newFilter) { _statePrintFilter = newFilter; }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &operator<<(std::basic_ostream<CharT, Traits> &o, const Tape &tm)
    {
        ostringstream ss;
        ss.flags(o.flags());
        ss.imbue(o.getloc());
        ss.precision(o.precision());

        ss << setw(7) << tm._steps << ' ';
        ss << setw(7) << ranges::count(tm._data, 1) << ' ';
        ss << setw(16) << tm._stateHist << ' ';
        printAux(ss, tm);
        return o << std::move(ss.str());
    }

  private:
    container_type _data = container_type(initialTapeSize, false, pa);
    container_type::iterator _head = _data.begin() + _data.size() / 2;
    size_t _steps = 0;
    char _state = 'A';
    std::pmr::vector<size_t> _stateHist{1UZ, pa};
    char _statePrintFilter = '\0';

    void moveLeft()
    {
        if (_head == _data.begin())
        {
            _data.insert(_data.begin(), 0);
            _head = _data.begin();
        }
        else
            --_head;
    }

    void moveRight()
    {
        if (_head == std::prev(_data.end()))
        {
            _data.push_back(0);
            _head = std::prev(_data.end());
        }
        else
            ++_head;
    }

    static std::string getStyle(char state)
    {
        switch (state)
        {
        case 'A':
            return ansi::str(ansi::bgBrightRed);
        case 'B':
            return ansi::bg(255, 192, 0) + ansi::str(ansi::black);
        case 'C':
            return ansi::str(ansi::bgBrightBlue);
        case 'D':
            return ansi::str(ansi::bgBrightGreen);
        case 'E':
            return ansi::str(ansi::bgBrightMagenta);
        case 'F':
            return ansi::str(ansi::bgBrightCyan);
        default:
            return ansi::str(ansi::invert);
        }
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &print1(std::basic_ostream<CharT, Traits> &o, const Tape &tm)
    {
        for (auto it = tm._data.begin(); it != tm._data.end(); ++it)
        {
            if (it == tm._head)
                o << getStyle(tm._state) << (*it != 0 ? (char)('0' + *it) : '_') << ansi::reset;
            else
                o << (*it != 0 ? (char)('0' + *it) : '_') << ansi::reset;
        }
        return o;
    }

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> &printAux(std::basic_ostream<CharT, Traits> &o, const Tape &tm)
    {
        auto start = tm._head - (tm._head - tm._data.begin()) % tmPrintWidth;
        for (auto it = start; it != tm._data.end() && it != start + tmPrintWidth; ++it)
        {
            if (it == tm._head)
                o << getStyle(tm._state) << (*it != 0 ? (char)('0' + *it) : '_') << ansi::reset;
            else if (it >= tm._data.begin())
                o << (*it != 0 ? (char)('0' + *it) : '_') << ansi::reset;
            else
                o << ' ';
        }
        return o;
    }
};

/// 1RB1LB_1LA1RZ. 2-state 2-color Busy Beaver champion.
///
/// Alice: drop, step forward, give control to Bob    | leave, step backward, give control to Bob.
/// Bob:   drop, step backward, give control to Alice | END.
auto bb2()
{
    using direction::left, direction::right;

    Tape t(2);
    t.statePrintFilter('A');
    if (debug)
        cout << t << '\n';
    while (true)
    {
        t(1, *t ? left : right, 'B');
        if (*t)
            return t.halt();
        t(1, left, 'A');
    }
}

/// 1RB1LZ_1LB0RC_1LC1LA. 3-state 2-color Busy Beaver champion.
///
/// Alice:   drop, step forward, give control to Bob    | END.
/// Bob:     drop, step backward                        | pick up, step forward, give control to Cameron.
/// Cameron: drop, step backward                        | leave, step backward, give control to Alice.
auto bb3()
{
    using direction::left, direction::right;

    Tape t(3);
    t.statePrintFilter('\0');
    if (debug)
        cout << t << '\n';
    while (*t == 0)
    {
        t(1, right, 'B');
        while (*t == 0)
            t(1, left);
        t(0, right, 'C');
        while (*t == 0)
            t(1, left);
        t(left, 'A');
    }
    return t(1, left, 'Z');
}

/// BB(4) champion. 1RB1LB_1LA0LC_1RZ1LD_1RD0RA.
///
/// Alice:   drop, step forward, Bob    | leave, step backward, Bob.
/// Bob:     drop, step backward, Alice | pick up, step backward, Cameron.
/// Cameron: END                        | leave, step backward, David.
/// David:   drop, step forward, David  | pick up, step forward, Alice.
auto bb4()
{
    using direction::left, direction::right;

    Tape t(4);
    // t.statePrintFilter('C');
    if (debug)
        cout << t << '\n';

    while (true)
    {
        t(1, *t ? left : right, 'B');
        if (*t == 0)
            t(1, left, 'A');
        else
        {
            t(0, left, 'C');
            if (*t == 0)
                return t.halt();
            t(left, 'D');
            while (*t == 0)
                t(1, right);
            t(0, right, 'A');
        }
    }
}

/// BB(5) champion. 1RB1LC_1RC1RB_1RD0LE_1LA1LD_1RZ0LA.
auto bb5()
{
    using direction::left, direction::right;

    Tape t(5);
    t.statePrintFilter('E');
    if (debug)
        cout << t << '\n';

    while (true)
    {
        if (*t == 0)
        {
            t(1, right, 'B');
            while (*t == 1)
                t(right);
            t(1, right, 'C');
        }
        else
            t(left, 'C');
        if (*t == 0)
        {
            t(1, right, 'D');
            while (*t == 1)
                t(left);
            t(1, left, 'A');
        }
        else
        {
            t(0, left, 'E');
            if (*t == 0)
                return t.halt();
            t(0, left, 'A');
        }
    }
}

/// 1RB0LC_1LD0LA_1RC1RD_1LA0LD. 4-state 2-color Blanking Beaver runner-up, 66345 steps.
auto bbb4RunnerUp()
{
    using direction::left, direction::right;

    Tape t(4);
    t.statePrintFilter('B');
    if (debug)
        cout << t << '\n';
    while (true)
    {
        if (*t == 0)
        {
            t(1, right, 'B');
            if (*t == 0)
            {
                t(1, left, 'D');
                while (*t == 1)
                    t(0, left);
                t(1, left, 'A');
            }
            else
                t(0, left, 'A');
        }
        else
        {
            t(0, left, 'C');
            while (*t == 0)
                t(1, right);
            t(right, 'D');
            while (*t == 1)
                t(0, left);
            if (allZero(t.data()))
                return t.steps();
            t(1, left, 'A');
        }
    }
    return t.steps();
}

/// Lin recurrence, living nightmare. 158491 preperiod, 17620 period. 1RB0RC_1LB1LD_0RA0LD_1LA1RC.
auto linRecurrence(size_t stepLimit)
{
    using direction::left, direction::right;

    Tape t(4);
    if (debug)
        cout << t << '\n';

    while (t.steps() <= stepLimit)
    {
        if (*t != 0)
        {
            t(0, right, 'C');
            if (*t == 0)
                t(0, right, 'A');
            else
            {
                t(0, left, 'D');
                while (true)
                {
                    if (*t == 0)
                    {
                        t(1, left, 'A');
                        break;
                    }
                    t(1, right, 'C');
                    if (*t == 0)
                    {
                        t(0, right, 'A');
                        break;
                    }
                    t(0, left, 'D');
                }
            }
        }
        else
        {
            t(1, right, 'B');
            while (*t == 0)
                t(1, left);
            t(1, left, 'D');
            while (true)
            {
                if (*t == 0)
                {
                    t(1, left, 'A');
                    break;
                }
                t(1, right, 'C');
                if (*t == 0)
                {
                    t(0, right, 'A');
                    break;
                }
                t(0, left, 'D');
            }
        }
    }
    return t.steps();
}

/// Lin recurrence, living nightmare. 158491 preperiod, 17620 period. 1RB0RC_1LB1LD_0RA0LD_1LA1RC.
auto linRecurrenceUnrolled(size_t stepLimit)
{
    using direction::left, direction::right;

    Tape t(4);
    if (debug)
        cout << t << '\n';

    // ofstream fout("test_out.txt");
    while (t.steps() <= stepLimit)
    {
        if (t[-1] == 0 && t[0] == 0 && t[1] == 0)
        {
            t(1, right, 'B', false);
            t(1, left, 'B', false);
            t(1, left, 'D', false);
            t(1, left, 'A');
        }
        else if (t[0] == 0 && t[1] == 1)
        {
            t.steps(t.steps() + 4);
            t(1, right, 'B', false);
            t(0, right, 'A');
        }
        else if (t[0] == 1 && t[1] == 0)
        {
            // fout << t.steps() << ", " << flush;
            t(0, right, 'C', false);
            t(0, right, 'A');
        }
        else if (t[0] == 1 && t[1] == 1)
        {
            t(0, right, 'C', false);
            t(0, left, 'D', false);
            t(1, left, 'A');
        }
        else
        {
            cout << "Not sure" << '\n';
            return t.steps();
        }
    }
    return t.steps();
}

/// BBB4 champion. 1RB1LC_1RD1RB_0RD0RC_1LD1LA.
auto bbb4()
{
    using direction::left, direction::right;

    Tape t(4);
    t.statePrintFilter('A');
    if (debug)
        cout << t << '\n';

    size_t counter = 0;
    while (true)
    {
        if (*t == 0)
        {
            cout << counter << '\n';
            counter = 0;
            t(1, right, 'B');
            while (*t == 1)
                t(right);
            t(1, right, 'D');
        }
        else
        {
            ++counter;
            t(left, 'C');
            while (*t == 1)
                t(0, right);
            if (allZero(t.data()))
                return t.steps();
            t(right, 'D');
        }
        while (*t == 0)
            t(1, left);
        t(left, 'A');
    }
}

/// BB(2, 3) champion. 1RB2LB1RZ_2LA2RB1LB. 38 steps.
auto bb23()
{
    using direction::left, direction::right;

    Tape t(2);
    if (debug)
        cout << t << '\n';

    while (true)
    {
        switch (*t)
        {
        case 0:
            t(1, right, 'B');
            break;
        case 1:
            t(2, left, 'B');
            break;
        default:
            return t.halt();
        }
        while (true)
        {
            switch (*t)
            {
            case 0:
                break;
            case 1:
                t(2, right, 'B');
                continue;
            default:
                t(1, left, 'B');
                continue;
            }
            break;
        }
        t(2, left, 'A');
    }
}

/// BB(3, 3) current champion. 0RB2LA1RA_1LA2RB1RC_1RZ1LB1LC. 119,112,334,170,342,541 steps.
auto bb33()
{
    using direction::left, direction::right;

    Tape t(3);
    t.statePrintFilter('C');
    if (debug)
        cout << t << '\n';

    while (true)
    {
        switch (*t)
        {
        case 0:
            t(right, 'B');
            while (*t != 0)
            {
                if (*t == 1)
                    t(2, right, 'B');
                else
                {
                    t(1, right, 'C');
                    while (*t == 2)
                        t(1, left, 'C');
                    if (*t == 0)
                        return t.halt();
                    t(left, 'B');
                }
            }
            t(1, left, 'A');
            break;
        case 1:
            t(2, left, 'A');
            break;
        default:
            t(1, right, 'A');
            break;
        }
    }
    return t.steps();
}

/// BB(3, 3) 8th top halter. 1RB2LA1RA_1LB1LA2RC_1RZ1LC2RB. 1808669046 steps.
auto bb33_8th()
{
    using direction::left, direction::right;

    Tape t(3);
    t.statePrintFilter('C');
    if (debug)
        cout << t << '\n';

    while (true)
    {
        switch (*t)
        {
        case 0:
            t(1, right, 'B');
            while (*t != 1)
            {
                if (*t == 0)
                    t(1, left, 'B');
                else
                {
                    t(right, 'C');
                    while (*t == 1)
                        t(left, 'C');
                    if (*t == 0)
                        return t.halt();
                    t(right, 'B');
                }
            }
            t(1, left, 'A');
            break;
        case 1:
            t(2, left, 'A');
            break;
        default:
            t(1, right, 'A');
            break;
        }
    }
}

/// BB(3, 3) holdout TM #816. 1RB2RA1LB_0LC0RA1LA_---2LA---.
auto bb33Holdout_tm816()
{
    using direction::left, direction::right;

    Tape t(3);
    t.statePrintFilter('C');
    if (debug)
        cout << t << '\n';

    while (true)
    {
        switch (*t)
        {
        case 0:
            t(1, right, 'B');
            break;
        case 1:
            t(2, right, 'A');
            continue;
        default:
            t(1, left, 'B');
            break;
        }
        switch (*t)
        {
        case 0:
            t(left, 'C');
            if (*t == 0 || *t == 2)
                return t.halt();
            t(2, left, 'A');
            break;
        case 1:
            t(0, right, 'A');
            break;
        default:
            t(1, left, 'A');
            break;
        }
    }
}

int main()
{
    debug = true;
    if (debug)
        ios::sync_with_stdio(false);
    // printTiming(debug ? 1 : 100000, bb2);
    // printTiming(debug ? 1 : 100000, bb3);
    printTiming(debug ? 1 : 100000, bb4);
    // printTiming(bb5);
    // printTiming(bbb4RunnerUp);
    // printTiming(bbb4);
    // printTiming(bb23);
    // printTiming(bb33);
    // printTiming(bb33_8th);
    // printTiming(bb33Holdout_tm816);
    // printTiming(linRecurrenceUnrolled, 10000);
    // printTiming(linRecurrence, 100000);
}
