// internal implementation details for commands
#include <filesystem>
#include <jmmt/fs/game_filesystem.hpp>

namespace jmpak {

	Ref<jmmt::fs::GameFileSystem> getGameFileSystem() {
		static Ref<jmmt::fs::GameFileSystem> ptr;
		std::filesystem::path path = std::filesystem::current_path();

		if(!ptr) {
			// try to create
			auto env = std::getenv("JMMT_FS_PATH");
			if(env) {
				path = env;
			}
			ptr = jmmt::fs::createGameFileSystem(path);
		}

		return ptr;
	}

}
