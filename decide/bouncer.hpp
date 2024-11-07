#pragma once

#include "../turing.hpp"

inline bool checkQuadratic(const std::vector<size_t> &steps, size_t skip = 1)
{
    if (steps.size() < 4 * skip + 1)
        return false;
    std::span s(steps.end() - 4 * skip - 1, steps.end());
    // Check it's not linear first
    // if (s[2 * skip] - 2 * s[3 * skip] + s[4 * skip] == 0)
    //     return false;
    for (size_t i = 0; i <= skip; ++i)
        if (s[i] - 3 * s[skip + i] + 3 * s[2 * skip + i] - s[3 * skip + i] != 0)
            return false;
    return true;
}

struct bouncer_result
{
    bool found = false;
    size_t startStep = 0;
    size_t skip = 0;
};

class BouncerDecider
{
  public:
    constexpr explicit BouncerDecider(bool verbose = false) : _verbose(verbose) {}

    [[nodiscard]] bouncer_result find(turing::TuringMachine m, size_t maxSteps, size_t maxSkip) const
    {
        std::vector<size_t> lSteps;
        std::vector<size_t> rSteps;
        while (!m.halted() && m.steps() < maxSteps)
        {
            auto res = m.step();
            if (!res.tapeExpanded)
                continue;
            // Tape grew
            if (m.head() < 0)
            {
                lSteps.push_back(m.steps());
                for (size_t skip = 1; skip <= maxSkip; ++skip)
                    if (checkQuadratic(lSteps, skip))
                    {
                        if (_verbose)
                            std::cout << "L: " << lSteps << ", skip = " << skip << '\n';
                        return {true, *(lSteps.end() - 4 * skip - 1), skip};
                    }
            }
            else
            {
                rSteps.push_back(m.steps());
                for (size_t skip = 1; skip <= maxSkip; ++skip)
                    if (checkQuadratic(rSteps, skip))
                    {
                        if (_verbose)
                            std::cout << "R: " << rSteps << ", skip = " << skip << '\n';
                        return {true, *(rSteps.end() - 4 * skip - 1), skip};
                    }
            }
        }
        return {false};
    }

  private:
    bool _verbose;
};
