#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <algorithm>

#include "worldloader.hpp"

Color multipliedPatchRgbColor(const Vector3& colorPoint) {
	return Color((u8)std::clamp(colorPoint.x * 2.f, 0.f, 255.f), (u8)std::clamp(colorPoint.y * 2.f, 0.f, 255.f), (u8)std::clamp(colorPoint.z * 2.f, 0.f, 255.f),255);
}

void renderPatch(const PatchNode& patch) {
	for(i32 i = 0; i < 16; ++i) {
		DrawCubeWires(patch.controlPoints[i], 1.f, 1.f, 1.f, multipliedPatchRgbColor(patch.rgbPoints[i]));
	}
}

void renderPatches(std::span<PatchNode> patches) {
	for(auto& patch : patches) {
		renderPatch(patch);
	}
}

void renderScene(WorldLoader& world) {
	renderPatches(world.getDomePatches());
	renderPatches(world.getPatches());
	DrawGrid(100, 5.f);
}

constexpr static auto kWidth = 1024;
constexpr static auto kHeight = 800;

int main(int argc, char** argv) {
	WorldLoader worldLoader;

	worldLoader.loadWorld(argv[1]);

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

	while(!WindowShouldClose()) {
		if(IsKeyDown(KEY_F)) {
			UpdateCamera(&camera, CAMERA_FREE);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		BeginMode3D(camera);
		renderScene(worldLoader);
		EndMode3D();
		EndDrawing();
	}

	return 0;
}
