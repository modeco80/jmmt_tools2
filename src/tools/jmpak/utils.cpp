#include "utils.hpp"

namespace jmpak {
	Ref<jmmt::fs::GameFileSystem> getGameFileSystem() {
		static Ref<jmmt::fs::GameFileSystem> ptr;
		std::filesystem::path path = std::filesystem::current_path();

		if(!ptr) {
			// Use $JMMT_FS_PATH as an override if it exists.
			auto env = std::getenv("JMMT_FS_PATH");
			if(env) {
				path = env;
			}
			ptr = jmmt::fs::createGameFileSystem(path);
		}

		return ptr;
	}
} // namespace jmpak
