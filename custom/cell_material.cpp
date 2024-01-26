#include "cell_material.h"
#include "core/math/vector2i.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "core/variant/typed_array.h"
#include "preludes.h"
#include "rng.hpp"
#include <unordered_map>
#include <vector>

std::unordered_map<const void *, std::vector<u32>> CellMaterial::material_tags = {};
std::unordered_map<const void *, u32> CellMaterial::material_ids = {};
std::vector<Ref<CellMaterial>> CellMaterial::materials = {};

void CellMaterial::_bind_methods() {
	ClassDB::bind_static_method(
			"CellMaterial",
			D_METHOD("add_material", "material"),
			&CellMaterial::add_material);

	ClassDB::bind_static_method(
			"CellMaterial",
			D_METHOD("find_material_idx", "material_id"),
			&CellMaterial::find_material_idx);
	ClassDB::bind_static_method(
			"CellMaterial",
			D_METHOD("find_material", "material_id"),
			&CellMaterial::find_material);
	ClassDB::bind_static_method(
			"CellMaterial",
			D_METHOD("get_material", "material_idx"),
			&CellMaterial::get_material);

	ClassDB::bind_method(D_METHOD("set_values_image", "value"), &CellMaterial::set_values_image);
	ClassDB::bind_method(D_METHOD("get_values_image"), &CellMaterial::get_values_image);

	ClassDB::bind_method(D_METHOD("set_material_id", "value"), &CellMaterial::set_material_id);
	ClassDB::bind_method(D_METHOD("get_material_id"), &CellMaterial::get_material_id);

	ClassDB::bind_method(D_METHOD("set_tags", "value"), &CellMaterial::set_tags);
	ClassDB::bind_method(D_METHOD("get_tags"), &CellMaterial::get_tags);

	ClassDB::bind_method(D_METHOD("get_material_idx"), &CellMaterial::get_material_idx);

	ClassDB::bind_method(D_METHOD("set_collision", "value"), &CellMaterial::set_collision);
	ClassDB::bind_method(D_METHOD("get_collision"), &CellMaterial::get_collision);

	BIND_ENUM_CONSTANT(CellCollision::COLLISION_NONE);
	BIND_ENUM_CONSTANT(CellCollision::COLLISION_SOLID);
	BIND_ENUM_CONSTANT(CellCollision::COLLISION_PLATFORM);
	BIND_ENUM_CONSTANT(CellCollision::COLLISION_LIQUID);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "values_image", PROPERTY_HINT_RESOURCE_TYPE, "Image"), "set_values_image", "get_values_image");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "material_id"), "set_material_id", "get_material_id");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "tags"), "set_tags", "get_tags");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision", PROPERTY_HINT_ENUM, "None,Top,Bottom,Left,Right"), "set_collision", "get_collision");
}

void CellMaterial::add_material(Ref<CellMaterial> value) {
	value->material_idx = materials.size();
	materials.push_back(value);

	// Add material id to material idx.
	material_ids[value->material_id.data_unique_pointer()] = value->material_idx;

	// Add tag to material idx.
	value->tags.push_back(value->material_id);
	for (int i = 0; i < value->tags.size(); i++) {
		StringName tag = value->tags[i];
		if (auto it = material_tags.find(tag.data_unique_pointer()); it != material_tags.end()) {
			it->second.push_back(value->material_idx);
		} else {
			material_tags[tag.data_unique_pointer()] = { value->material_idx };
		}
	}
}

u32 CellMaterial::find_material_idx(StringName material_id) {
	if (auto it = material_ids.find(material_id.data_unique_pointer()); it != material_ids.end()) {
		return it->second;
	} else {
		return 0;
	}
}

Ref<CellMaterial> CellMaterial::find_material(StringName material_id) {
	if (auto it = material_ids.find(material_id.data_unique_pointer()); it != material_ids.end()) {
		return materials[it->second];
	} else {
		return Ref<CellMaterial>();
	}
}

Ref<CellMaterial> CellMaterial::get_material(u32 material_idx) {
	if (material_idx < materials.size()) {
		return materials[material_idx];
	} else {
		return Ref<CellMaterial>();
	}
}

void CellMaterial::set_material_id(StringName value) {
	material_id = value;
}

StringName CellMaterial::get_material_id() {
	return material_id;
}

void CellMaterial::set_tags(TypedArray<StringName> value) {
	tags = value;
}

TypedArray<StringName> CellMaterial::get_tags() {
	return tags;
}

u32 CellMaterial::get_material_idx() {
	return material_idx;
}

void CellMaterial::set_collision(CellCollision value) {
	collision = value;
}

CellCollision CellMaterial::get_collision() {
	return collision;
}

void CellMaterial::set_values_image(Ref<Image> value) {
	values_image = value;
	values_width = value->get_width();
	values_height = value->get_height();
	// todo: copy values from img
	values.resize(values_width * values_height);
}

Ref<Image> CellMaterial::get_values_image() {
	return values_image;
}

u32 CellMaterial::get_value_at(const Vector2i coord, Rng &rng) {
	u32 value = 0;

	if (values_width != 0) {
		u32 x = coord.x % values_width;
		u32 y = coord.y % values_height;
		value = (u32)values[x + y * values_width];
	}

	if (max_value_noise != 0) {
		value = rng.gen_range_u32(0, max_value_noise);
		value = MIN(value, 15u);
	}

	return value;
}

u32 CellMaterial::get_hue_at(const Vector2i coord, Rng &rng) {
	// todo
	return 0;
}

void CellMaterial::print() {
	print_line("-----------", material_idx, "-----------");

	// todo

	// print_line("durability ", durability);

	// print_line("cell_collision ", collision);
	// print_line("friction ", friction);

	// print_line("cell_biome ", biome_contribution);

	// print_line("can_change_hue ", can_color);
	// print_line("values_width ", values_width);
	// print_line("values_height ", values_height);
	// print_line("max_value_noise ", max_value_noise);

	// print_line("density ", density);
	// print_line("movement_vertical_step ", movement_vertical_step);
	// print_line("movement_chance ", movement_chance);
	// print_line("horizontal_movement ", horizontal_movement);
}