#include <cstring>

#include <jmmt/ps2/vif.hpp>
#include <jmmt/ps2/vu_float.hpp>

#include <jmmt/structs/level/patch.hpp>
#include "utils.hpp"

// ignore dependency on any math library for this test
struct testVector2 {
	float x;
	float y;

	constexpr testVector2(float x, float y)
	: x(x), y(y) {}
};

struct testVector3 {
	float x;
	float y;
	float z;

	constexpr testVector3(float x, float y, float z)
		: x(x), y(y), z(z) {}
};

struct TestTraits {
	using Vec2 = testVector2;
	using Vec3 = testVector3;
};

using testVuVector = jmmt::ps2::VuVector<TestTraits>;

int main() {
	auto worldPak = gOpenPakFile("TR_training.pak");

	auto octreeBlob = gReadFileFromPak(worldPak, "TerrainGroup/patch.bin");

	const auto pPatchHeader = octreeBlob.cast<jmmt::structs::level::aPatchHeader>();
	printf("%d patches\n", pPatchHeader->patchCount);
	const auto pPatchArray = octreeBlob.castArrayAt<jmmt::structs::level::aVifPatch>(pPatchHeader->patchOffset, pPatchHeader->patchCount);

	jmmt::ps2::VifEmulator vif0Emu;
	u8 vu0MicroMem[jmmt::ps2::VU0_MEMORY_SIZE]{};

	for(auto i = 0; i < pPatchArray.size(); ++i) {
		printf("patch %d {\n", i);
		// Disassemble the VIFtag
		auto* testPacket = reinterpret_cast<const u8*>(&pPatchArray[i]);
		//jmmt::ps2::vifDisassemble(&testPacket[0], sizeof(jmmt::structs::level::aVifPatch));

		// Run the patch VIFtag, unpacking into
		vif0Emu.execute(&testPacket[0],  sizeof(jmmt::structs::level::aVifPatch), &vu0MicroMem[0], jmmt::ps2::VU0_MEMORY_SIZE);

		printf("control = [");
		for(auto i = 0; i < 16; ++i) {
			if((i % 4) == 0)
				printf("\n\t");
			testVuVector vec(reinterpret_cast<u32*>(&vu0MicroMem[0]), 0x230 + (i*16), testVuVector::LANES_XYZ);
			auto convertedVector = vec.toFloatXYZ<12>();
			//printf("vec[%d @ %04x]: (%f, %f, %f)\n", i, 0x230 + (i*16), convertedVector.x, convertedVector.y, convertedVector.z);
			printf("(%0.4f, %0.4f, %0.4f) ", convertedVector.x * pPatchHeader->scale, convertedVector.y * pPatchHeader->scale, convertedVector.z * pPatchHeader->scale);
		}
		printf("\n]\n");

		printf("rgb = [");
		for(auto i = 0; i < 16; ++i) {
			if((i % 4) == 0)
				printf("\n\t");
			testVuVector vec(reinterpret_cast<u32*>(&vu0MicroMem[0]), 0x330 + (i*16), testVuVector::LANES_XYZ);
			auto convertedVector = vec.toFloatXYZ<4>();
			//printf("rgbvec[%d @ %04x]: (%f, %f, %f)\n", i, 0x330 + (i*16), convertedVector.x, convertedVector.y, convertedVector.z);

			printf("(%0.4f, %0.4f, %0.4f) ", convertedVector.x, convertedVector.y, convertedVector.z);
		}
		printf("\n]\n");

		printf("uv = [\n");
		for(auto i = 0; i < 4; ++i) {
			testVuVector vec(reinterpret_cast<u32*>(&vu0MicroMem[0]), 0x430 + (i*16), testVuVector::LANES_XY);
			auto convertedVector = vec.toFloatXY<12>();
			//printf("uv[%d @ %04x]: (%f, %f)\n", i, 0x430 + (i*16), convertedVector.x, convertedVector.y);
			printf("\t(%f, %f)\n", convertedVector.x, convertedVector.y);
		}
		printf("]\n");

		printf("}\n");
		// reset the VU0 micromem and VIF emulator
		std::memset(&vu0MicroMem[0], 0, jmmt::ps2::VU0_MEMORY_SIZE);
		vif0Emu.reset();
	}

	return 0;
}
