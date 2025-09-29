#pragma once
#include <filesystem>
#include <jmmt/game_version.hpp>
#include <mco/base_types.hpp>
#include <mco/pimple_container.hpp>
#include <jmmt/fs/package_metadata.hpp>
#include <unordered_map>

namespace jmmt::fs {

	class PakFileSystem;

	/// This class wraps accessing the filesystem as extracted from the disc.
	class GameFileSystem {
		class Impl;
		Unique<Impl> impl;

	   public:
		/// Constructor. [path] must be a root path.
		GameFileSystem(const std::filesystem::path& path);
		~GameFileSystem();

		/// Initalize the game file system. Returns true if it was successfully initialized.
		bool initialize();

		/// Returns true if the filesystem is valid.
		bool isValidFilesystem() const;

		/// Returns the identified game version of the current filesystem, or
		/// [GameVersion::Invalid] if invalid.
		GameVersion getVersion() const;

		/// Gets metadata of all package files that are in this filesystem.
		/// Not directly useful (intended for the pak file system), but public just in case.
		const std::unordered_map<std::string, PackageMetadata>& getPackageMetadata() const;

		/// Opens a package file. Returns a Ref<> to the package filesystem.
		Ref<PakFileSystem> openPackageFile(const std::string& packageFileName);
	};

	/// Creates a [GameFileSystem] instance for the path specified in [path].
	/// You should prefer this function, since it will heap allocate and automatically
	/// frees the instance when it fails to initalize properly.
	Ref<GameFileSystem> createGameFileSystem(const std::filesystem::path& path);

} // namespace jmmt::fs
