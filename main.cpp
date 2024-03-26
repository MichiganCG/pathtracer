#include "library.hpp"

constexpr uint32_t ImageWidth = 1920 / 4;
constexpr uint32_t ImageHeight = 1080 / 4;
constexpr uint32_t SamplesPerPixel = 16;

bool intersect(const Ray& ray, Vec3& normal, float& distance)
{
	static const std::vector<Sphere> Scene =
		{
			Sphere({ 0.0f, 1.0f, 3.0f }, 1.0f),
			Sphere({ -2.0f, 1.0f, 3.0f }, 1.0f),
			Sphere({ 0.0f, 3.0f, 3.0f }, 1.0f),
			Sphere({ 0.0f, -100.0f, 0.0f }, 100.0f)
		};

	distance = std::numeric_limits<float>::infinity();

	for (const Sphere& sphere : Scene)
	{
		Vec3 new_normal;
		float new_distance;
		if (not intersect_sphere(ray, sphere, new_normal, new_distance)) continue;
		if (new_distance >= distance) continue;

		normal = new_normal;
		distance = new_distance;
	}

	return std::isfinite(distance);
}

Color bsdf(Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	incident = random_on_sphere();
	if (dot(incident, normal) < 0.0f) incident = incident * -1.0f;
	return Color(0.8f);
}

Color evaluate(const Ray& ray)
{
	Vec3 normal;
	float distance;
	if (not intersect(ray, normal, distance)) return ray.direction * ray.direction;

	Vec3 point = ray.origin + ray.direction * distance;
	Vec3 outgoing = ray.direction;
	Vec3 incident;

	Color scatter = bsdf(outgoing, normal, incident);
	float lambertian = std::abs(dot(normal, incident));

	Ray new_ray;
	new_ray.origin = point + incident * 0.0001f;
	new_ray.direction = normalize(incident);
	return evaluate(new_ray) * scatter * lambertian;
}

Color render_sample(float u, float v)
{
	Ray ray;
	ray.origin = Vec3(0.0f, 1.0f, -3.0f);
	ray.direction = normalize(Vec3(u, v, 1.0f));
	return evaluate(ray);
}

Color render_pixel(uint32_t x, uint32_t y)
{
	Color result;

	for (uint32_t i = 0; i < SamplesPerPixel; ++i)
	{
		float u = (static_cast<float>(x) + random_float() - static_cast<float>(ImageWidth) / 2.0f) / ImageWidth;
		float v = (static_cast<float>(y) + random_float() - static_cast<float>(ImageHeight) / 2.0f) / ImageWidth;
		result = result + render_sample(u, v);
	}

	return result / SamplesPerPixel;
}

int main()
{
	std::vector<Color> colors;

	for (uint32_t y = 0; y < ImageHeight; ++y)
	{
		for (uint32_t x = 0; x < ImageWidth; ++x)
		{
			Color color = render_pixel(x, y);
			colors.push_back(color);
		}
	}

	write_image("output.png", ImageWidth, ImageHeight, colors.data());
	return 0;
}
