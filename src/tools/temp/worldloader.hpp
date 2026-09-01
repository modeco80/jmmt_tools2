#pragma once
#include <mco/base_types.hpp>
#include "math.hpp"

struct PatchNode {
	float controlPoints[16];
	// TODO:
	// RGB
	// Texture UV
};

struct OctreeNode {
	Aabb aabbVolume;

	PatchNode* firstPatch;
	u32 patchCount;

	OctreeNode* sibling;
	OctreeNode* child;
};

// TODO
class WorldParser
{
public:
};
