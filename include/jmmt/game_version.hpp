//! Game version related types.
#pragma once
#include <mco/base_types.hpp>

namespace jmmt {

	namespace impl {

		enum class Region : u8 {
			Invalid = 0xff,
			NtscU = 0x0,
			Pal,
		};

		enum class Version : u8 {
			Invalid = 0xff,

			// PREVIEW VERSIONS
			Prealpha = 0,
			FirstLook,
			// RELEASE VERSIONS
			Version1,
			Version2
		};

/// This contains all versions of the game we care about. When adding a new version/region,
/// you'll need to update the above enums as well as this macro to add the new version.
/// Once you do, all consuming code should Just Work.
#define jmmtVersions()                            \
	X(NtscU, Version1, "SLUS_202.29") /* NTSC */ \
	X(Pal, Prealpha, "ABCD_123.45")   /* PAL */  \
	X(Pal, FirstLook, "")                        \
	X(Pal, Version1, "SLES_506.20")              \
	X(Pal, Version2, "")

		/// This is how we end up actually storing the game version.
		/// This is encoded as a u16 where the high byte is the region, and the low byte is the version.
		/// This is then stuffed into a type-safe enum which should make it nice and easy to work with.
#define jmmtMakeGameVersion(region, version) (static_cast<u16>(Region::region) << 8) | static_cast<u16>(Version::version)

		enum class GameVersion : u16 {
			Invalid = jmmtMakeGameVersion(Invalid, Invalid),
#define X(region, version, _slusname) \
	region##_##version = jmmtMakeGameVersion(region, version),
			jmmtVersions()
#undef X
		};

#undef jmmtMakeGameVersion
	} // namespace impl

	using impl::GameVersion;
	using impl::Region;
	using impl::Version;

	constexpr Version extractGameVersion(GameVersion version) {
		return static_cast<Version>(static_cast<u8>(static_cast<u16>(version) & 0xff));
	}

	constexpr Region extractGameRegion(GameVersion version) {
		return static_cast<Region>(static_cast<u8>((static_cast<u16>(version) >> 8) & 0xff));
	}

	template<class F>
	auto forEachGameVersion(F&& f) {
		// Effectively this generates a unrolled loop over all valid versions.
#define X(region, version, slusname) \
		if(f(GameVersion::region##_##version, slusname) == false) return;
		jmmtVersions()
#undef X
	}

	// Make sure the above functions function properly at compile time. Yay for compile-time unit testing.
	static_assert(extractGameRegion(GameVersion::Pal_Version1) == Region::Pal, "extractGameRegion() needs fixed");
	static_assert(extractGameVersion(GameVersion::Pal_Version1) == Version::Version1, "extractGameVersion() needs fixed");

	std::string getVersionString(GameVersion version);

} // namespace jmmt
