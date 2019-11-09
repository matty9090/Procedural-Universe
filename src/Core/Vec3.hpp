#pragma once

#include <string>
#include <sstream>

#include <d3d11.h>
#include <SimpleMath.h>

/*
	3D vector class
*/
template <class T = float> struct Vec3 {
	T x, y, z;
	
	Vec3() : x(0), y(0), z(0) {}
	Vec3(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}

	inline float Distance(Vec3 v) {
		T dx = (v.x - x) * (v.x - x);
		T dy = (v.y - y) * (v.y - y);
		T dz = (v.z - z) * (v.z - z);

		return sqrtf(dx + dy + dz);
	}

    inline float DistanceSqr(Vec3 v) {
		T dx = (v.x - x) * (v.x - x);
		T dy = (v.y - y) * (v.y - y);
		T dz = (v.z - z) * (v.z - z);

		return dx + dy + dz;
	}

	inline float Length() {
		return sqrtf(x * x + y * y + z * z);
	}

	inline Vec3 Normalised() {
		float m = Length();
		return Vec3(x / m, y / m, z / m);
	}

	inline Vec3 operator+(const Vec3& v) const {
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	inline Vec3 operator-(const Vec3& v) const {
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	inline void operator+=(Vec3 v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}

	inline Vec3 operator*(Vec3 v) {
		return Vec3(y * v.getZ() - z * v.getY(),
					z * v.getX() - x * v.getZ(),
					x * v.getY() - y * v.getX());
	}

	inline Vec3 operator*(T v) {
		return Vec3(x * v, y * v, z * v);
	}

    inline Vec3 operator/(T v) {
		return Vec3(x / v, y / v, z / v);
	}

    inline void operator*=(T v) {
		x *= v;
        y *= v;
        z *= v;
	}

    inline void operator/=(T v) {
		x /= v;
        y /= v;
        z /= v;
	}

	std::string ToString() {
		std::ostringstream ss;
		ss.precision(5);

		ss << "( " << x << ", " << y << ", " << z << ")";

		return ss.str();
	}

    DirectX::SimpleMath::Vector3 AsVector3()
    {
        return DirectX::SimpleMath::Vector3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    }
};

typedef Vec3<double> Vec3d;