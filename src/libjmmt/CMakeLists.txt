
add_library(jmmt_lib
	game_version.cpp

	crc.cpp

	# LZSS
	lzss/decompress.cpp

	# Filesystem library
	fs/game_filesystem.cpp

	# Package filesystem
	fs/pak_filesystem.cpp

	# PS2 library
	ps2/vif.cpp
	ps2/vif_instructions.cpp
	ps2/vif_instruction_unpack.cpp
	ps2/vif_disasm.cpp
)

# OpenSSL SHA256 implementation
# TODO: Provide other implementations and require at least one is selected.
if(JMMT_USE_OPENSSL)
	find_package(OpenSSL CONFIG REQUIRED)
	target_link_libraries(jmmt_lib PRIVATE
		OpenSSL::Crypto
	)
	target_sources(jmmt_lib PRIVATE
		impl/sha256_openssl.cpp
	)
endif()

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

add_executable(fstest fs/fstest.cpp)
jmmt_target(fstest)
target_link_libraries(fstest PUBLIC
    jmmt::libjmmt
)

add_subdirectory(tests)
