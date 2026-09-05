#pragma once
#include <mco/base_types.hpp>

namespace jmmt::ps2 {

	template<u32 bitOffset>
	inline constexpr float itofImpl(u32 value) {
		float f32Value = static_cast<float>(static_cast<i32>(value));
		if(bitOffset)
			f32Value *= std::bit_cast<float>(0x3f800000 - (bitOffset << 23));
		return f32Value;
	}

	inline float itof4(u32 value) { return itofImpl<4>(value); }
	inline float itof12(u32 value) { return itofImpl<12>(value); }

	/// Helper class which loads a vector lane from VU micromem and converts it
	/// back to floating-point.
	template<class TVectorTraits>
	class VuVector {
		u32 lanes[4]{};
	public:
		enum LANE {
			LANE_X = (1<<1),
			LANE_Y = (1<<2),
			LANE_Z = (1<<3),
			LANE_W = (1<<4),

			// common lane specs
			LANES_XYZ = LANE_X | LANE_Y | LANE_Z,
			LANES_XYZW = LANE_X | LANE_Y | LANE_Z | LANE_W,
			LANES_XY = LANE_X | LANE_Y
		};

	private:
		void lqFromVuMem(u32* pVUMem, u32 qwAddr, i32 lanespec) {
			if(lanespec & LANE_X) lanes[0] = pVUMem[(qwAddr/4)];
			if(lanespec & LANE_Y) lanes[1] = pVUMem[(qwAddr/4)+1];
			if(lanespec & LANE_Z) lanes[2] = pVUMem[(qwAddr/4)+2];
			if(lanespec & LANE_W) lanes[3] = pVUMem[(qwAddr/4)+3];
		}
	public:
		VuVector() = default;
		VuVector(u32* pVUMem, u32 qwAddr, i32 lanespec) {
			lqFromVuMem(pVUMem, qwAddr, lanespec);
		}

		template<u32 bitOffset>
		constexpr typename TVectorTraits::Vec3 toFloatXYZ() const {
			return typename TVectorTraits::Vec3(itofImpl<bitOffset>(lanes[0]), itofImpl<bitOffset>(lanes[1]), itofImpl<bitOffset>(lanes[2]));
		}

		template<u32 bitOffset>
		constexpr typename TVectorTraits::Vec2 toFloatXY() const {
			return typename TVectorTraits::Vec2(itofImpl<bitOffset>(lanes[0]), itofImpl<bitOffset>(lanes[1]));
		}
	};

}
