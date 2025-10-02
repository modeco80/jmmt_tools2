#include <filesystem>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/fs/pak_filesystem.hpp>

int main() {
	auto fs = jmmt::fs::createGameFileSystem(std::filesystem::current_path());
	if(fs) {
		printf("Detected game version: %s\n", jmmt::getVersionString(fs->getVersion()).c_str());

		const auto& m = fs->getPackageMetadata();
		// for(auto& [k, v] : m) {
		//	std::printf("package file %s\n", k.c_str());
		// }

		printf("Opening config.pak\n");
		auto pak = fs->openPackageFile("config.pak");
		if(!pak) {
			printf("Couldn't open\n");
			return 0;
		}

		auto fh = pak->openFile("text/strings.csv");
		if(fh != -1) {
			while(true) {
				char buffer[2048] {};
				auto n = pak->readSome(fh, &buffer[0], 2048);
				if(n == 0) {
					break;
				}
				printf("Read %u bytes\n", n);
				printf("%s\n", buffer);
			}

			pak->seekFile(fh, 100, jmmt::fs::PakFileSystem::SeekBegin);

			printf("test 2\n");
			while(true) {
				char buffer[65535] {};
				auto origin = pak->tellFile(fh);
				auto n = pak->readSome(fh, &buffer[0], 65535);
				if(n == 0) {
					break;
				}
				printf("Read %u bytes at %08x\n", n, origin);
				printf("%s\n", buffer);
			}
		} else {
			std::printf("Couldn't open file\n");
		}
	} else {
		printf("Not a JMMT game root\n");
	}

	return 0;
}
