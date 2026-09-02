#include <jmmt/structs/level/octree.hpp>
#include "utils.hpp"
#include "math.hpp"

#include <vector>
#include <set>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

using aOctree = jmmt::structs::level::aOctree;

// Octree state
Blob::TypedArrayView<aOctree> gOctreeArray;

struct TraverseState {
	const aOctree* pNode;
	u32 depth;
};

template<class Fn>
void traverseOctreeImpl(Fn&& fn, const aOctree& octreeNode) {
	std::vector<TraverseState> stackList;
	std::set<const aOctree*> set;

	// push root node
	stackList.push_back(TraverseState{ &octreeNode, 0});

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
			stackList.push_back(TraverseState { &gOctreeArray[current.pNode->sibling], current.depth});
		}

		if(current.pNode->child) {
			if(current.pNode->child > gOctreeArray.size())
				continue;
			if(auto it = set.find(&gOctreeArray[current.pNode->child]); it != set.end())
				continue;
			stackList.push_back(TraverseState {&gOctreeArray[current.pNode->child], current.depth + 1 });
		}
	}
}

template<class Fn>
void traverseOctree(Fn&& fn) {
	traverseOctreeImpl(fn, gOctreeArray[1]);
}

// Rendering

std::vector<Color> treeDepthColors;

void initColors() {
	for(auto i = 0; i < 256; ++i)
		treeDepthColors.push_back(Color(rand() % 255, rand() % 255, rand() % 255, 255));
}

void renderOctreeNode(const aOctree& octreeNode, u32 depth) {
	Vector3 vMin{octreeNode.minX, octreeNode.minZ, octreeNode.minY};
	Vector3 vMax{octreeNode.maxX, octreeNode.maxZ, octreeNode.maxY};
	Aabb aabb{vMin, vMax};

	DrawCubeWires(aabb.getCenter(), aabb.getLength(), aabb.getWidth(), aabb.getHeight(), treeDepthColors[depth % treeDepthColors.size()]);
	DrawCubeWires(aabb.getCenter(), 1.f, 1.f, 1.f, WHITE);
}

void renderOctrees() {
	traverseOctree([&](const aOctree& node, u32 depth) {
		renderOctreeNode(node, depth);
	});
}

void renderScene() {
	renderOctrees();
	DrawGrid(10, 1.f);
}

constexpr static auto kWidth = 1024;
constexpr static auto kHeight = 800;

int main(int argc, char** argv) {
	auto worldPak = gOpenPakFile(argv[1]);

	auto octreeBlob = gReadFileFromPak(worldPak, "TerrainGroup/octree.bin");
	gOctreeArray = octreeBlob.castArray<aOctree>();

	InitWindow(kWidth, kHeight, "jmmt_tools2 world viewer");

	Camera3D camera{};
	camera.position = Vector3(10., 10. , 10.);
	camera.target = Vector3{};
	camera.up = Vector3{0, 1, 0};
	camera.fovy = 45;
	camera.projection = CAMERA_PERSPECTIVE;

	DisableCursor();
	SetTargetFPS(60);

	rlSetClipPlanes(1.f, 100000.f);

	initColors();

	while(!WindowShouldClose()) {
		if (IsKeyDown(KEY_F)) {
			UpdateCamera(&camera, CAMERA_FREE);
		}

		BeginDrawing();
			ClearBackground(BLACK);

			// Sidenote but I fucking hate raylib's pass by value slop
			// It's easier in C but Just Use A Fucking Pointer
			BeginMode3D(camera);
				renderScene();
			EndMode3D();

			//DrawText("Hello World", 20, 20, 10, BLACK);
		EndDrawing();
	}


	return 0;
}
