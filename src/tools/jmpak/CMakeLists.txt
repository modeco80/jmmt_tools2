add_executable(jmpak
	# command helpers
	cmd.cpp
	# commands
	cmd_list.cpp
	cmd_listpaks.cpp
	# driver program
	main.cpp
	)
jmmt_target(jmpak)
target_link_libraries(jmpak PUBLIC
    jmmt::libjmmt
    # While libjmmt links in mcolib implicitly,
    # I think it's better to specify usage of it explicitly.
    mco::base
    mco::io
)
