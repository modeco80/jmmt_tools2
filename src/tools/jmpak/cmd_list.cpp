#include <cstdio>
#include "cmd.hpp"
#include <jmmt/fs/pak_filesystem.hpp>
#include <mco/utils.hpp>
namespace jmpak {

	class ListCommandImpl {
	public:
		int run(const char* fileName) {
			auto fs = getGameFileSystem();
			if(!fs) {
				std::printf("filesystem initalization failure\n");
				return 1;
			}

			auto pak = fs->openPackageFile(fileName);
			if(!pak) {
				std::printf("could not open package file \"%s\"\n", fileName);
				return 1;
			}

			for(auto& [filename, meta] : pak->getMetadata()) {
				std::printf("0x%08x %32s %8s\n", meta.dateStamp, filename.c_str(), mco::makeHumanReadableByteSize(meta.fileSize).c_str());
			}

			return 0;
		}
	};

	int commandList(int argc, char** argv) {
		// We expect a single argument, the packfile to list
		if(argc != 1) {
			std::printf("no packfile given\n");
			return 1;
		}

		auto listCmd = ListCommandImpl();
		return listCmd.run(argv[0]);
	}

}
