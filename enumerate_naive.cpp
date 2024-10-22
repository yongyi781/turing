#include "pch.hpp"
#include "turing.hpp"

using namespace std;

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
    using R = turing::TuringMachine::rule_type;
    using V = R::value_type;
    auto trs = views::cartesian_product(range((uint8_t)0, (uint8_t)(numSymbols - 1)),
                                        array{turing::direction::left, turing::direction::right},
                                        range('A', (char)('A' + numStates - 1))) |
               ranges::to<vector>();
    R r(numStates, numSymbols);
    r[0, 0] = {1, turing::direction::right, 'B'};
    return it::tree_preorder(
        pair{1, 'C'},
        [&](auto &&t, auto rec) {
            auto &&[i, nextValid] = t;
            for (uint8_t symbol = 0; symbol < (uint8_t)numSymbols; ++symbol)
                for (turing::direction dir : {turing::direction::left, turing::direction::right})
                    for (char state = 'A'; state <= min(nextValid, (char)('A' + numStates - 1)); ++state)
                    {
                        r[i / numSymbols, i % numSymbols] = {symbol, dir, state};
                        rec({i + 1, max((char)(state + 1), nextValid)});
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
    vector<char> s{'A'};
    while (!s.empty())
    {
        auto state = s.back();
        s.pop_back();
        for (int symbol = 0; symbol < nSymbols; ++symbol)
        {
            auto &&[_, dir, next] = rule[state - 'A', symbol];
            if (next >= 'A' && next < 'A' + nStates && next != state && !visited[next - 'A'])
            {
                visited[next - 'A'] = true;
                s.push_back(next);
            }
        }
    }
    return all_of(visited.begin(), visited.begin() + nStates, identity{});
}

auto solve()
{
    // string code;
    // code = "1RB1LF_1LB1LC_1RD0LE_---0RB_0RC0LA_1RC0RF";
    // cout << "Enter code: ";
    // cin >> code;
    // cout << "Turing machine = " << code << '\n';
    // auto res = analyze(turing::known::boydJohnson(), 'A', 1000, 40);
    // auto res = findTranslatedCyclerPeriod({"1RB1LA_0LA0RA_1LA0RA"}, 1000, 50000);
    // return res;
    // return it::wrap(res).map fun(x, (char)('0' + x)).to<string>();
    // run({code}, 0, 'A');

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
