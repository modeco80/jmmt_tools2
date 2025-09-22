#pragma once
#include <bit>
#include <concepts>
#include <jmmt/fixed_string.hpp>
#include <mco/base_types.hpp>

namespace jmmt {

	namespace impl {
		/// A FourCC.
		enum class FourCC : u32 {};

		/// A compile-time string type which can hold a FourCC.
		using FourCCString = FixedString<4>;

		/// Extracts the raw integer repressentation from a FourCC.
		constexpr u32 extractFcc(FourCC fcc) {
			return static_cast<u32>(fcc);
		}

		template <class T>
		concept FourCCPolicy = requires(T t) {
			// Every policy has a consteval execute() template function
			// which takes in a arbitrary length FixedString, and returns a FourCCString.
			{ T::template execute<"FOUR">() } -> std::convertible_to<FourCCString>;
		};

		/// FourCC policy which requires the string to be at least 4 characters long
		/// (it is truncated to fit 4 characters if it is longer).
		///
		/// This is the default policy used by FourCCGenerator.
		struct FourCCDefaultPolicy {
			template <FixedString source>
			consteval static FourCCString execute() {
				static_assert(source.length() >= 4, "Provided string can not be a FourCC");
				return source.template truncate<4>();
			}
		};

		/// FourCC policy which pads a string with a given character (defaults to space)
		/// if the string is too small to be a FourCC. Some FourCCs use this, so
		/// this is provided to make those work in a cleaner fashion.
		template <char padCharacter = 0x20>
		struct FourCCPadPolicy {
			template <FixedString source>
			consteval static FourCCString execute() {
				FourCCString string;

				if(source.length() < 4) {
					string = FourCCString(source, source.length(), padCharacter);
				} else {
					// Just truncate. We don't need to pad if we reach this branch.
					string = source.template truncate<4>();
				}

				return string;
			}
		};

		/// A configurable compile-time FourCC generator.
		template <FourCCPolicy Policy = FourCCDefaultPolicy>
		struct FourCCGenerator {
			template <FixedString fccRawString, std::endian Endian>
			consteval static std::uint32_t generateRaw() {
				auto fccString = Policy::template execute<fccRawString>();
				switch(Endian) {
					case std::endian::little:
						return (fccString[0]) | (fccString[1] << 8) | (fccString[2] << 16) | (fccString[3] << 24);

					case std::endian::big:
						return (fccString[0] << 24) | (fccString[1] << 16) | (fccString[2] << 8) | fccString[3];
				}
			}

			/// Generate a FourCC value. This all runs at compile time!
			template <FixedString fccRawString, std::endian Endian = std::endian::little>
			constexpr static FourCC generate() {
				return static_cast<FourCC>(generateRaw<fccRawString, Endian>());
			}
		};

		/// [FourCCGenerator], but endian can be picked at runtime.
		template <FourCCPolicy Policy = FourCCDefaultPolicy>
		struct RuntimeFourCCGenerator {
			using Generator = FourCCGenerator<Policy>;

			template <FixedString fccString>
			inline static FourCC generate(std::endian endian = std::endian::little) {
				switch(endian) {
					case std::endian::little:
						return Generator::template generate<fccString, std::endian::little>();
					case std::endian::big:
						return Generator::template generate<fccString, std::endian::big>();
				}
			}
		};

	} // namespace impl

	using impl::FourCC;
	using impl::FourCCDefaultPolicy;
	using impl::FourCCPadPolicy;
	using impl::FourCCString;

	using impl::extractFcc;
	using impl::FourCCGenerator;
	using impl::RuntimeFourCCGenerator;

} // namespace jmmt
