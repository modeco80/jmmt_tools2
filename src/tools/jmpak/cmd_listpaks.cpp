#include <cstdio>
#include <jmmt/fs/pak_filesystem.hpp>

#include "cmd.hpp"
#include "utils.hpp"

namespace jmpak {

	namespace {
		void commandListPaksHelp() {
			std::printf("L - Lists all pakfiles in this filesystem\n");
		}

		int commandListPaks(int argc, char** argv) {
			static_cast<void>(argc);
			static_cast<void>(argv);

			auto fs = getGameFileSystem();
			if(!fs) {
				std::printf("filesystem initalization failed\n");
				return 1;
			}

			std::printf("Package files in filesystem:\n");
			for(auto& [filename, metadata] : fs->getPackageMetadata()) {
				std::printf("%s\n", filename.c_str());
			}
			return 0;
		}
	} // namespace

	static Command cmdListPaks('L', &commandListPaksHelp, &commandListPaks);
} // namespace jmpak
