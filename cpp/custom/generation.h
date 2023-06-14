#ifndef GENERATION_H
#define GENERATION_H

#include "preludes.h"

#include "modules/noise/fastnoise_lite.h"
#include "scene/resources/curve.h"

class Generation : public Object {
	GDCLASS(Generation, Object);

protected:
	static void _bind_methods();

public:
	static void surface_pass(
			u32 rock,
			u32 dirt,
			i32 surface_top,
			i32 surface_bot);

	static void cavern_pass(
			Ref<Curve> horizontal_gradient,
			Ref<Curve> vertical_gradient,
			Ref<FastNoiseLite> cavern,
			f32 cavern_x_scale,
			i32 surface_top,
			f32 cavern_threshold);
};

#endif