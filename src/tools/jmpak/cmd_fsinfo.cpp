#include <cstdio>
#include <jmmt/fs/pak_filesystem.hpp>
#include <jmmt/game_version.hpp>

#include "cmd.hpp"
#include "utils.hpp"

namespace jmpak {

	namespace {
		void commandFsInfoHelp() {
			std::printf("filesystem information\n");
		}

		int commandFsInfo(int argc, char** argv) {
			static_cast<void>(argc);
			static_cast<void>(argv);

			auto fs = getGameFileSystem();
			if(!fs) {
				std::printf("filesystem initalization failed\n");
				return 1;
			}

			std::printf("Game version: %s\n", jmmt::getVersionString(fs->getVersion()).c_str());
			std::printf("Package files in filesystem:\n");

			for(auto& [filename, metadata] : fs->getPackageMetadata()) {
				std::printf("%s\n", filename.c_str());
			}
			return 0;
		}
	} // namespace

	static Command cmdFsInfo('i', &commandFsInfoHelp, &commandFsInfo);
} // namespace jmpak
