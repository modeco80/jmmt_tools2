#include <cstdio>
#include <jmmt/fs/pak_filesystem.hpp>
#include <mco/utils.hpp>
#include "utils.hpp"
#include "cmd.hpp"

namespace jmpak {

	namespace {
		void commandListHelp() {
			std::printf("l [pakfile] - Lists all files in the requested pakfile\n");
		}

		int commandList(int argc, char** argv) {
			// We expect a single argument, the packfile to list
			if(argc != 1) {
				std::printf("no packfile given\n");
				return 1;
			}

			auto fs = getGameFileSystem();
			if(!fs) {
				std::printf("filesystem initalization failure\n");
				return 1;
			}

			auto pak = fs->openPackageFile(argv[0]);
			if(!pak) {
				std::printf("could not open package file \"%s\"\n", argv[0]);
				return 1;
			}

			for(auto& [filename, meta] : pak->getMetadata()) {
				std::printf("0x%08x %32s %8s\n", meta.dateStamp, filename.c_str(), mco::makeHumanReadableByteSize(meta.fileSize).c_str());
			}

			return 0;
		}
	} // namespace

	static Command cmdList('l', &commandListHelp, &commandList);
} // namespace jmpak
