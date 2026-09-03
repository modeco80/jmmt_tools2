#include "utils.hpp"

#include <cstring>

#include "jmmt/fs/pak_filesystem.hpp"

Ref<jmmt::fs::GameFileSystem> gGetGameFileSystem() {
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

	if(!ptr->initialize()) {
		ptr = nullptr;
	}

	return ptr;
}

// Stupid cache for loaded PAK files.
static std::unordered_map<std::string, Ref<jmmt::fs::PakFileSystem>> gLoadedPackages;

Ref<jmmt::fs::PakFileSystem> gOpenPakFile(const std::string& pakName) {
	if(auto it = gLoadedPackages.find(pakName); it == gLoadedPackages.end()) {
		auto sp = gGetGameFileSystem()->openPackageFile(pakName);
		if(!sp) {
			throw std::runtime_error("Could not open specified PAK file");
		}

		gLoadedPackages[pakName] = sp;
		return sp;
	} else {
		return it->second;
	}
}

void gClosePakFile(const std::string& pakName) {
	if(auto it = gLoadedPackages.find(pakName); it != gLoadedPackages.end()) {
		gLoadedPackages.erase(pakName);
	}
}

Blob gReadFileFromPak(Ref<jmmt::fs::PakFileSystem> pakHandle, const std::string& path) {
	auto pakFileFd = pakHandle->fileOpen(path);
	if(pakFileFd == jmmt::fs::PakFileSystem::FileNotExist) {
		throw std::runtime_error("Could not open file in PAK");
	}

	auto size = pakHandle->fileGetSize(pakFileFd);
	auto uniq = std::make_unique<u8[]>(size);

	// Read chunk-sized package data
	u8 buf[65535] {};
	usize cursor = 0;
	auto* pDst = uniq.get();
	while(true) {
		auto nRead = pakHandle->fileRead(pakFileFd, &buf[0], sizeof(buf));
		if(nRead == 0)
			break;

		// Copy read data into blob buffer
		memcpy(pDst, &buf[0], nRead);
		pDst += nRead;
	}

	pakHandle->fileClose(pakFileFd);

	return Blob {
		.pBlobData = std::move(uniq),
		.blobSize = size
	};
}

Blob gReadFileFromPak(const std::string& pakName, const std::string& path) {
	return gReadFileFromPak(gOpenPakFile(pakName), path);
}
