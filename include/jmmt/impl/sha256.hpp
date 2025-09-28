#pragma once
#include <array>
#include <mco/base_types.hpp>
#include <mco/io/stream.hpp>

namespace jmmt::impl {
	/// The resulting bytes of a SHA-256 digest.
	using ShaDigest = std::array<u8, 32>;

	/// Perform a SHA-256 digest.
	ShaDigest sha256Digest(const u8* pData, usize size);

	/// Performs a SHA-256 digest on a mcolib stream.
	ShaDigest sha256Digest(mco::Stream& stream);
} // namespace jmmt::impl
