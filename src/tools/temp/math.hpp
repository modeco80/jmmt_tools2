#pragma once
#include <raylib.h>
#include <raymath.h>

/// An axis-aligned bounding box.
struct Aabb {
	Vector3 vMin;
	Vector3 vMax;

	Vector3 getCenter() const {
		return Vector3(
			(vMax.x + vMin.x) * 0.5f,
			(vMax.y + vMin.y) * 0.5f,
			(vMax.z + vMin.z) * 0.5f
		);
	}

	float getLength() const {
		return vMax.x - vMin.x;
	}

	float getWidth() const {
		return vMax.y - vMin.y;
	}

	float getHeight() const {
		return vMax.z - vMin.z;
	}
};
