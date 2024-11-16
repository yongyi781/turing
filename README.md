# turing
Some fast C++ code for Turing machine enumeration and analysis. Work in progress.

## Dependencies
* https://github.com/yongyi781/euler

## Features
* turing.hpp -- main header file
* analyze.cpp -- Turns an n-state machine into a 1-state machine that can look ahead and behind, and outputs the corresponding "packed" transitions
* detect_period.hpp, detect_period.cpp -- Cycler and translated cycler detection.
* test/ -- Tests
* antihydra.cpp -- Just some [antihydra](https://wiki.bbchallenge.org/wiki/Antihydra) code
* enumerate.cpp -- Turing machine enumeration by [Brady's algorithm](https://nickdrozd.github.io/2022/01/14/bradys-algorithm.html)
* tape_growth.cpp -- Simulates a Turing machine and outputs between number of steps and tape size whenever the tape grows

