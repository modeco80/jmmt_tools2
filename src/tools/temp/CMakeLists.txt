add_executable(jmtest
	main.cpp
	utils.cpp
	)

jmmt_target(jmtest)

target_link_libraries(jmtest PUBLIC
    jmmt::libjmmt
    # While libjmmt links in mcolib implicitly,
    # I think it's better to specify usage of it explicitly.
    mco::base
    mco::io

    raylib
)
