#include <format>
#include <jmmt/game_version.hpp>
#include <jmmt/impl/hex_buffer.hpp>

namespace jmmt {

	constexpr static const char* regionToString(Region region) {
		using enum Region;
		switch(region) {
			// case Invalid: return "invalid";
			case NtscU:
				return "NTSC-U";
			case Pal:
				return "PAL";
			default:
				return "???";
		}
	}

	constexpr static const char* versionToString(Version version) {
		using enum Version;
		switch(version) {
			// case Invalid: return "invalid";
			case Prealpha:
				return "Pre-Alpha";
			case DvdPreview:
				return "First Look Demo (DVD Preview)";
			case Release:
				return "Version 1.0";
			case ReleasePalGermany:
				return "Version 2.0 (PAL Germany)";
			case ReleasePalItaly:
				return "Version 3.0 (PAL Italy)";
			default:
				return "???";
		}
	}

	// Declare digests here.
#define X(region, version, _slusname, sha256) \
	constexpr static auto GameDigest_##region##_##version = impl::hexToBuffer<sha256>();
	jmmtVersions()
#undef X

	std::string_view getVersionSlusName(GameVersion version) {
		switch(version) {
#define X(region, _version, slusname, _sha256) \
	case GameVersion::region##_##_version:     \
		return slusname;                       \
		break;
			jmmtVersions()
#undef X
			default : return "";
		}
	}

	const impl::ShaDigest* getVersionHash(GameVersion version) {
		switch(version) {
#define X(region, _version, _slusname, sha256)    \
	case GameVersion::region##_##_version:        \
		return &GameDigest_##region##_##_version; \
		break;
			jmmtVersions()
#undef X
			default : return nullptr;
		}
	}

	std::string getVersionString(GameVersion version) {
		// Handle the invalid version specifically.
		if(version == GameVersion::Invalid)
			return "Invalid";
		return std::format("{} {}", regionToString(extractGameRegion(version)), versionToString(extractGameVersion(version)));
	}

} // namespace jmmt
