#include <ostream>
#ifndef VEC3F_H
#define VEC3F_H


class Vec3f {
public:
	float x;
	float y;
	float z;

	Vec3f() : x(0), y(0), z(0) {}
	//Vec3f(float a = 0) : x(a), y(a), z(a) {}
	Vec3f(float a) : x(a), y(a), z(a) {}
	Vec3f(float x2, float y2, float z2) : x(x2), y(y2), z(z2) {}

	Vec3f operator + (const Vec3f v) {
		Vec3f result;
		result.x = x + v.x;
		result.y = y + v.y;
		result.z = z + v.z;
		return result;
	}

	Vec3f operator + (const float c) {
		Vec3f result;
		result.x = x + c;
		result.y = y + c;
		result.z = z + c;
		return result;
	}

	Vec3f operator - (const Vec3f v) {
		Vec3f result;
		result.x = x - v.x;
		result.y = y - v.y;
		result.z = z - v.z;
		return result;
	}

	Vec3f operator * (const Vec3f v) {
		Vec3f result;
		result.x = x * v.x;
		result.y = y * v.y;
		result.z = z * v.z;
		return result;
	}

	Vec3f operator * (const float c) {
		Vec3f result;
		result.x = x * c;
		result.y = y * c;
		result.z = z * c;
		return result;
	}

	Vec3f operator / (const float c) {
		Vec3f result;
		result.x = x / c;
		result.y = y / c;
		result.z = z / c;
		return result;
	}

	friend std::ostream& operator<< (std::ostream& os, const Vec3f& v); 

	float dot(Vec3f v) {
		return (x * v.x) + (y * v.y) + (z * v.z);
	}

	Vec3f cross(Vec3f v) {
		Vec3f result(0);
		float xNew = (y * v.z) - (z * v.y);
		float yNew = (z * v.x) - (x * v.z);
		float zNew = (x * v.y) - (y * v.x);
		result.setEach(xNew, yNew, zNew);
		return result;
	}

	void normalize() {
		float magnitude = sqrt((x * x) + (y * y) + (z * z));
		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}

	float calculateMagnitude() {
		float magnitude = sqrt((x * x) + (y * y) + (z * z));
		return magnitude;
	}

	void setAll(float val) {
		x = val;
		y = val;
		z = val;
	}

	void setEach(float xNew, float yNew, float zNew) {
		x = xNew;
		y = yNew;
		z = zNew;
	}

	void clamp() {
		if (x > 1) x = 1;
		if (y > 1) y = 1;
		if (z > 1) z = 1;
	}

};

std::ostream& operator<<(std::ostream& os, const Vec3f& v) {

	os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
	return os;

}


#endif
