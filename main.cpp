#include "library.hpp"

#include <vector>

constexpr uint32_t ImageWidth = 1920 / 2;
constexpr uint32_t ImageHeight = 1080 / 2;
constexpr uint32_t SamplesPerPixel = 64;

Scene make_scene()
{
	Scene scene;

	scene.insert_sphere({ 0.0f, 1.0f, 3.0f }, 1.0f);
	scene.insert_sphere({ -2.0f, 1.0f, 3.0f }, 1.0f);
	scene.insert_sphere({ 0.0f, 3.0f, 3.0f }, 1.0f);
	scene.insert_plane({ 0.0f, 1.0f, 0.0f }, 0.0f);

	return scene;
}

const Scene Scene = make_scene();

Color escaped(Vec3 direction)
{
	return direction * direction;
}

Color bsdf_diffuse(Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	incident = random_on_sphere();
	if (dot(incident, normal) < 0.0f) incident = incident * -1.0f;
	return Color(0.8f);
}

Color bsdf(uint32_t material, Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	switch (material)
	{
		case 0: return bsdf_diffuse(outgoing, normal, incident);
		default: break;
	}

	return Color(0.0f);
}

Color evaluate(const Ray& ray)
{
	float distance;
	Vec3 normal;
	uint32_t material;

	if (not Scene.intersect(ray, distance, normal, material)) return escaped(ray.direction);

	Vec3 outgoing = ray.direction;
	Vec3 incident;

	Color scatter = bsdf(material, outgoing, normal, incident);
	Ray new_ray = bounce(ray, distance, incident);

	scatter = scatter * std::abs(dot(normal, incident));
	if (get_luminance(scatter) < 0.01f) return Color(0.0f);
	return evaluate(new_ray) * scatter;
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
		float u = (static_cast<float>(x) + random_float() - ImageWidth / 2.0f) / ImageWidth;
		float v = (static_cast<float>(y) + random_float() - ImageHeight / 2.0f) / ImageWidth;
		result = result + render_sample(u, v);
	}

	return result / SamplesPerPixel;
}

int main()
{
	std::vector<Color> colors(ImageWidth * ImageHeight);

	parallel_for(0, ImageHeight, [&](uint32_t y)
	{
		for (uint32_t x = 0; x < ImageWidth; ++x)
		{
			Color color = render_pixel(x, y);
			size_t index = y * ImageWidth + x;
			colors[index] = color;
		}
	});

	write_image("output.png", ImageWidth, ImageHeight, colors.data());
	return 0;
}
