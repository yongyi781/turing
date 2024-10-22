#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

inline turing::TuringMachine universal23() { return {"1RB2LA1LA_2LA2RB0RA"}; }
inline turing::TuringMachine bb622() { return {"1RB0RF_1RC0LD_1LB1RC_---0LE_1RA1LE_---0RC"}; }

// bool isLNF(const turing::TuringMachine::rule_type &rule){

//     for (size_t i = 0; i < rule.rows(); ++i)
//         for (size_t j = 0; j < rule.columns(); ++j){
//             auto &&[symbol, dir, state] = rule[i, j];
//             if (rule[i, j].dir == turing::direction::left)
//                 return false;
//             }
//     return true;
// }

bool enumTuringRules(int numStates, int numSymbols, auto f)
{
    auto trs =
        views::cartesian_product(range((symbol_type)0, (symbol_type)(numSymbols - 1)),
                                 array{turing::direction::left, turing::direction::right}, range(0, numStates - 1)) |
        ranges::to<vector>();
    turing_rule r(numStates, numSymbols);
    r[0, 0] = {1, turing::direction::right, 1};
    return it::tree_preorder(
        pair{1, 2},
        [&](auto &&t, auto rec) {
            auto &&[i, nextValid] = t;
            for (symbol_type symbol = 0; (int)symbol < numSymbols; ++symbol)
                for (turing::direction dir : {turing::direction::left, turing::direction::right})
                    for (state_type state = 0; state <= min(nextValid, numStates - 1); ++state)
                    {
                        r[i / numSymbols, i % numSymbols] = {symbol, dir, state};
                        rec({i + 1, max(state + 1, nextValid)});
                    }
            return it::result_continue;
        },
        fun(t, t.first < numSymbols * numStates))([&](auto &&t) {
        auto &&[i, nextValid] = t;
        if (i == numSymbols * numStates && !it::callbackResult(f, r))
            return it::result_break;
        return it::result_continue;
    });
}

constexpr size_t maxStates = 4;

bool allStatesReachable(const turing::TuringMachine::rule_type &rule)
{
    int nStates = rule.rows();
    int nSymbols = rule.columns();
    array<bool, maxStates> visited{};
    vector<char> s{0};
    while (!s.empty())
    {
        auto state = s.back();
        s.pop_back();
        for (int symbol = 0; symbol < nSymbols; ++symbol)
        {
            auto &&[_, dir, next] = rule[state, symbol];
            if (next >= 0 && next < nStates && next != state && !visited[next])
            {
                visited[next] = true;
                s.push_back(next);
            }
        }
    }
    return all_of(visited.begin(), visited.begin() + nStates, identity{});
}

auto solve()
{
    map<int, int> hist;
    int64_t total = 0;
    ofstream fout("data/analyze_out2.txt");
    fout << fixed << setprecision(3);
    constexpr int nSteps = 1'000'000;
    // constexpr int tapeSizeBound = isqrt(nSteps);
    enumTuringRules(3, 2, [&](auto &&r) {
        if (!allStatesReachable(r))
            return;
        ++total;
        auto p = turing::findTranslatedCyclerPeriod(r, 1000, 1000);
        ++hist[p];
        // if (p == 0 || p >= 100)
        // cout << setw(4) << p << " | " << to_string(r) << '\n';
        if (p == 2)
        {
            turing::TuringMachine m{r};
            size_t tapeSize2 = 0;
            for (int i = 1; i <= nSteps; ++i)
            {
                m.step();
                if (i == 100'000)
                    tapeSize2 = m.tape().size();
                // if (m.tape().size() >= tapeSizeBound)
                //     break;
            }

            // if ((int)m.tape().size() < tapeSizeBound)
            double ratio = (double)m.tape().size() / (double)tapeSize2;
            fout << setw(8) << ratio << " | " << setw(5) << m.tape().size() << " | " << setw(5) << tapeSize2 << " | "
                 << to_string(r) << '\n'
                 << flush;
        }
    });

    // for (auto &&rule : interesting)
    // {
    //     turing::TuringMachine m(rule);
    //     for (int i = 0; i < 1000000; ++i)
    //         m.step();

    //     cout << setw(5) << m.tape().data().size() << " | " << to_string(rule) << '\n';
    // }

    for (auto &&[p, c] : hist)
        cout << "# with period " << p << ": " << c << '\n';
    return total;
}

int main()
{
    // ios::sync_with_stdio(false);
    printTiming(solve);
}
