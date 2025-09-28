#include <format>
#include <jmmt/game_version.hpp>
#include <jmmt/impl/hex_buffer.hpp>

namespace jmmt {

	constexpr static const char* regionToString(Region region) {
		using enum Region;
		switch(region) {
			//case Invalid: return "invalid";
			case NtscU: return "NTSC-U";
			case Pal: return "PAL";
			default:
				return "???";
		}
	}

	constexpr static const char* versionToString(Version version) {
		using enum Version;
		switch(version) {
			//case Invalid: return "invalid";
			case Prealpha: return "Pre-Alpha";
			case DvdPreview: return "First Look Demo (DVD Preview)";
			case Release: return "Version 1.0";
			case ReleasePalGermany: return "Version 2.0 (PAL Germany)";
			case ReleasePalItaly: return "Version 3.0 (PAL Italy)";
			default:
				return "???";
		}
	}

	const impl::ShaDigest* getVersionHash(GameVersion version) {
#define X(region, _version, _slusname, sha256)    \
		if(version == GameVersion::region##_##_version) { \
			constexpr static auto V = impl::hexToBuffer<sha256>(); \
			return &V; \
		}
		jmmtVersions()
#undef X
		return nullptr;
	}


	std::string getVersionString(GameVersion version) {
		// Handle the invalid version specifically.
		if(version == GameVersion::Invalid)
			return "Invalid";
		return std::format("{} {}", regionToString(extractGameRegion(version)), versionToString(extractGameVersion(version)));
	}

}
