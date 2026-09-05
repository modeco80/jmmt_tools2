#pragma once
#include <mco/base_types.hpp>
#include <string>
#include <span>

#include "math.hpp"

struct PatchNode {
	Vector3 controlPoints[16];
	Vector3 rgbPoints[16];
	Vector2 uvPoints[4];
	// TODO Need sector n texture
};

struct OctreeNode {
	Aabb aabbVolume;

	PatchNode* firstPatch;
	u32 patchCount;

	OctreeNode* sibling;
	OctreeNode* child;
};

// TODO
class WorldLoader {
	OctreeNode* domeOctreeRoot;
	PatchNode* domePatches;
	OctreeNode* octreeRoot;
	PatchNode* patches;
	u32 domeOctreeCount;
	u32 domePatchCount;
	u32 octreeCount;
	u32 patchCount;
   public:
	WorldLoader() = default;
	~WorldLoader();

	void loadWorld(const std::string& pakName);

	constexpr std::span<PatchNode> getDomePatches() const {
		return { domePatches, domePatchCount };
	}

	constexpr std::span<PatchNode> getPatches() const {
		return {patches, patchCount };
	}
};
