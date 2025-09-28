#include <jmmt/impl/sha256.hpp>
#include <openssl/evp.h>

namespace jmmt::impl {
	ShaDigest sha256Digest(const u8* pData, usize length) {
		ShaDigest digest{};
		auto ctx = EVP_MD_CTX_new();
		EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
		EVP_DigestUpdate(ctx, pData, length);
		EVP_DigestFinal_ex(ctx, &digest[0], nullptr);
		EVP_MD_CTX_free(ctx);
		return digest;
	}

	ShaDigest sha256Digest(mco::Stream& stream) {
		ShaDigest digest{};
		u8 buffer[1024]{};

		auto ctx = EVP_MD_CTX_new();

		EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);

		while(true) {
			auto n = stream.read(&buffer[0], sizeof(buffer));
			if(n == 0)
				break;
			EVP_DigestUpdate(ctx, &buffer[0], n);
		}

		EVP_DigestFinal_ex(ctx, &digest[0], nullptr);
		EVP_MD_CTX_free(ctx);
		return digest;
	}
}
