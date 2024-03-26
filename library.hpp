#pragma once

#include "stb_image_write.h"

#include <stdexcept>
#include <cstdint>
#include <string>
#include <vector>
#include <random>
#include <cmath>

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

struct Sphere
{
	Sphere(Vec3 center, Float radius) : center(center), radius(radius) {}

	Vec3 center;
	Float radius;
};

using Color = Vec3;

inline Vec3 operator+(Vec3 value, Vec3 other) { return { value.x + other.x, value.y + other.y, value.z + other.z }; }
inline Vec3 operator-(Vec3 value, Vec3 other) { return { value.x - other.x, value.y - other.y, value.z - other.z }; }
inline Vec3 operator*(Vec3 value, Vec3 other) { return { value.x * other.x, value.y * other.y, value.z * other.z }; }
inline Vec3 operator/(Vec3 value, Vec3 other) { return { value.x / other.x, value.y / other.y, value.z / other.z }; }
inline Vec3 operator*(Vec3 value, Float other) { return { value.x * other, value.y * other, value.z * other }; }
inline Vec3 operator/(Vec3 value, Float other) { return { value.x / other, value.y / other, value.z / other }; }

inline Float dot(Vec3 value, Vec3 other) { return value.x * other.x + value.y * other.y + value.z * other.z; }
inline Float magnitude_squared(Vec3 value) { return dot(value, value); }
inline Float magnitude(Vec3 value) { return std::sqrt(magnitude_squared(value)); }
inline Vec3 normalize(Vec3 value) { return value * (Float(1.0) / magnitude(value)); }

/**
 * @return A random floating point value between 0 (inclusive) and 1 (exclusive).
 */
inline Float random_float()
{
	static std::default_random_engine random(42);
	std::uniform_real_distribution<Float> distribution;
	return distribution(random);
}

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
 * Finds whether a ray intersects with a sphere.
 * Also outputs the intersection distance and surface normal if an intersection occurred.
 * @return Whether the intersection occurred.
 */
inline bool intersect_sphere(const Ray& ray, const Sphere& sphere, Vec3& normal, Float& distance)
{
	Vec3 offset = ray.origin - sphere.center;
	Float radius2 = sphere.radius * sphere.radius;
	Float center = -dot(offset, ray.direction);

	Float extend2 = center * center + radius2 - magnitude_squared(offset);
	if (extend2 < Float(0.0)) return false;

	Float extend = std::sqrt(extend2);
	distance = center - extend;
	if (distance < Float(0.0)) distance = center + extend;
	if (distance < Float(0.0)) return false;

	normal = normalize(ray.direction * distance + offset);
	return true;
}

/**
 * Outputs a series of colors as a PNG image file.
 */
inline void write_image(const std::string& filename, uint32_t width, uint32_t height, const Color* colors)
{
	std::vector<uint8_t> data;
	data.reserve(width * height * 3);

	auto convert_single = [](float value)
	{
		//Gamma correction and clamp
		float corrected = std::sqrt(std::max(Float(0.0), std::min(value, Float(1.0))));
		return static_cast<uint8_t>(corrected * std::numeric_limits<uint8_t>::max());
	};

	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			auto& color = colors[(height - y - 1) * width + x];
			data.push_back(convert_single(color.x));
			data.push_back(convert_single(color.y));
			data.push_back(convert_single(color.z));
		}
	}

	auto casted_width = static_cast<int>(width);
	auto casted_height = static_cast<int>(height);
	int result = stbi_write_png(filename.c_str(), casted_width, casted_height, 3, data.data(), 0);
	if (result == 0) throw std::runtime_error("Error in when outputting image.");
}
