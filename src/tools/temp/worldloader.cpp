#include <jmmt/structs/level/octree.hpp>
#include <jmmt/structs/level/patch.hpp>

// VIF fun
#include <jmmt/ps2/vif.hpp>

#include "math.hpp"
#include "utils.hpp"

#include "worldloader.hpp"

using aOctree = jmmt::structs::level::aOctree;
using aVifPatch = jmmt::structs::level::aVifPatch;

#if 0
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
#endif

// Patch processing

// VIF/vu0 micromem emulation state.
// Each aVifPatch structure is actually an individual DMAC VIFtag,
// which needs to be unpacked into VU micro-memory, then further processed.
static jmmt::ps2::VifEmulator vif0Emu;
static u8 vu0MicroMem[jmmt::ps2::VU0_MEMORY_SIZE]{};

/// Unpacks JMMT aVifPatch to PatchNode
void unpackVifPatchToNode(PatchNode& p, const aVifPatch& patch, float scale) {
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
}

void unpackPatches(Ref<jmmt::fs::PakFileSystem> worldPak, PatchNode*& pPatchArray, u32& patchCount, const std::string& patchFileName) {
	auto patchBlob = gReadFileFromPak(worldPak, patchFileName);

	const auto patchHeader = patchBlob.cast<jmmt::structs::level::aPatchHeader>();
	const auto patchArray = patchBlob.castArrayAt<jmmt::structs::level::aVifPatch>(patchHeader->patchOffset, patchHeader->patchCount);

	// TODO: Sector & patch texture jazz

	patchCount = patchHeader->patchCount;
	pPatchArray = new PatchNode[patchHeader->patchCount];

	// Unpack patch data
	for(auto i = 0; i < patchHeader->patchCount; ++i) {
		unpackVifPatchToNode(pPatchArray[i], patchArray[i], patchHeader->scale);
	}
}

WorldLoader::~WorldLoader() {
	if(patches)
		delete[] patches;
}

void WorldLoader::loadWorld(const std::string& pakName) {
	auto worldPak = gOpenPakFile(pakName);

	// TODO: Unpack octrees
	//auto octreeBlob = gReadFileFromPak(worldPak, "TerrainGroup/octree.bin");
	//auto gOctreeArray = octreeBlob.castArray<aOctree>();

	// Unpack patche files
	unpackPatches(worldPak, domePatches, domePatchCount, "TerrainGroup/domepatch.bin");
	unpackPatches(worldPak, patches, patchCount, "TerrainGroup/patch.bin");
}
