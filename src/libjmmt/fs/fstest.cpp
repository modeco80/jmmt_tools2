#include <filesystem>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/fs/pak_filesystem.hpp>
#include "mco/io/file_stream.hpp"
#include "mco/io/stream_utils.hpp"
#include <jmmt/fs/pak_file_stream.hpp>

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

		for(auto& [k,v] : pak->getMetadata()) {
			printf("file %s (%d bytes, datestamp 0x%08x)\n", k.c_str(), v.fileSize, v.dateStamp);
		}

		auto inStream = jmmt::fs::PakFileStream::open(pak, "text/strings.csv");
		auto outFile = mco::FileStream::open("strings2.csv", mco::FileStream::ReadWrite | mco::FileStream::Create);
		mco::teeStreams(inStream, outFile, inStream.getSize());
		outFile.close();
	} else {
		printf("Not a JMMT game root\n");
	}

	return 0;
}
