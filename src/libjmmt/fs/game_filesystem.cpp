#include <filesystem>
#include <jmmt/crc.hpp>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/impl/sha256.hpp>
#include <jmmt/structs/package_toc.hpp>
#include <mco/io/file_stream.hpp>

#include "jmmt/fs/pak_filesystem.hpp"
#include "jmmt/game_version.hpp"

namespace jmmt::fs {

	const std::string_view getTypeFolderName(GameFileSystem::FileType type) {
		switch(type) {
			case GameFileSystem::FileData:
				return "DATA";
			case GameFileSystem::FileIrx:
				return "IRX";
			case GameFileSystem::FileMovies:
				return "MOVIES";
			case GameFileSystem::FileMusic:
				return "MUSIC";
			default:
				return "";
		}
	}

	mco::FileStream openGameFile(const std::filesystem::path& root, const std::string_view filename, GameFileSystem::FileType type) {
		auto folderPath = root / getTypeFolderName(type);

		if(type == GameFileSystem::FileData) {
			auto datFilename = std::format("{:X}.DAT", jmmt::hashString(filename));
			if(auto composedPath = folderPath / datFilename; std::filesystem::is_regular_file(composedPath)) {
				return mco::FileStream::open(composedPath.string().c_str());
			} else {
				// If the .DAT name didn't work, then just try the clear name.
				composedPath = folderPath / filename;
				return mco::FileStream::open(composedPath.string().c_str());
			}
		} else {
			// All other file types are always clearnamed.
			auto composedPath = folderPath / filename;
			return mco::FileStream::open(composedPath.string().c_str());
		}
	}

	/// This class wraps the logic of detecting the version of the JMMT game
	/// that is being opened by the GameFileSystem implementation.
	class GameDetector {
		std::filesystem::path rootPath {};
		GameVersion detectedVersion {};
		bool detected = false;

		void setDetectedVersion(GameVersion version) {
			detectedVersion = version;
			detected = true;
		}

	   public:
		explicit GameDetector(const std::filesystem::path& rootPath)
			: rootPath(rootPath) {
		}

		// TODO: This should probably throw explicit errors at some point detailing what failed,
		// but for now, this works okay.

		std::optional<GameVersion> detectVersion() {
			// First, check for folders in the root directory that should always exist.
			// If any of these fail, then we are probably not in a JMMT game root,
			// and thus we can't reliably detect any version of the game.
			if(!std::filesystem::is_directory(rootPath / "DATA") ||
			   !std::filesystem::is_directory(rootPath / "IRX") ||
			   !std::filesystem::is_directory(rootPath / "MOVIES") ||
			   !std::filesystem::is_directory(rootPath / "MUSIC"))
				return std::nullopt;

			// Probe for a game version.
			// This should auto-update if/when new JMMT versions
			// happen to be discovered. I doubt that'll happen, but futureproofing...
			forEachGameVersion([&](GameVersion version) {
				const auto slusName = getVersionSlusName(version);
				if(std::filesystem::is_regular_file(rootPath / slusName)) {
					// We found a canidate ELF filename which matches a version.
					auto str = (rootPath / slusName).string();
					auto stream = mco::FileStream::open(str.c_str());
					auto hash = impl::sha256Digest(stream);
					if(hash == *getVersionHash(version)) {
						setDetectedVersion(version);
						return false;
					}
				} else {
					return true;
				}

				return true;
			});

			// No version was detected by the above logic, so it's probably safe to say
			// this isn't a JMMT filesystem.
			if(!detected) {
				return std::nullopt;
			}

			return detectedVersion;
		}
	};

	class GameFileSystem::Impl {
	   public:
		std::filesystem::path rootPath;
		std::optional<GameVersion> detectedVersion;
		std::unordered_map<std::string, PackageMetadata> metadata;

		explicit constexpr Impl(const std::filesystem::path& path)
			: rootPath(path) {
			detectedVersion = GameVersion::Invalid;
		}

		void scanVersionImpl() {
			auto detector = GameDetector(rootPath);
			detectedVersion = detector.detectVersion();
		}

		bool initalizeImpl() {
			// First, scan the game version. If no game version was detected,
			// then we'll just early out as soon as possible.
			scanVersionImpl();

			if(getVersionImpl() == GameVersion::Invalid) {
				return false;
			}

			// Try and load package.toc data.
			try {
				auto packageTocFile = openFileImpl("package.toc", FileData);
				auto nTocEntries = packageTocFile.getSize() / sizeof(structs::PackageTocHeader);
				for(auto i = 0; i < nTocEntries; ++i) {
					structs::PackageTocHeader tocEntry;
					if(auto n = packageTocFile.read(&tocEntry, sizeof(tocEntry)); n != sizeof(tocEntry)) {
						// A short read should be impossible.
						detectedVersion = std::nullopt;
						return false;
					}

					// Convert package.toc metadata to the libjmmt format.
					metadata[tocEntry.fileName] = {
						.nrPackageFiles = tocEntry.tocFileCount,
						.chunkStartOffset = tocEntry.tocStartOffset,
						.chunkDataSize = tocEntry.tocSize
					};
				}
			} catch(std::system_error& err) {
				// If this somehow fails, then the filesystem most likely isn't valid.
				detectedVersion = std::nullopt;
				return false;
			}

			return true;
		}

		bool isValidFilesystemImpl() const {
			return getVersionImpl() != GameVersion::Invalid;
		}

		GameVersion getVersionImpl() const {
			return detectedVersion.value_or(GameVersion::Invalid);
		}

		Ref<PakFileSystem> openPackageFileImpl(Ref<GameFileSystem> that, const std::string& packageFileName) {
			if(auto it = metadata.find(packageFileName); it != metadata.end()) {
				auto sp = std::make_shared<PakFileSystem>(that, it->second, packageFileName);
				if(auto ec = sp->initialize(); ec != PakFileSystem::Success) {
					return nullptr;
				}
				return sp;
			}

			return nullptr;
		}

		mco::FileStream openFileImpl(const std::string& filename, FileType type) {
			return openGameFile(rootPath, filename, type);
		}
	};

	// GameFileSystem

	GameFileSystem::GameFileSystem(const std::filesystem::path& path)
		: impl(std::make_unique<Impl>(path)) {
	}

	GameFileSystem::~GameFileSystem() = default;

	bool GameFileSystem::initialize() {
		return impl->initalizeImpl();
	}

	bool GameFileSystem::isValidFilesystem() const {
		return impl->isValidFilesystemImpl();
	}

	GameVersion GameFileSystem::getVersion() const {
		return impl->getVersionImpl();
	}

	const std::unordered_map<std::string, PackageMetadata>& GameFileSystem::getPackageMetadata() const {
		return impl->metadata;
	}

	Ref<PakFileSystem> GameFileSystem::openPackageFile(const std::string& packageFileName) {
		return impl->openPackageFileImpl(shared_from_this(), packageFileName);
	}

	mco::FileStream GameFileSystem::openFile(const std::string& filename, FileType type) {
		return impl->openFileImpl(filename, type);
	}

	Ref<GameFileSystem> createGameFileSystem(const std::filesystem::path& path) {
		if(auto sp = std::make_shared<GameFileSystem>(path); sp->initialize())
			return sp;
		// Failure to initalize deallocates as well.
		return nullptr;
	}

} // namespace jmmt::fs
