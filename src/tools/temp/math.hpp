#pragma once
#include <raylib.h>
#include <raymath.h>

#include <jmmt/ps2/vu_float.hpp>

/// An axis-aligned bounding box.
struct Aabb {
	Vector3 vMin;
	Vector3 vMax;

	Vector3 getCenter() const {
		return Vector3(
		(vMax.x + vMin.x) * 0.5f,
		(vMax.y + vMin.y) * 0.5f,
		(vMax.z + vMin.z) * 0.5f);
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

// Raylib vector traits for jmmt::ps2::VuVector
// It is a template so that libjmmt itself does not
// need to provide a math type.
struct RaylibVectorTraits {
	using Vec2 = Vector2;
	using Vec3 = Vector3;
	using Vec4 = Vector4;
};

using VuVector = jmmt::ps2::VuVector<RaylibVectorTraits>;
