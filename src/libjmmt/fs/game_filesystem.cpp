#include <filesystem>
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/structs/package_toc.hpp>
#include <mco/io/file_stream.hpp>
#include <jmmt/impl/sha256.hpp>
#include <jmmt/crc.hpp>
#include "jmmt/game_version.hpp"

namespace jmmt::fs {

	bool doesFolderExist(const std::filesystem::path& root, const std::string_view folderName) {
		return std::filesystem::is_directory(root / folderName);
	}

	bool doesFileExist(const std::filesystem::path& path, const std::string_view fileName) {
		return std::filesystem::is_regular_file(path / fileName);
	}

	/// Internal helper function to open a game path. This might be exposed later.
	mco::FileStream openGameFile(const std::filesystem::path& root, const std::string_view filename) {
		auto datFilename = std::format("{:X}.DAT", jmmt::HashString(filename));
		auto path = root / "DATA" / datFilename;

		if(std::filesystem::is_regular_file(path)) {
			return mco::FileStream::open(path.string().c_str());
		}

		// If the .DAT name didn't work, then just try the clear name.
		path = root / "DATA" / filename;
		return mco::FileStream::open(path.string().c_str());
	}

	class GameDetector {
		std::filesystem::path rootPath{};
		GameVersion detectedVersion{};
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
			// and thus we can't detect any version of the game.
			if(!doesFolderExist(rootPath, "DATA") ||
			   !doesFolderExist(rootPath, "IRX") ||
			   !doesFolderExist(rootPath, "MOVIES") ||
			   !doesFolderExist(rootPath, "MUSIC"))
				return std::nullopt;

			// Probe for a game version.
			// This should auto-update if/when new JMMT versions
			// happen to be discovered. I doubt that'll happen, but futureproofing...
			forEachGameVersion([&](GameVersion version, const std::string_view slusName) {
				if(doesFileExist(rootPath, slusName)) {
					// We found a canidate filename which matches a version.
					auto str = (rootPath /slusName).string();
					auto stream = mco::FileStream::open(str.c_str());
					auto hash = impl::sha256Digest(stream);
					if(hash == *getVersionHash(version)) {
						// The hash matches. We can now prove this version of the game.
						setDetectedVersion(version);
						return false;
					}
				} else {
					return true;
				}

				return true;
			});

			// No version was detected.
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
		std::unordered_map<std::string, PackageFileMetadata> metadata;

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
				auto packageTocFile = openGameFile(rootPath, "package.toc");
				auto nTocEntries = packageTocFile.getSize() / sizeof(structs::PackageTocHeader);
				for(auto i = 0; i < nTocEntries; ++i) {
					structs::PackageTocHeader tocEntry;
					if(auto n = packageTocFile.read(&tocEntry, sizeof(tocEntry)); n != sizeof(tocEntry)) {
						// A short read should be impossible.
						detectedVersion = std::nullopt;
						return false;
					}

					metadata[tocEntry.fileName] = {
						.nrPackageFiles = tocEntry.tocFileCount,
						.chunksStartOffset = tocEntry.tocStartOffset
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

	const std::unordered_map<std::string, GameFileSystem::PackageFileMetadata>& GameFileSystem::getMetadata() const {
		return impl->metadata;
	}

	Ref<PakFileSystem> GameFileSystem::openPackageFile(const std::string& packageFileName) {
		// STUB FOR NOW
		return nullptr;
	}

	Ref<GameFileSystem> createGameFileSystem(const std::filesystem::path& path) {
		if(auto sp = std::make_shared<GameFileSystem>(path); sp->initialize())
			return sp;
		// Failure to initalize deallocates as well.
		return nullptr;
	}

} // namespace jmmt::fs
