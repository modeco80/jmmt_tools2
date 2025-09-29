#include <filesystem>
#include <jmmt/fs/game_filesystem.hpp>
#include "jmmt/game_version.hpp"

int main() {
	auto fs = jmmt::fs::createGameFileSystem(std::filesystem::current_path());
	if(fs) {
		printf("Detected game version: %s\n", jmmt::getVersionString(fs->getVersion()).c_str());

		const auto& m = fs->getPackageMetadata();
		for(auto& [k, v] : m) {
			std::printf("package file %s\n", k.c_str());
		}
	} else {
		printf("Not a JMMT game root\n");
	}

	return 0;
}
