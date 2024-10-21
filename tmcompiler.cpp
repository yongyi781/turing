#include "pch.hpp"

using namespace std;

ostream &printStep(string_view triple, size_t numStates, ostream &o)
{
    if (triple == "---")
        triple = "1RZ";
    char maxState = 'A' + numStates - 1;
    char symbol = triple[0];
    char dir = toupper(triple[1]);
    char newState = toupper(triple[2]);
    if (newState < 'A' || newState > maxState)
        o << "return ";
    o << "t.Step(" << symbol << ", " << "Direction." << (dir == 'R' ? "Right" : "Left") << ", '" << newState << "');";
    if (newState >= 'A' && newState <= maxState)
        o << " goto " << newState << ";";
    return o;
}

ostream &printMain(string_view code, ostream &o)
{
    return o << R"(using System;
using System.Collections.Generic;

bool quiet = args.Length > 0 && (args[0] == "-q" || args[0] == "-quiet");
var steps = MyTM();
Console.WriteLine($"The Turing machine )"
             << code << R"( halted after {steps} steps.");
)";
}

ostream &printClasses(ostream &o)
{
    return o << R"(public enum Direction : byte { Left, Right }
public class Tape(bool debug) { private readonly List<byte> _data = [0]; private int _index = 0; private int _steps = 0; private char _state = 'A'; public byte Value => _data[_index]; public byte Peek(int delta) { int i = _index + delta; return i >= 0 && i < _data.Count ? _data[i] : (byte)0; } public int Step(byte b, Direction dir, char newState = '\0') { _data[_index] = b; if (newState != '\0') _state = newState; if (dir == Direction.Left) MoveLeft(); else MoveRight(); _steps++; if (debug) Console.WriteLine(this); return _steps; } public int Steps => _steps; public override string ToString() { var sb = new System.Text.StringBuilder(); sb.Append($"{_steps,8}"); sb.Append(' '); for (int i = 0; i < _data.Count; ++i) { if (i == _index) sb.Append(GetStyle(_state)); sb.Append(_data[i] == 0 ? '_' : (char)('0' + _data[i])); if (i == _index) sb.Append("\x1B[0m"); } return sb.ToString(); } private void MoveLeft() { if (_index == 0) _data.Insert(0, 0); else --_index; } private void MoveRight() { if (_index == _data.Count - 1) _data.Add(0); ++_index; } private static string GetStyle(char state) => state switch { 'A' => "\x1B[41m", 'B' => "\x1B[48;2;255;128;0m", 'C' => "\x1B[44m", 'D' => "\x1B[42m", 'E' => "\x1B[45m", 'F' => "\x1B[46m", 'Z' => "\x1B[7m", _ => "", }; }
)";
}

ostream &printTM(string_view code, ostream &o)
{
    o << "// " << code << R"(
int MyTM()
{
    Tape t = new(!quiet);
    if (!quiet) { Console.WriteLine(t.ToString()); }
)";
    auto states =
        it::split(string(code), '_').map fun(s, it::range(0, s.size() - 1, 3).map fun(i, s.substr(i, 3)).to()).to();
    for (size_t i = 0; i < states.size(); ++i)
    {
        o << (char)('A' + i) << ": switch (t.Value) {";
        for (size_t j = 0; j < states[i].size(); ++j)
        {
            if (j == states[i].size() - 1)
                o << " default: ";
            else
                o << " case " << j << ": ";
            printStep(states[i][j], states.size(), o);
        }
        o << " }\n";
    }
    return o << "}\n";
}

auto compile(string_view code, bool decompile, bool quiet)
{
    cout << "Compiling C# code for Turing machine " << code << "...\n";
    string p = string("C:/Temp/TMs/");
    p += code;
    p += ".cs";
    string outputPath = (string("C:/Temp/TMs/") + string(code) + ".exe");
    ofstream fout(p);
    printMain(code, fout) << '\n';
    printTM(code, fout) << '\n';
    printClasses(fout);
    fout.close();
    int res = system(
        (string("\"C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe\" ") + p +
         " -nologo -o -out:" + outputPath)
            .c_str());
    if (res == 0)
    {
        cout << ansi::green << "Compile succeeded." << ansi::reset << '\n';
        if (decompile)
            system((string("C:/Apps/ILSpy/ILSpy.exe ") + outputPath + " -n:T:Program").c_str());
        else
        {
            auto t1 = now();
            system((outputPath + (quiet ? " -q" : "")).c_str());
            cout << ansi::cyan << "Program finished in ";
            io::print(now() - t1) << "." << ansi::reset;
        }
    }
    else
    {
        cerr << ansi::red << "Compile failed." << ansi::reset << '\n';
        remove(p.c_str());
    }
}

// The point of this program is to compile C# code corresponding to a Turing machine, and then to decompile it with
// ILSpy.
int main(int argc, char *argv[])
{
    auto args = span(argv, argc);
    const char *code = "1RB1LC_0LA1RD_1LA0LC_0RB0RD";
    bool decompile = false;
    bool quiet = false;
    for (auto &&arg : args | views::drop(1))
        if (strcmp(arg, "-dec") == 0 || strcmp(arg, "-decompile") == 0)
            decompile = true;
        else if (strcmp(arg, "-q") == 0 || strcmp(arg, "-quiet") == 0)
            quiet = true;
        else
            code = arg;
    compile(code, decompile, quiet);
}
