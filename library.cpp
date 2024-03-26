#include "library.hpp"

#include <stdexcept>
#include <vector>
#include <random>
#include <thread>
#include <atomic>

using Random = std::default_random_engine;
thread_local std::unique_ptr<Random> thread_random;

static Random* make_random_engine(uint32_t seed)
{
	auto random = std::make_unique<Random>(seed);
	Random* result = random.get();
	thread_random = std::move(random);
	return result;
}

Float random_float()
{
	Random* random = thread_random.get();
	if (random == nullptr) random = make_random_engine(0);
	std::uniform_real_distribution<Float> distribution;
	return distribution(*random);
}

static float intersect_sphere(const Ray& ray, Vec3 center, Float radius, Vec3& normal)
{
	Vec3 offset = ray.origin - center;
	Float radius2 = radius * radius;
	Float mapped = -dot(offset, ray.direction);

	Float extend2 = mapped * mapped + radius2 - magnitude_squared(offset);
	if (extend2 < Float(0.0)) return std::numeric_limits<Float>::infinity();

	Float extend = std::sqrt(extend2);
	Float distance = mapped - extend;
	if (distance < Float(0.0)) distance = mapped + extend;
	if (distance < Float(0.0)) return std::numeric_limits<Float>::infinity();

	normal = normalize(ray.direction * distance + offset);
	return distance;
}

static float intersect_plane(const Ray& ray, Vec3 normal, Float offset)
{
	Float mapped = dot(ray.direction, normal);

	if (std::abs(mapped) > 1E-4f)
	{
		Float distance = dot(ray.origin, normal);
		distance = (distance + offset) / -mapped;
		if (distance >= Float(0.0f)) return distance;
	}

	return std::numeric_limits<Float>::infinity();
}

bool Scene::intersect(const Ray& ray, Float& distance, Vec3& normal, uint32_t& material) const
{
	distance = std::numeric_limits<float>::infinity();

	for (auto& sphere : spheres)
	{
		Vec3 center = std::get<0>(sphere);
		Float radius = std::get<1>(sphere);

		Vec3 new_normal;
		float new_distance = intersect_sphere(ray, center, radius, new_normal);

		if (new_distance < distance)
		{
			distance = new_distance;
			normal = new_normal;
			material = std::get<2>(sphere);
		}
	}

	for (auto& plane : planes)
	{
		Vec3 new_normal = std::get<0>(plane);
		Float offset = std::get<1>(plane);
		float new_distance = intersect_plane(ray, new_normal, offset);

		if (new_distance < distance)
		{
			distance = new_distance;
			normal = new_normal;
			material = std::get<2>(plane);
		}
	}

	return std::isfinite(distance);
}

void parallel_for(uint32_t begin, uint32_t end, const std::function<void(uint32_t)>& action)
{
	if (end == begin) return;
	if (end < begin) std::swap(begin, end);

	uint32_t workers = std::thread::hardware_concurrency();
	workers = std::min(std::max(workers, 1U), end - begin);

	std::vector<std::thread> threads;
	std::atomic<uint32_t> current = begin;

	for (uint32_t i = 0; i < workers; ++i)
	{
		auto entry = [i, end, &current, &action]()
		{
			make_random_engine(i);

			while (true)
			{
				uint32_t index = current++;
				if (index >= end) break;
				action(index);
			}
		};

		threads.emplace_back(entry);
	}

	for (auto& thread : threads) thread.join();
}

void write_image(const std::string& filename, uint32_t width, uint32_t height, const Color* colors)
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
