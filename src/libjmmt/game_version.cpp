#include <format>
#include <jmmt/game_version.hpp>

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
			case FirstLook: return "First Look Demo";
			case Version1: return "Version 1.0";
			case Version2: return "Version 2.0";
			default:
				return "???";
		}
	}

	std::string getVersionString(GameVersion version) {
		// Handle the invalid version specifically.
		if(version == GameVersion::Invalid)
			return "invalid";
		return std::format("{} {}", regionToString(extractGameRegion(version)), versionToString(extractGameVersion(version)));
	}

}
