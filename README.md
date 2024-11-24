# turing
Some fast C++ code for Turing machine enumeration and analysis. Work in progress.

## Dependencies
* https://github.com/yongyi781/euler (headers only)
  * You just have to clone that repository and put it side-by-side with this one.

## Features
* turing.hpp &mdash; main header file
* analyze.cpp &mdash; Turns an n-state machine into a 1-state machine that can look ahead and behind, and outputs the corresponding "packed" transitions
* antihydra.cpp &mdash; Just some [antihydra](https://wiki.bbchallenge.org/wiki/Antihydra) code
* enumerate.cpp &mdash; Turing machine enumeration by [Brady's algorithm](https://nickdrozd.github.io/2022/01/14/bradys-algorithm.html)
* simulate.cpp &mdash; Simple Turing machine simulator.
* tape_growth.cpp &mdash; Simulates a Turing machine and outputs between number of steps and tape size whenever the tape grows
* transcript.cpp &mdash; Output transcript of a Turing machine.
* decide/ &mdash; Deciders for cyclers, translated cyclers, and polynomial bouncers.
* test/ &mdash; Tests

## Building
This project can be built with CMake.
