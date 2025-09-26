#include <jmmt/fs/game_filesystem.hpp>
#include <mco/io/file_stream.hpp>

#include <jmmt/structs/package_toc.hpp>

namespace jmmt::fs {

	/// Internal helper function to open a game path. This might be exposed later.
	mco::FileStream openGamePath(const std::string_view gamePath);
	//}

	class GameDetector {
		std::filesystem::path rootPath;
		GameVersion detectedVersion;
		bool detected;

		void setDetectedVersion(GameVersion version) {
			detectedVersion = version;
			detected = true;
		}

	   public:
		explicit GameDetector(const std::filesystem::path& rootPath)
			: rootPath(rootPath) {
		}

		std::optional<GameVersion> detectVersion() {
			for(auto& file : std::filesystem::directory_iterator(rootPath)) {
				// This should auto-update if/when new JMMT versions
				// happen to be discovered. I doubt that'll happen, but futureproofing...
				auto guessFilename = file.path().filename().string();
				forEachGameVersion([&](GameVersion version, const std::string_view slusName) {
					// First try the default case.
					if(guessFilename == slusName) {
						setDetectedVersion(version);
						return false;
					}

					// Okay, we have to uppercase the input filename.
					// The Linux ISO9660 driver prefers lowercasing filenames.
					for(auto& c : guessFilename)
						c = std::toupper(c);

					if(guessFilename == slusName) {
						setDetectedVersion(version);
						return false;
					}

					// Well, that didn't work. Continue iterating.
					return true;
				});
			}

			// No version was detected, so just explicitly set it back to an invalid version.
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
			scanVersionImpl();

			// No game was detected.
			if(getVersionImpl() == GameVersion::Invalid) {
				return false;
			}

			// Try and load package.toc data.
			auto packageTocFile = openGamePath("package.toc");

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
		impl->scanVersionImpl();
	}

	bool GameFileSystem::initialize() {
		return impl->initalizeImpl();
	}

	const std::unordered_map<std::string, GameFileSystem::PackageFileMetadata>& GameFileSystem::getMetadata() const {
		return impl->metadata;
	}

	GameFileSystem::~GameFileSystem() = default;

} // namespace jmmt::fs
