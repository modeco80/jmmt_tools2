//! Game version related types.
#pragma once
#include <mco/base_types.hpp>
#include <jmmt/impl/sha256.hpp>

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
			Prealpha = 0, // pal prealpha (abcd-12345)
			DvdPreview,	  // pal beta (same serial as pal v1)

			// RELEASE VERSIONS
			Release,  // ntsc (slus-20229), pal/aus v1 (sles-50362)
			ReleasePalGermany, // pal v2 (fr,de), sles-50619
			ReleasePalItaly, // pal v3 (es, it) sles-50620
		};

/// This contains all versions of the game we care about. When adding a new version/region,
/// you'll need to update the above enums as well as this macro to add the new version.
/// Once you do, all consuming code should Just Work.
#define jmmtVersions()                                                                                                          \
	X(NtscU, Release, "SLUS_202.29", "75ced445f45d493aece8e8c441c88cb8a88843620ca74c5ca82c8ce3ce45a264") /* NTSC */             \
	X(Pal, Prealpha, "ABCD_123.45", "48f21a3b7198bbe952139fa6c62b62ea100b038cc57acb49947b8b9fd3a67923")	 /* PAL pre-releases */ \
	X(Pal, DvdPreview, "SLES_503.62", "efc4d3448837b7c177c7f5f0f8cf0edfe0f5e56fb275df3efa99e27c10c58654")                       \
	X(Pal, Release, "SLES_503.62", "4c05711a234e146d415e4a05ba72fd7fb42e4daca7d8d3d26a79742c4c1112b9") /* PAL releases */       \
	X(Pal, ReleasePalGermany, "SLES_506.19", "b0603a83373365b604e648737e1706f2b4e9ba721f1411e4a4196e9f577f7cdd")                         \
	X(Pal, ReleasePalItaly, "SLES_506.20", "4454e604bf9bdb317a21e43c016f0fecb41135ed9fd0101dc0f81114011b0fa4")

		/// This is how we end up actually storing the game version.
		/// This is encoded as a u16 where the high byte is the region, and the low byte is the version.
		/// This is then stuffed into a type-safe enum which should make it nice and easy to work with.
#define jmmtMakeGameVersion(region, version) (static_cast<u16>(Region::region) << 8) | static_cast<u16>(Version::version)

		enum class GameVersion : u16 {
			Invalid = jmmtMakeGameVersion(Invalid, Invalid),
#define X(region, version, _slusname, _sha256) \
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

	template <class F>
	auto forEachGameVersion(F&& f) {
		// Effectively this generates a unrolled loop over all valid versions.
#define X(region, version, slusname, _sha256)                 \
	if(f(GameVersion::region##_##version, slusname) == false) \
		return;
		jmmtVersions()
#undef X
	}

	// Make sure the above functions function properly at compile time. Yay for compile-time unit testing.
	static_assert(extractGameRegion(GameVersion::Pal_Release) == Region::Pal, "extractGameRegion() needs fixed");
	static_assert(extractGameVersion(GameVersion::Pal_Release) == Version::Release, "extractGameVersion() needs fixed");


	const impl::ShaDigest* getVersionHash(GameVersion version);
	std::string getVersionString(GameVersion version);

} // namespace jmmt
