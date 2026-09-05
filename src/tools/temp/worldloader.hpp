#pragma once
#include <mco/base_types.hpp>
#include <string>

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
	OctreeNode* octreeRoot;
	PatchNode* patches;
	u32 octreeCount;
	u32 patchCount;
   public:
	WorldLoader() = default;
	~WorldLoader();

	void loadWorld(const std::string& pakName);

	u32 getPatchCount() const { return patchCount; }
	PatchNode* getPatches() const {
		return patches;
	}
};
