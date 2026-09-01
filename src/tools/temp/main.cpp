#include <jmmt/structs/level/octree.hpp>
#include "utils.hpp"
#include "math.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

using aOctree = jmmt::structs::level::aOctree;

void renderOctreeNode(const aOctree& octreeNode, Color color) {
	Vector3 vMin{octreeNode.minX, octreeNode.minZ, octreeNode.minY};
	Vector3 vMax{octreeNode.maxX, octreeNode.maxZ, octreeNode.maxY};
	Aabb aabb{vMin, vMax};

	DrawCubeWires(aabb.getCenter(), aabb.getLength(), aabb.getWidth(), aabb.getHeight(), color);
	DrawCubeWires(aabb.getCenter(), 0.5f, 0.5f, 0.5f, WHITE);
}

void renderOctrees(const Blob::TypedArrayView<aOctree>& octreeData) {
	for(auto i = 1; i < octreeData.size(); ++i) {
		auto& octreeNode = octreeData[i];
		renderOctreeNode(octreeNode, RED);
	}
}

void renderScene(const Blob::TypedArrayView<aOctree>& octreeData) {
	renderOctrees(octreeData);
	DrawGrid(10, 1.f);
}

constexpr static auto kWidth = 1024;
constexpr static auto kHeight = 800;

int main(int argc, char** argv) {
	auto worldPak = gOpenPakFile(argv[1]);

	auto octreeBlob = gReadFileFromPak(worldPak, "TerrainGroup/octree.bin");
	auto octreeData = octreeBlob.castArray<aOctree>();

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

	while(!WindowShouldClose()) {
		if (IsKeyDown(KEY_F)) {
			UpdateCamera(&camera, CAMERA_FREE);
		}

		BeginDrawing();
			ClearBackground(BLACK);

			// Sidenote but I fucking hate raylib's pass by value slop
			// It's easier in C but Just Use A Fucking Pointer
			BeginMode3D(camera);
				renderScene(octreeData);
			EndMode3D();

			//DrawText("Hello World", 20, 20, 10, BLACK);
		EndDrawing();
	}


	return 0;
}
