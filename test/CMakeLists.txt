set(targets
    basic
    decide_bouncer
    decide_tcycler
    performance_simulate)

foreach(target ${targets})
    message("Adding target (test): ${target}")
    add_executable(${target} "${target}.cpp")
    add_test(NAME ${target} COMMAND ${target})
    target_precompile_headers(${target} REUSE_FROM pch)
endforeach()
