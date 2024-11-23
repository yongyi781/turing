set(targets
    cycler
    tcycler
    bouncer)

foreach(target ${targets})
    message("Adding target: ${target}")
    add_executable(${target} "${target}.cpp")
    target_precompile_headers(${target} REUSE_FROM pch)
endforeach()
