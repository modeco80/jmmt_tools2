#pragma once
#include <jmmt/fs/game_filesystem.hpp>
#include <jmmt/fs/pak_filesystem.hpp>

#include "blob.hpp"

// FIXME: could be a part of jmmt::fs?

/// Obtains a global GameFileSystem.
Ref<jmmt::fs::GameFileSystem> gGetGameFileSystem();

Ref<jmmt::fs::PakFileSystem> gOpenPakFile(const std::string& pakName);
void gClosePakFile(const std::string& pakName);

/// Reads a file from a loaded PAK into a blob.
Blob gReadFileFromPak(Ref<jmmt::fs::PakFileSystem> pakHandle, const std::string& path);

/// Reads a file from a loaded PAK into a blob.
Blob gReadFileFromPak(const std::string& pakName, const std::string& path);
