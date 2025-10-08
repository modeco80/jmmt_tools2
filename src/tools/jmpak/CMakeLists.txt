add_executable(jmpak
	# command class
	cmd.cpp
	# utils
	utils.cpp
	# commands
	cmd_extractall.cpp
	cmd_list.cpp
	cmd_fsinfo.cpp
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
