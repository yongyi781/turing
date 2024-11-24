set(targets
    fractal
    fractal2
    p17620
    sp4)

foreach(target ${targets})
    message("Adding target (specific): ${target}")
    add_executable(${target} "${target}.cpp")
    target_precompile_headers(${target} REUSE_FROM pch)
endforeach()
