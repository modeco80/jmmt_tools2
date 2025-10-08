#include <cstdio>
#include <filesystem>
#include <jmmt/fs/pak_filesystem.hpp>
#include <mco/utils.hpp>

#include "cmd.hpp"
#include <jmmt/fs/pak_file_stream.hpp>
#include <mco/io/file_stream.hpp>
#include "mco/io/stream_utils.hpp"
#include "utils.hpp"

// FIXME:
// Actual options:
// * -d/--directory <dir>
//

namespace jmpak {

	namespace {

		void commandExtractAllHelp() {
			std::printf(
				"Extracts the entire package's contents.\n"
			);
		}

		int commandExtractAll(int argc, char** argv) {
			if(argc != 1) {
				std::printf("no packfile provided");
				return 1;
			}

			auto fs = getGameFileSystem();
			if(!fs) {
				std::printf("filesystem initalization failure\n");
				return 1;
			}

			auto pak = fs->openPackageFile(argv[0]);
			if(!pak) {
				std::printf("could not open package file \"%s\"\n", argv[0]);
				return 1;
			}

			auto pakFilenamePath = std::filesystem::path(argv[0]);
			auto outputRoot = std::filesystem::current_path() /pakFilenamePath.stem();

			if(!std::filesystem::exists(outputRoot))
				std::filesystem::create_directories(outputRoot);

			for(auto& [filename, meta] : pak->getMetadata()) {
				auto filenameCopy = filename;
				for(auto& c : filenameCopy)
					if(c == '\\')
						c = '/';

				auto outPath = outputRoot / filenameCopy;
				if(auto parent = outPath.parent_path(); !std::filesystem::exists(parent))
					std::filesystem::create_directories(parent);

				auto pakFileStream = jmmt::fs::PakFileStream::open(pak, filename);
				auto outputStream = mco::FileStream::open(outPath.string().c_str(), mco::FileStream::ReadWrite | mco::FileStream::Create);
				mco::teeStreams(pakFileStream, outputStream, meta.fileSize);

				printf("Extracted %s\n", outPath.string().c_str());
			}

			return 0;
		}
	} // namespace

	static Command cmdExtractAll('x', &commandExtractAllHelp, &commandExtractAll);
} // namespace jmpak
