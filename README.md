# turing
Some fast C++ code for Turing machine enumeration and analysis. Work in progress.

## Dependencies
* https://github.com/yongyi781/euler

## Features
* turing.hpp &mdash; main header file
* analyze.cpp &mdash; Turns an n-state machine into a 1-state machine that can look ahead and behind, and outputs the corresponding "packed" transitions
* antihydra.cpp &mdash; Just some [antihydra](https://wiki.bbchallenge.org/wiki/Antihydra) code
* enumerate.cpp &mdash; Turing machine enumeration by [Brady's algorithm](https://nickdrozd.github.io/2022/01/14/bradys-algorithm.html)
* tape_growth.cpp &mdash; Simulates a Turing machine and outputs between number of steps and tape size whenever the tape grows
* transcript.cpp &mdash; Output transcript of a Turing machine.
* decide/ &mdash; Deciders for cyclers, translated cyclers, and polynomial bouncers.
* test/ &mdash; Tests

## Precompiled header
This code uses precompiled headers. Here's an example `tasks.json` file to get started. If you are on Linux or Mac, you may want to remove the .exe extension from the output file names.
```json
{
  "tasks": [
    {
      "type": "cppbuild",
      "label": "C/C++: clang++ build",
      "command": "clang++",
      "args": [
        "-fcolor-diagnostics",
        "-fno-caret-diagnostics",
        "-fansi-escape-codes",
        "-isystem",
        "../euler/include",
        "-include-pch",
        "${workspaceFolder}/release/turing.pch",
        "-march=native",
        "-O3",
        "-pedantic",
        "-pthread",
        "-std=c++26",
        "-Wall",
        "-Wdocumentation",
        "-Weffc++",
        "-Wextra",
        "${file}",
        "-o",
        "${workspaceFolder}/release/${relativeFileDirname}/${fileBasenameNoExtension}.exe",
        "-lstdc++exp",
        "-ltbb12",
        "-Wl,-s"
      ],
      "icon": {
        "color": "terminal.ansiCyan"
      },
      "group": {
        "kind": "build",
        "isDefault": true
      },
      "presentation": {
        "reveal": "silent",
      },
      "problemMatcher": {
        "base": "$gcc",
        "source": "clang++"
      }
    },
    {
      "type": "cppbuild",
      "label": "C/C++: clang++ build (debug)",
      "command": "clang++",
      "args": [
        "-fcolor-diagnostics",
        "-fno-caret-diagnostics",
        "-fansi-escape-codes",
        "-std=c++26",
        "-g",
        "-DDEBUG",
        "-pthread",
        "-isystem",
        "C:/tools/ntl/include",
        "-isystem",
        "C:/GitHub/cppitertools",
        "-isystem",
        "../euler/include",
        "-L",
        "C:/tools/ntl/lib",
        "-include-pch",
        "${workspaceFolder}/debug/turing.pch",
        "-Wall",
        "-Wextra",
        "-Weffc++",
        "-Wdocumentation",
        "-pedantic",
        "-march=native",
        "-fsanitize=undefined,nullability",
        "-fsanitize-trap=all",
        "${file}",
        "-o",
        "${workspaceFolder}/debug/${relativeFileDirname}/${fileBasenameNoExtension}.exe",
        "-lstdc++exp",
        "-ltbb12",
      ],
      "icon": {
        "color": "terminal.ansiRed"
      },
      "group": {
        "kind": "build"
      },
      "presentation": {
        "reveal": "silent",
      },
      "problemMatcher": {
        "base": "$gcc",
        "source": "clang++"
      }
    },
    {
      "type": "cppbuild",
      "label": "C/C++: clang++ build (debug, optimized)",
      "command": "clang++",
      "args": [
        "-fcolor-diagnostics",
        "-fansi-escape-codes",
        "-std=c++26",
        "-g",
        "-O3",
        "-pthread",
        "-isystem",
        "../euler/include",
        "-include-pch",
        "${workspaceFolder}/release/turing.pch",
        "-Wall",
        "-Wextra",
        "-Weffc++",
        "-Wdocumentation",
        "-pedantic",
        "-march=native",
        "${file}",
        "-o",
        "${workspaceFolder}/debug/${relativeFileDirname}/${fileBasenameNoExtension}.exe",
        "-lstdc++exp",
        "-ltbb12",
      ],
      "icon": {
        "color": "terminal.ansiCyan"
      },
      "group": "build",
      "presentation": {
        "reveal": "silent",
        "close": true
      }
    },
    {
      "label": "C/C++: precompile header",
      "group": "build",
      "dependsOn": [
        "C/C++: clang++ precompile header (debug)",
        "C/C++: clang++ precompile header (release)"
      ],
      "dependsOrder": "parallel"
    },
    {
      "label": "C/C++: clang++ precompile header (release)",
      "command": "clang++",
      "args": [
        "-xc++-header",
        "-fdiagnostics-color=always",
        "-std=c++26",
        "-g",
        "-O3",
        "-pthread",
        "-fpch-instantiate-templates",
        "-isystem",
        "../euler/include",
        "-Wall",
        "-Wextra",
        "-Weffc++",
        "-Wdocumentation",
        "-pedantic",
        "-march=native",
        "${workspaceFolder}/pch.hpp",
        "-o",
        "${workspaceFolder}/release/turing.pch"
      ],
      "icon": {
        "color": "terminal.ansiBlue"
      },
      "group": "build",
      "presentation": {
        "close": true
      }
    },
    {
      "label": "C/C++: clang++ precompile header (debug)",
      "command": "clang++",
      "args": [
        "-xc++-header",
        "-fdiagnostics-color=always",
        "-std=c++26",
        "-g",
        "-pthread",
        "-fpch-instantiate-templates",
        "-isystem",
        "../euler/include",
        "-Wall",
        "-Wextra",
        "-Weffc++",
        "-Wdocumentation",
        "-pedantic",
        "-march=native",
        "${workspaceFolder}/pch.hpp",
        "-o",
        "${workspaceFolder}/debug/turing.pch"
      ],
      "icon": {
        "color": "terminal.ansiBlue"
      },
      "group": "build",
      "presentation": {
        "close": true
      }
    }
  ],
  "version": "2.0.0"
}
```