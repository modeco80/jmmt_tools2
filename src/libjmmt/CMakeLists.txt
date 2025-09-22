
add_library(jmmt_lib
	ps2/vif.cpp
	ps2/vif_disasm.cpp
)
jmmt_target(jmmt_lib)
target_link_libraries(jmmt_lib PUBLIC
    mco::base
    mco::io
)

add_library(jmmt::libjmmt ALIAS jmmt_lib)

add_executable(vifdis_test ps2/vifdis_test.cpp)
jmmt_target(vifdis_test)
target_link_libraries(vifdis_test PUBLIC
    jmmt::libjmmt
)

add_subdirectory(tests)
