#pragma once
#include <jmmt/fs/game_filesystem.hpp>

namespace jmpak {
	/// Obtains a global GameFileSystem which can be used in all of jmpak.
	Ref<jmmt::fs::GameFileSystem> getGameFileSystem();
} // namespace jmpak
