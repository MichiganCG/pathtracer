#pragma once

#include "stb_image_write.h"

#include <cstdint>
#include <string>
#include <cmath>
#include <functional>

using Float = float;

struct Vec3
{
	Vec3(Float x, Float y, Float z) : x(x), y(y), z(z) {}
	explicit Vec3(Float value = 0.0f) : x(value), y(value), z(value) {}

	Float x, y, z;
};

struct Ray
{
	Ray(Vec3 origin, Vec3 direction) : origin(origin), direction(direction) {}
	Ray() = default;

	Vec3 origin, direction;
};

class Scene
{
public:
	Scene() = default;

	void insert_sphere(Vec3 center, Float radius, uint32_t material = 0)
	{
		spheres.emplace_back(center, radius, material);
	}

	void insert_plane(Vec3 normal, Float offset, uint32_t material = 0)
	{
		planes.emplace_back(normal, offset, material);
	}

	/**
	 * Finds whether a ray intersects with a scene.
	 * @return Whether the intersection occurred.
	 * If intersected, also outputs the travelled distance, and surface normal and material.
	 */
	bool intersect(const Ray& ray, Float& distance, Vec3& normal, uint32_t& material) const;

private:
	std::vector<std::tuple<Vec3, Float, uint32_t>> spheres;
	std::vector<std::tuple<Vec3, Float, uint32_t>> planes;
};

using Color = Vec3;

inline Vec3 operator+(Vec3 value, Vec3 other) { return { value.x + other.x, value.y + other.y, value.z + other.z }; }
inline Vec3 operator-(Vec3 value, Vec3 other) { return { value.x - other.x, value.y - other.y, value.z - other.z }; }
inline Vec3 operator*(Vec3 value, Vec3 other) { return { value.x * other.x, value.y * other.y, value.z * other.z }; }
inline Vec3 operator/(Vec3 value, Vec3 other) { return { value.x / other.x, value.y / other.y, value.z / other.z }; }
inline Vec3 operator*(Vec3 value, Float other) { return { value.x * other, value.y * other, value.z * other }; }
inline Vec3 operator/(Vec3 value, Float other) { return { value.x / other, value.y / other, value.z / other }; }
inline Vec3 operator+(Vec3 value) { return { +value.x, +value.y, +value.z }; }
inline Vec3 operator-(Vec3 value) { return { -value.x, -value.y, -value.z }; }

inline Float dot(Vec3 value, Vec3 other) { return value.x * other.x + value.y * other.y + value.z * other.z; }
inline Float magnitude_squared(Vec3 value) { return dot(value, value); }
inline Float magnitude(Vec3 value) { return std::sqrt(magnitude_squared(value)); }
inline Vec3 normalize(Vec3 value) { return value * (Float(1.0) / magnitude(value)); }
inline Vec3 reflect(Vec3 value, Vec3 normal) { return normal * (2.0f * dot(value, normal)) - value; }
/**
 * @return A random floating point value between 0 (inclusive) and 1 (exclusive).
 */
Float random_float();

/**
 * @return A random point in the volume of a unit sphere.
 */
inline Vec3 random_in_sphere()
{
	Vec3 result;

	do
	{
		result.x = random_float();
		result.y = random_float();
		result.z = random_float();
		result = result * 2.0f - Vec3(1.0f);
	}
	while (magnitude_squared(result) > 1.0f);

	return result;
}

/**
 * @return A random point on the surface of a unit sphere.
 */
inline Vec3 random_on_sphere() { return normalize(random_in_sphere()); }

/**
 * Bounces an old ray to form a new ray.
 * @param ray The old ray.
 * @param distance Distance on the old ray to start the new ray.
 * @param direction The direction of the new ray.
 */
inline Ray bounce(const Ray& ray, Float distance, Vec3 direction)
{
	Vec3 point = ray.origin + ray.direction * distance;
	point = point + direction * 1E-4f; //Avoids shadow acne problem
	return { point, direction };
}

/**
 * Flips incident to be on the same side of a surface as outgoing.
 * @param normal The normal vector that describes the surface.
 */
inline void make_same_side(Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	Float dot_o = dot(outgoing, normal);
	Float dot_i = dot(incident, normal);

	//Flip the vector to be on the same side as normal
	if (dot_o * dot_i < 0.0f)
	{
		incident = reflect(-incident, normal);
	}
}

/**
 * Returns the luminance value of a color.
 * This can be thought of as the visually perceived brightness.
 */
inline Float get_luminance(Color color) { return dot(color, { 0.212671f, 0.715160f, 0.072169f }); }

/**
 * Executes an action in parallel, taking advantage of multiple threads.
 * @param begin The first index to execute (inclusive).
 * @param end One past the last index to execute (exclusive).
 * @param action The action to execute in parallel.
 */
void parallel_for(uint32_t begin, uint32_t end, const std::function<void(uint32_t)>& action);

/**
 * Outputs a series of colors as a PNG image file.
 */
void write_image(const std::string& filename, uint32_t width, uint32_t height, const Color* colors);
