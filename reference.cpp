#define COMPILE_REFERENCE
#ifdef COMPILE_REFERENCE

#include "library.hpp"

#include <vector>

constexpr uint32_t ImageWidth = 1920 / 2;
constexpr uint32_t ImageHeight = 1080 / 2;
constexpr uint32_t SamplesPerPixel = 640;

Scene make_scene()
{
	Scene scene;

	scene.insert_sphere({ 0.0f, 1.0f, 3.0f }, 1.0f, 2);
	scene.insert_sphere({ -2.0f, 1.0f, 3.0f }, 1.0f, 1);
	scene.insert_sphere({ 0.0f, 3.0f, 3.0f }, 0.3f, 3);
	scene.insert_plane({ 0.0f, 1.0f, 0.0f }, 0.0f);
	scene.insert_box({ 2.0f, 1.1f, 3.0f }, Vec3(2.0f, 2.0f, 0.5f), 2);

	return scene;
}

const Scene Scene = make_scene();

Color bsdf_lambertian_reflection(Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	//Uniform sampling
	incident = random_on_sphere();
	float uniform_hemisphere_pdf = 1.0f / Pi / 2.0f;

	//Importance sampling based on cosine distribution (tried shortcut here, it doesn't actually work!!)
	//	incident = normal + random_on_sphere();
	//	float cosine_hemisphere_pdf = abs_dot(incident, normal) / Pi;

	make_same_side(outgoing, normal, incident);
	float evaluated = 1.0f / Pi;
	float pdf = uniform_hemisphere_pdf;

	if (almost_zero(pdf)) return Color();
	return Color(evaluated / pdf);
}

Color bsdf_specular_reflection(Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	incident = reflect(outgoing, normal);
	float correction = abs_dot(incident, normal);
	if (almost_zero(correction)) return Color();
	return Color(1.0f / correction);
}

Color bsdf_specular_fresnel(Vec3 outgoing, Vec3 normal, Vec3& incident, float eta)
{
	float cos_o = dot(outgoing, normal);
	if (cos_o < 0.0f) eta = 1.0f / eta;

	float cos_i = fresnel_cos_i(eta, cos_o);
	float evaluated = fresnel_value(eta, cos_o, cos_i);

	//Uniform sampling
	//	if (random_float() < 0.5f)
	//	{
	//		//Specular reflection
	//		incident = normalize(reflect(outgoing, normal));
	//	}
	//	else
	//	{
	//		//Specular transmission
	//		evaluated = 1.0f - evaluated;
	//		incident = fresnel_refract(eta, cos_i, outgoing, normal);
	//	}
	//
	//	float correction = abs_dot(incident, normal);
	//	if (almost_zero(correction)) return Color();
	//	return Color(evaluated / correction / 0.5f);

	//Importance sampling
	if (random_float() < evaluated) incident = normalize(reflect(outgoing, normal));
	else incident = fresnel_refract(eta, cos_i, outgoing, normal);

	float correction = abs_dot(incident, normal);
	if (almost_zero(correction)) return Color();
	return Color(1.0f / correction);
}

Color bsdf(uint32_t material, Vec3 outgoing, Vec3 normal, Vec3& incident)
{
	switch (material)
	{
		case 0: return bsdf_lambertian_reflection(outgoing, normal, incident) * 0.5f;
		case 1: return bsdf_specular_reflection(outgoing, normal, incident) * 0.8f;
		case 2: return bsdf_specular_fresnel(outgoing, normal, incident, 1.0f / 1.5f) * 0.9f;
		default: break;
	}

	return Color(0.0f);
}

Color emit(uint32_t material)
{
	switch (material)
	{
		case 3: return Color(1.0f);
		default: break;
	}

	return Color(0.0f);
}

Color escape(Vec3 direction)
{
	return direction * direction;
}

Color evaluate(const Ray& ray, uint32_t depth)
{
	if (depth == 0) return escape(ray.direction);

	float distance;
	Vec3 normal;
	uint32_t material;

	if (not Scene.intersect(ray, distance, normal, material)) return escape(ray.direction);

	Vec3 outgoing = -ray.direction;
	Vec3 incident;

	Color scatter = bsdf(material, outgoing, normal, incident);
	Color emission = emit(material);

	Ray new_ray = bounce(ray, distance, incident);
	float lambertian = abs_dot(normal, incident);
	return emission + scatter * evaluate(new_ray, depth - 1) * lambertian;
}

Color render_sample(float u, float v)
{
	Ray ray;
	ray.origin = Vec3(0.0f, 1.5f, -3.0f);
	ray.direction = normalize(Vec3(u, v, 1.0f));
	return evaluate(ray, 128);
}

Color render_pixel(uint32_t x, uint32_t y)
{
	Color result;
	uint32_t count = 0;

	for (uint32_t i = 0; i < SamplesPerPixel; ++i)
	{
		float u = (static_cast<float>(x) + random_float() - ImageWidth / 2.0f) / ImageWidth;
		float v = (static_cast<float>(y) + random_float() - ImageHeight / 2.0f) / ImageWidth;

		Color sample = render_sample(u, v);
		if (is_invalid(sample)) continue;
		result = result + sample;
		++count;
	}

	return result / static_cast<float>(count);
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

#endif
