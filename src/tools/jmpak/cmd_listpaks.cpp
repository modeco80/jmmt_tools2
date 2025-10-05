#include <cstdio>
#include "cmd.hpp"
#include <jmmt/fs/pak_filesystem.hpp>

namespace jmpak {

	int commandListPaks() {
		auto fs = getGameFileSystem();
		if(!fs) {
			std::printf("filesystem initalization failed\n");
			return 1;
		}

		std::printf("Package files in filesystem:\n");
		for (auto& [filename, metadata] : fs->getPackageMetadata()) {
			std::printf("%s\n", filename.c_str());
		}
		return 0;
	}

}
