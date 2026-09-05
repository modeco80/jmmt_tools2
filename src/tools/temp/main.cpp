#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <jmmt/structs/level/octree.hpp>
#include <jmmt/structs/level/patch.hpp>
#include <set>
#include <vector>

// VIF fun
#include <jmmt/ps2/vif.hpp>
#include <jmmt/ps2/vu_float.hpp>

#include "math.hpp"
#include "utils.hpp"
#include "worldloader.hpp"

// Raylib math setup for jmmt::ps2::VuVector
// It is a template so that libjmmt itself does not
// need to provide a math type.
struct RaylibVectorTraits {
	using Vec2 = Vector2;
	using Vec3 = Vector3;
	using Vec4 = Vector4;
};
using VuVector = jmmt::ps2::VuVector<RaylibVectorTraits>;


using aOctree = jmmt::structs::level::aOctree;
using aVifPatch = jmmt::structs::level::aVifPatch;

// Octree state
Blob::TypedArrayView<aOctree> gOctreeArray;

std::vector<PatchNode> gPatches;

struct TraverseState {
	const aOctree* pNode;
	u32 depth;
};

template <class Fn>
void traverseOctreeImpl(Fn&& fn, const aOctree& rootNode) {
	std::vector<TraverseState> stackList;
	std::set<const aOctree*> set;

	// push root node
	stackList.push_back(TraverseState { &rootNode, 0 });

	while(!stackList.empty()) {
		auto current = stackList.back();
		stackList.pop_back();

		if(set.find(current.pNode) != set.end())
			continue;

		fn(*current.pNode, current.depth);
		set.insert(current.pNode);

		if(current.pNode->sibling) {
			if(current.pNode->sibling > gOctreeArray.size())
				continue;
			if(auto it = set.find(&gOctreeArray[current.pNode->sibling]); it != set.end())
				continue;
			stackList.push_back(TraverseState { &gOctreeArray[current.pNode->sibling], current.depth });
		}

		if(current.pNode->child) {
			if(current.pNode->child > gOctreeArray.size())
				continue;
			if(auto it = set.find(&gOctreeArray[current.pNode->child]); it != set.end())
				continue;
			stackList.push_back(TraverseState { &gOctreeArray[current.pNode->child], current.depth + 1 });
		}
	}
}

template <class Fn>
void traverseOctree(Fn&& fn) {
	traverseOctreeImpl(fn, gOctreeArray[1]);
}

// Patch processing

// VIF/vu0 micromem emulation state.
// Each aVifPatch structure is actually an individual DMAC VIFtag,
// which needs to be unpacked into VU micro-memory, then further processed.
static jmmt::ps2::VifEmulator vif0Emu;
static u8 vu0MicroMem[jmmt::ps2::VU0_MEMORY_SIZE]{};

// Converts jmmt aVifPatch structure to something somewhat standard
PatchNode convertPatch(const aVifPatch& patch, float scale) {
	PatchNode p;

	vif0Emu.reset();
	vif0Emu.execute(reinterpret_cast<const u8*>(&patch),  sizeof(jmmt::structs::level::aVifPatch), &vu0MicroMem[0], jmmt::ps2::VU0_MEMORY_SIZE);

	// Each thing the VIFtag dumped into VU micromem is actually fixed-point,
	// so we have to further convert it back to floating point vectors.

	for(auto i = 0; i < 16; ++i) {
		VuVector vec(reinterpret_cast<u32*>(&vu0MicroMem[0]), 0x230 + (i*16), VuVector::LANES_XYZ);
		p.controlPoints[i] = vec.toFloatXYZ<12>() * scale;
	}

	for(auto i = 0; i < 16; ++i) {
		VuVector vec(reinterpret_cast<u32*>(&vu0MicroMem[0]), 0x330 + (i*16), VuVector::LANES_XYZ);
		p.rgbPoints[i] = vec.toFloatXYZ<4>();
	}

	for(auto i = 0; i < 4; ++i) {
		VuVector vec(reinterpret_cast<u32*>(&vu0MicroMem[0]), 0x430 + (i*16), VuVector::LANES_XY);
		p.uvPoints[i] = vec.toFloatXY<12>();
	}

	return p;
}

// Rendering

std::vector<Color> treeDepthColors;

void initColors() {
	for(auto i = 0; i < 256; ++i)
		treeDepthColors.push_back(Color(rand() % 255, rand() % 255, rand() % 255, 255));
}

const Color& getColorFor(const aOctree& octreeNode) {
	// use the bbox of the octree node as a really stupid hash function
	const u32 hashKeyMin = std::bit_cast<u32>(octreeNode.minX) | std::bit_cast<u32>(octreeNode.minY) >> 16 | std::bit_cast<u32>(octreeNode.minZ) >> 20;
	const u32 hashKeyMax = std::bit_cast<u32>(octreeNode.maxX) | std::bit_cast<u32>(octreeNode.maxY) >> 16 | std::bit_cast<u32>(octreeNode.maxZ) >> 20;
	return treeDepthColors[(hashKeyMin ^ (hashKeyMax >> 2)) % treeDepthColors.size()];
}

void renderOctreeNode(const aOctree& octreeNode, u32 depth) {
	Vector3 vMin { octreeNode.minX, octreeNode.minY, octreeNode.minZ };
	Vector3 vMax { octreeNode.maxX, octreeNode.maxY, octreeNode.maxZ };
	Aabb aabb { vMin, vMax };

	DrawCubeWires(aabb.getCenter(), aabb.getLength(), aabb.getWidth(), aabb.getHeight(), getColorFor(octreeNode));
	DrawCubeWires(aabb.getCenter(), 1.f, 1.f, 1.f, WHITE);
}

void renderOctrees() {
	traverseOctree([&](const aOctree& node, u32 depth) {
		renderOctreeNode(node, depth);
	});
}

Color multipliedPatchRgbColor(const Vector3& colorPoint) {
	return Color((u8)std::clamp(colorPoint.x * 2.f, 0.f, 255.f), (u8)std::clamp(colorPoint.y * 2.f, 0.f, 255.f), (u8)std::clamp(colorPoint.z * 2.f, 0.f, 255.f),255);
}

void renderPatch(const PatchNode& patch) {
	for(i32 i = 0; i < 16; ++i) {
		DrawCubeWires(patch.controlPoints[i], 1.f, 1.f, 1.f, multipliedPatchRgbColor(patch.rgbPoints[i]));
	}
}

void renderScene() {
	for(u32 i = 0; i < gPatches.size(); ++i)
		renderPatch(gPatches[i]);
	DrawGrid(100, 5.f);
	//renderOctrees();
}

constexpr static auto kWidth = 1024;
constexpr static auto kHeight = 800;

int main(int argc, char** argv) {
	auto worldPak = gOpenPakFile(argv[1]);

	auto octreeBlob = gReadFileFromPak(worldPak, "TerrainGroup/octree.bin");
	gOctreeArray = octreeBlob.castArray<aOctree>();

	auto patchBlob = gReadFileFromPak(worldPak, "TerrainGroup/patch.bin");

	const auto pPatchHeader = patchBlob.cast<jmmt::structs::level::aPatchHeader>();
	printf("%d patches\n", pPatchHeader->patchCount);
	const auto pPatchArray = patchBlob.castArrayAt<jmmt::structs::level::aVifPatch>(pPatchHeader->patchOffset, pPatchHeader->patchCount);

	gPatches.resize(pPatchHeader->patchCount-1);
	for(auto i = 1; i < pPatchHeader->patchCount; ++i) {
		gPatches[i-1] = convertPatch(pPatchArray[i], pPatchHeader->scale);
	}

	InitWindow(kWidth, kHeight, "jmmt_tools2 world viewer");

	Camera3D camera {};
	camera.position = Vector3(10., 10., 10.);
	camera.target = Vector3 {};
	camera.up = Vector3 { 0, 0, 1 };
	camera.fovy = 45;
	camera.projection = CAMERA_PERSPECTIVE;

	DisableCursor();
	SetTargetFPS(60);

	rlSetClipPlanes(1.f, 100000.f);

	initColors();

	while(!WindowShouldClose()) {
		if(IsKeyDown(KEY_F)) {
			UpdateCamera(&camera, CAMERA_FREE);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode3D(camera);
		renderScene();
		EndMode3D();
		EndDrawing();
	}

	return 0;
}
