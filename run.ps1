param (
    [Parameter(Position=0)]
    [string]$File,
    [Parameter(ValueFromRemainingArguments)]
    [string[]]$Args,
    [string]$Std = "c++26",
    [switch]$Rebuild,
    [switch]$TimeReport
)

$ErrorActionPreference = 'Stop'

$mode = $DebugPreference ? "debug" : "release"
if (-not $File.Contains(".")) {
    $File += ".cpp"
}
$content = Get-Content -Path $File -Raw
$usePch = $content -match "#include `"pch.hpp`"" -or $content -match "#include `"../pch.hpp`""
if ($content -match "// stack size: (\d+)") {
    $stack = $Matches[1]
}
$sanitize = $content -match "// sanitize"
$outputFile = Join-Path $mode ($File -replace "\.cpp$", ".exe")
$directory = Split-Path -Path $outputFile
# Check if directory exists, if not, create it
if (!(Test-Path -Path $directory)) {
    Write-Host "Creating directory $directory"
    New-Item -ItemType Directory -Path $directory | Out-Null
}

$clangArgs = $(
    "-fcolor-diagnostics",
    "-fno-caret-diagnostics",
    "-fansi-escape-codes",
    "-Wall",
    "-Wextra",
    "-Weffc++",
    "-Wdocumentation",
    "-pedantic",
    "-std=$Std",
    ($DebugPreference ? "-O0" : "-O3"),
    ($DebugPreference ? "-g" : ""),
    "-march=native",
    "-pthread",
    "-isystem",
    "C:\tools\primecount\include",
    "-isystem",
    "C:/Projects/euler/include",
    "-L",
    "C:\tools\primecount\lib",
    "-L",
    "C:\tools\ntl\lib",
    "$File",
    "-o",
    "$outputFile",
    "-lstdc++exp",
    "-ltbb12",
    "-lgmp",
    "-Wl,-s"
)
$buildMessage = "Building in $mode mode"
if ($usePch) {
    $clangArgs += "-include-pch", ($mode + "\euler.pch")
} else {
    $buildMessage += " without PCH"
}
if ($stack) {
    $clangArgs += "-Wl,--stack,$stack"
    $buildMessage += " and stack size $stack"
}
if ($VerbosePreference) {
    $clangArgs += "-v"
    $buildMessage += " with verbose output"
}
if ($sanitize) {
    $clangArgs += "-fsanitize=undefined,nullability", "-fsanitize-trap=all"
    $buildMessage += " with sanitizers"
}
if ($TimeReport) {
    $clangArgs += "-ftime-trace"
    $buildMessage += " with time report"
}
$buildMessage += "..."

function Build-File {
    $lastBuildTime = (Get-Item $outputFile -ErrorAction SilentlyContinue).LastWriteTime
    $sourceTime = (Get-Item $File).LastWriteTime

    $Rebuild = $Rebuild -or $sourceTime -gt $lastBuildTime -or $Std -ne "c++26" -or $TimeReport
    if ($Rebuild) {
        Write-Host -ForegroundColor Blue $buildMessage
        $buildStartTime = Get-Date
        clang++ $clangArgs
        $buildEndTime = Get-Date
        $success = $LASTEXITCODE -eq 0
        $color = $success ? "Green" : "Red"
        Write-Host -ForegroundColor $color "Build $($success ? "succeeded" : "failed") in $("{0:0.00} s" -f ($buildEndTime - $buildStartTime).TotalSeconds)."
        return $success
    } else {
        return $true
    }
}

if (Build-File) {
    & $outputFile $Args
    if ($LASTEXITCODE -ne 0) {
        switch ($LASTEXITCODE) {
            -2147483645 { $message = "Breakpoint" }
            -1073741819 { $message = "Segmentation fault" }
            -1073741795 { $message = "Illegal instruction" }
            -1073741676 { $message = "Integer division by zero" }
            -1073741571 { $message = "Stack overflow" }
            -1073741511 { $message = "Entry point not found" }
            -1073740940 { $message = "Heap corrupted" }
            -1073740791 { $message = "Unhandled C++ exception" }
            -1073741569 { $message = "Malformed function table" }

            Default { $message = "Program failed" }
        }
        Write-Host -ForegroundColor Red "$message (exit code $LASTEXITCODE / 0x$("{0:X}" -f $LASTEXITCODE))"
    }
    exit $LASTEXITCODE
}
