#include <filesystem>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/fs/pak_filesystem.hpp>
#include "mco/io/file_stream.hpp"

int main() {
	auto fs = jmmt::fs::createGameFileSystem(std::filesystem::current_path());
	if(fs) {
		printf("Detected game version: %s\n", jmmt::getVersionString(fs->getVersion()).c_str());

		const auto& m = fs->getPackageMetadata();
		// for(auto& [k, v] : m) {
		//	std::printf("package file %s\n", k.c_str());
		// }

		printf("Opening config.pak\n");
		auto pak = fs->openPackageFile("SF_san_fran.pak");
		if(!pak) {
			printf("Couldn't open\n");
			return 0;
		}

		for(auto& [k,v] : pak->getMetadata()) {
			printf("file %s (%d bytes, datestamp 0x%08x)\n", k.c_str(), v.fileSize, v.dateStamp);
		}

#if 0
		auto fh = pak->openFile("text/strings.csv");
		if(fh != -1) {
			auto outFile = mco::FileStream::open("strings.csv", mco::FileStream::ReadWrite | mco::FileStream::Create);
			while(true) {
				u8 buffer[2048] {};
				auto n = pak->readSome(fh, &buffer[0], 2048);
				if(n == 0) {
					break;
				}
				outFile.write(&buffer[0], n);
			}

			outFile.close();
		} else {
			std::printf("Couldn't open file\n");
		}
#endif
	} else {
		printf("Not a JMMT game root\n");
	}

	return 0;
}
