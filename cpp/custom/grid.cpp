#include "grid.h"

#include "core/typedefs.h"
#include "preludes.h"

#include "cell.hpp"
#include "chunk.hpp"
#include "rng.hpp"

namespace Step {

void step_reaction(
		u32 &cell_material_idx,
		bool &active,
		bool &changed,
		u64 *chunk_ptr,
		i32 local_x,
		i32 local_y,
		u32 *other_ptr,
		i32 other_offset_x,
		i32 other_offset_y,
		u64 &rng) {
	u32 other_material_idx = Cell::material_idx(*other_ptr);

	bool swap;
	CellMaterial *mat;
	i32 reaction_range_idx;
	if (cell_material_idx > other_material_idx) {
		swap = true;
		mat = Grid::cell_materials + other_material_idx;
		reaction_range_idx = cell_material_idx - other_material_idx;
	} else {
		swap = false;
		mat = Grid::cell_materials + cell_material_idx;
		reaction_range_idx = other_material_idx - cell_material_idx;
	}

	if (reaction_range_idx >= mat->reaction_ranges_len) {
		return;
	}

	u64 reaction_range = *(mat->reaction_ranges + reaction_range_idx);
	u64 reaction_start = reaction_range & 0xffffffff;
	u64 reaction_end = reaction_range >> 32;

	if (reaction_start >= reaction_end) {
		return;
	}

	active = true;

	// TODO: Bulk reactions
	// for (u64 i = reaction_start; i < reaction_end; i++) {
	// 	CellReaction reaction = *(mat->reactions + i);

	// 	if (reaction.probability >= Rng::gen_32bit(rng)) {
	// 		u32 out1, out2;
	// 		if (swap) {
	// 			out1 = reaction.mat_idx_out2;
	// 			out2 = reaction.mat_idx_out1;
	// 		} else {
	// 			out1 = reaction.mat_idx_out1;
	// 			out2 = reaction.mat_idx_out2;
	// 		}

	// 		if (out1 != cell_material_idx) {
	// 			cell_material_idx = out1;
	// 			changed = true;
	// 		}

	// 		if (out2 != other_material_idx) {
	// 			Cell::set_material_idx(*other_ptr, out2);
	// 			Chunk::activate_neighbors_offset(
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					other_offset_x,
	// 					other_offset_y,
	// 					other_ptr);
	// 		}

	// 		return;
	// 	}
	// }
}

void swap_cells(
		u32 cell,
		u32 *cell_ptr,
		i32 x,
		i32 y,
		u32 *other_ptr,
		i32 other_x,
		i32 other_y) {
	*cell_ptr = *other_ptr;
	*other_ptr = cell;
	Grid::activate_neighbors(x, y, cell_ptr);
	Grid::activate_neighbors(other_x, other_y, cell_ptr);
}

void step_cell(
		u32 *cell_ptr,
		u64 *chunk_ptr,
		i32 local_x,
		i32 local_y,
		u64 &rng) {
	// u32 cell = *cell_ptr;

	// if (!Cell::is_active(cell) || Cell::is_updated(cell)) {
	// 	return;
	// }

	// bool active = false;
	// bool changed = false;

	// u32 cell_material_idx = Cell::material_idx(cell);

	// // Reactions
	// // x x x
	// // . o x
	// // . . .

	// step_reaction(
	// 		cell_material_idx,
	// 		active,
	// 		changed,
	// 		chunk_ptr,
	// 		local_x,
	// 		local_y,
	// 		cell_ptr + 1,
	// 		1,
	// 		0,
	// 		rng);

	// step_reaction(
	// 		cell_material_idx,
	// 		active,
	// 		changed,
	// 		chunk_ptr,
	// 		local_x,
	// 		local_y,
	// 		cell_ptr - Grid::width - 1,
	// 		-1,
	// 		-1,
	// 		rng);

	// step_reaction(
	// 		cell_material_idx,
	// 		active,
	// 		changed,
	// 		chunk_ptr,
	// 		local_x,
	// 		local_y,
	// 		cell_ptr - Grid::width,
	// 		0,
	// 		-1,
	// 		rng);

	// step_reaction(
	// 		cell_material_idx,
	// 		active,
	// 		changed,
	// 		chunk_ptr,
	// 		local_x,
	// 		local_y,
	// 		cell_ptr - Grid::width + 1,
	// 		1,
	// 		-1,
	// 		rng);

	// Cell::set_material_idx(cell, cell_material_idx);
	// Cell::set_updated(cell);

	// // Movement

	// CellMaterial *mat = Grid::cell_materials + cell_material_idx;

	// auto b = cell_ptr + Grid::width;
	// CellMaterial *b_mat = Grid::cell_materials + Cell::material_idx(*b);

	// auto br = cell_ptr + Grid::width + 1;
	// CellMaterial *br_mat = Grid::cell_materials + Cell::material_idx(*br);

	// auto bl = cell_ptr + Grid::width - 1;
	// CellMaterial *bl_mat = Grid::cell_materials + Cell::material_idx(*bl);

	// auto l = cell_ptr - 1;
	// CellMaterial *l_mat = Grid::cell_materials + Cell::material_idx(*l);

	// auto r = cell_ptr + 1;
	// CellMaterial *r_mat = Grid::cell_materials + Cell::material_idx(*r);

	// auto t = cell_ptr - Grid::width;
	// CellMaterial *t_mat = Grid::cell_materials + Cell::material_idx(*t);

	// auto tl = cell_ptr - Grid::width - 1;
	// CellMaterial *tl_mat = Grid::cell_materials + Cell::material_idx(*tl);

	// auto tr = cell_ptr - Grid::width + 1;
	// CellMaterial *tr_mat = Grid::cell_materials + Cell::material_idx(*tr);

	// switch (mat->cell_movement) {
	// 	case Grid::CELL_MOVEMENT_SOLID: {
	// 	} break;
	// 	case Grid::CELL_MOVEMENT_POWDER: {
	// 		if (b_mat->density < mat->density) {
	// 			swap_cells(
	// 					cell,
	// 					cell_ptr,
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					b,
	// 					0,
	// 					1);
	// 			return;
	// 		} else if (bl_mat->density < mat->density && br_mat->density < mat->density) {
	// 			if (Rng::gen_bool(rng)) {
	// 				swap_cells(
	// 						cell,
	// 						cell_ptr,
	// 						chunk_ptr,
	// 						local_x,
	// 						local_y,
	// 						bl,
	// 						-1,
	// 						1);
	// 				return;
	// 			} else {
	// 				swap_cells(
	// 						cell,
	// 						cell_ptr,
	// 						chunk_ptr,
	// 						local_x,
	// 						local_y,
	// 						br,
	// 						1,
	// 						1);
	// 				return;
	// 			}
	// 		} else if (bl_mat->density < mat->density) {
	// 			swap_cells(
	// 					cell,
	// 					cell_ptr,
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					bl,
	// 					-1,
	// 					1);
	// 			return;
	// 		} else if (br_mat->density < mat->density) {
	// 			swap_cells(
	// 					cell,
	// 					cell_ptr,
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					br,
	// 					1,
	// 					1);
	// 			return;
	// 		}
	// 	} break;
	// 	case Grid::CELL_MOVEMENT_LIQUID: {
	// 		// TODO: Movemet speed.

	// 		const u32 dissipate_chance = 8388608;

	// 		if (b_mat->density < mat->density) {
	// 			swap_cells(
	// 					cell,
	// 					cell_ptr,
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					b,
	// 					0,
	// 					1);
	// 			return;
	// 			// } else if (bl_mat->density < mat->density && br_mat->density < mat->density) {

	// 		} else if (bl_mat->density < mat->density) {
	// 			Cell::set_value(cell, 1, false);

	// 			swap_cells(
	// 					cell,
	// 					cell_ptr,
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					bl,
	// 					-1,
	// 					1);
	// 			return;
	// 		} else if (br_mat->density < mat->density) {
	// 			Cell::set_value(cell, 0, false);

	// 			swap_cells(
	// 					cell,
	// 					cell_ptr,
	// 					chunk_ptr,
	// 					local_x,
	// 					local_y,
	// 					br,
	// 					1,
	// 					1);
	// 			return;
	// 		} else if (l_mat->density < mat->density && r_mat->density < mat->density) {
	// 			if (Cell::value(cell)) {
	// 				if (Rng::gen_32bit(rng) < dissipate_chance) {
	// 					cell = 0;
	// 					changed = true;
	// 				} else {
	// 					swap_cells(
	// 							cell,
	// 							cell_ptr,
	// 							chunk_ptr,
	// 							local_x,
	// 							local_y,
	// 							l,
	// 							-1,
	// 							0);
	// 					return;
	// 				}
	// 			} else {
	// 				if (Rng::gen_32bit(rng) < dissipate_chance) {
	// 					cell = 0;
	// 					changed = true;
	// 				} else {
	// 					swap_cells(
	// 							cell,
	// 							cell_ptr,
	// 							chunk_ptr,
	// 							local_x,
	// 							local_y,
	// 							r,
	// 							1,
	// 							0);
	// 					return;
	// 				}
	// 			}
	// 		} else if (l_mat->density < mat->density) {
	// 			if (Rng::gen_32bit(rng) < dissipate_chance) {
	// 				cell = 0;
	// 				changed = true;
	// 			} else {
	// 				Cell::set_value(cell, 1, false);

	// 				swap_cells(
	// 						cell,
	// 						cell_ptr,
	// 						chunk_ptr,
	// 						local_x,
	// 						local_y,
	// 						l,
	// 						-1,
	// 						0);
	// 				return;
	// 			}
	// 		} else if (r_mat->density < mat->density) {
	// 			if (Rng::gen_32bit(rng) < dissipate_chance) {
	// 				cell = 0;
	// 				changed = true;
	// 			} else {
	// 				Cell::set_value(cell, 0, false);

	// 				swap_cells(
	// 						cell,
	// 						cell_ptr,
	// 						chunk_ptr,
	// 						local_x,
	// 						local_y,
	// 						r,
	// 						1,
	// 						0);
	// 				return;
	// 			}
	// 		}
	// 	} break;
	// 	case Grid::CELL_MOVEMENT_GAS: {
	// 		// TODO: Reverse liquid movement.
	// 	} break;
	// }

	// if (changed) {
	// 	*cell_ptr = cell;

	// 	Grid::activate_neighbors(i32 x, i32 y, cell_ptr);
	// 	Chunk::activate_neighbors(chunk_ptr, local_x, local_y, cell_ptr);
	// } else if (active) {
	// 	Cell::set_active(cell, true);
	// 	*cell_ptr = cell;

	// 	Chunk::activate_point(chunk_ptr, local_x, local_y);
	// } else {
	// 	Cell::set_active(cell, false);
	// 	*cell_ptr = cell;
	// }
}

void step_chunk(
		u64 chunk,
		u64 *chunk_ptr,
		u32 *cell_start,
		u64 &rng) {
	if (chunk == 0) {
		return;
	}

	u32 rows = Chunk::get_rows(chunk);
	auto rect = Chunk::active_rect(chunk);

	// Alternate between left and right.
	i32 x_start;
	i32 x_end;
	i32 x_step;
	if ((Grid::tick & 1) == 0) {
		x_start = rect.x_start;
		x_end = rect.x_end;
		x_step = 1;
	} else {
		x_start = rect.x_end - 1;
		x_end = rect.x_start - 1;
		x_step = -1;
	}

	// Iterate over each cell in the chunk.
	for (i32 local_y = rect.y_start; local_y < rect.y_end; local_y++) {
		if ((rows & (1u << local_y)) == 0) {
			continue;
		}

		i32 local_x = x_start;
		while (local_x != x_end) {
			auto cell_ptr = cell_start + local_x + local_y * Grid::width;
			step_cell(cell_ptr, chunk_ptr, local_x, local_y, rng);

			local_x += x_step;
		}
	}
}

void step_row(i32 row_idx) {
	u64 rng = ((u64)row_idx + (u64)Grid::tick) * 6364136223846792969uLL;

	auto next_chunk = *Grid::chunks + row_idx * Grid::chunks_width + 1;

	// Iterate over each chunk left to right.
	for (i32 i = 1; i < Grid::chunks_width - 1; i++) {
		auto chunk_ptr = Grid::chunks + row_idx * Grid::chunks_width + i;
		u64 chunk = next_chunk;
		next_chunk = chunk_ptr[1];

		auto cell_start = Grid::cells + row_idx * Grid::width * 32 + i * 32;

		step_chunk(chunk, chunk_ptr, cell_start, rng);
	}
}

void pre_step() {
	Grid::updated_bit >>= Cell::Shifts::SHIFT_UPDATED;
	Grid::updated_bit %= 3;
	Grid::updated_bit += 1;
	Grid::updated_bit <<= Cell::Shifts::SHIFT_UPDATED;

	Grid::tick++;
}

} // namespace Step

void Grid::_bind_methods() {
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("delete_grid"),
			&Grid::delete_grid);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("new_empty", "wish_width", "wish_height"),
			&Grid::new_empty);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("get_size"),
			&Grid::get_size);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("get_size_chunk"),
			&Grid::get_size_chunk);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("get_cell_data", "image_size", "rect"),
			&Grid::get_cell_data);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("set_cell_rect", "rect", "cell_material_idx"),
			&Grid::set_cell_rect);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("set_cell", "position", "cell_material_idx"),
			&Grid::set_cell);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("set_border_cell", "position", "cell_material_idx"),
			&Grid::set_border_cell);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("step_manual"),
			&Grid::step_manual);

	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("get_tick"),
			&Grid::get_tick);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("get_cell_material_idx", "position"),
			&Grid::get_cell_material_idx);

	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("init_materials", "num_materials"),
			&Grid::init_materials);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD(
					"add_material",
					"cell_movement",
					"density",
					"durability",
					"cell_collision",
					"friction",
					"reactions",
					"idx"),
			&Grid::add_material);

	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("get_seed"),
			&Grid::get_seed);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("set_seed", "seed"),
			&Grid::set_seed);

	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("is_chunk_active", "position"),
			&Grid::is_chunk_active);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("free_memory"),
			&Grid::free_memory);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("print_materials"),
			&Grid::print_materials);
	ClassDB::bind_static_method(
			"Grid",
			D_METHOD("run_tests"),
			&Grid::run_tests);

	// ADD_GROUP("Test group", "group_");
	// ADD_SUBGROUP("Test subgroup", "group_subgroup_");

	BIND_ENUM_CONSTANT(CELL_COLLISION_SOLID);
	BIND_ENUM_CONSTANT(CELL_COLLISION_PLATFORM);
	BIND_ENUM_CONSTANT(CELL_COLLISION_LIQUID);
	BIND_ENUM_CONSTANT(CELL_COLLISION_NONE);

	BIND_ENUM_CONSTANT(CELL_MOVEMENT_SOLID);
	BIND_ENUM_CONSTANT(CELL_MOVEMENT_POWDER);
	BIND_ENUM_CONSTANT(CELL_MOVEMENT_LIQUID);
	BIND_ENUM_CONSTANT(CELL_MOVEMENT_GAS);
}

void Grid::delete_grid() {
	if (cells != nullptr) {
		print_line("Deleting grid");

		delete[] cells;
		cells = nullptr;
		width = 0;
		height = 0;

		delete[] border_cells;
		border_cells = nullptr;

		delete[] chunks;
		chunks = nullptr;
		chunks_width = 0;
		chunks_height = 0;
	}
}

void Grid::new_empty(i32 wish_width, i32 wish_height) {
	delete_grid();

	// Make sure that the height is a multiple of 64/8.
	// This is to avoid mutably sharing cache lines between threads.
	chunks_width = CLAMP(wish_width / 32 + 7, 8, 2048) & ~8;
	chunks_height = CLAMP(wish_height / 32, 3, 2048);
	chunks = new u64[chunks_width * chunks_height];
	// Set all chunk to active.
	for (i32 i = 0; i < chunks_width * chunks_height; i++) {
		chunks[i] = ~0uLL;
	}

	width = chunks_width * 32;
	height = chunks_height * 32;
	cells = new u32[width * height];
	// Set all cells to empty and active.
	for (i32 i = 0; i < width * height; i++) {
		cells[i] = Cell::Masks::MASK_ACTIVE;
	}

	border_cells = new u32[height * 32];
	// Set all border cells to empty.
	for (i32 i = 0; i < height * 32; i++) {
		border_cells[i] = 0;
	}
}

Vector2i Grid::get_size() {
	return Vector2i(width, height);
}

Vector2i Grid::get_size_chunk() {
	return Vector2i(chunks_width, chunks_height);
}

Ref<Image> Grid::get_cell_data(Vector2i image_size, Rect2i rect) {
	// TODO: Try to use set_cell instead to avoid creating a new buffer.
	auto image_data = PackedByteArray();
	image_data.resize(image_size.x * image_size.y * 4);
	auto image_buffer = reinterpret_cast<u32 *>(image_data.ptrw());

	// TODO: Could be optimized by handling oob separately.
	for (i32 img_y = 0; img_y < MIN(image_size.y, rect.size.y); img_y++) {
		const i32 cell_y = rect.position.y + img_y;

		for (i32 img_x = 0; img_x < MIN(image_size.x, rect.size.x); img_x++) {
			const i32 cell_x = rect.position.x + img_x;

			image_buffer[img_y * image_size.x + img_x] = get_cell_checked(cell_x, cell_y);
		}
	}

	return Image::create_from_data(
			image_size.x,
			image_size.y,
			false,
			Image::FORMAT_RF,
			image_data);
}

u32 Grid::get_cell_checked(i32 x, i32 y) {
	if (cells == nullptr) {
		return 0;
	} else if (y < 0) {
		return border_cells[0];
	} else if (y >= height) {
		return border_cells[(height - 1) * 32];
	} else if (x < 0 || x >= width) {
		return border_cells[y * 32 + (x & 31)];
	}

	return cells[y * width + x];
}

void Grid::activate_neighbors(i32 x, i32 y, u32 *cell_ptr) {
	Chunk::activate_point(x - 1, y - 1);
	Chunk::activate_point(x, y - 1);
	Chunk::activate_point(x + 1, y - 1);

	Chunk::activate_point(x - 1, y);
	// Chunk::activate_point(x, y);
	Chunk::activate_point(x + 1, y);

	Chunk::activate_point(x - 1, y + 1);
	Chunk::activate_point(x, y + 1);
	Chunk::activate_point(x + 1, y + 1);

	cell_ptr[-width - 1] |= Cell::Masks::MASK_ACTIVE;
	cell_ptr[-width] |= Cell::Masks::MASK_ACTIVE;
	cell_ptr[-width + 1] |= Cell::Masks::MASK_ACTIVE;

	cell_ptr[-1] |= Cell::Masks::MASK_ACTIVE;
	cell_ptr[0] |= Cell::Masks::MASK_ACTIVE;
	cell_ptr[1] |= Cell::Masks::MASK_ACTIVE;

	cell_ptr[width - 1] |= Cell::Masks::MASK_ACTIVE;
	cell_ptr[width] |= Cell::Masks::MASK_ACTIVE;
	cell_ptr[width + 1] |= Cell::Masks::MASK_ACTIVE;
}

void Grid::set_cell_rect(Rect2i rect, u32 cell_material_idx) {
	ERR_FAIL_COND_MSG(cells == nullptr, "Grid is not initialized");

	rect = rect.intersection(Rect2i(32, 32, width - 64, height - 64));
	if (rect.size.x <= 0 || rect.size.y <= 0) {
		// Empty rect.
		return;
	}

	for (i32 y = rect.position.y; y < rect.get_end().y; y++) {
		for (i32 x = rect.position.x; x < rect.get_end().x; x++) {
			auto cell_ptr = cells + y * width + x;

			assert(cell_ptr >= cells);
			assert(cell_ptr < cells + width * height);

			Cell::set_material_idx(*cell_ptr, cell_material_idx);
		}
	}

	for (i32 y = rect.position.y - 1; y < rect.get_end().y + 1; y++) {
		for (i32 x = rect.position.x - 1; x < rect.get_end().x + 1; x++) {
			auto cell_ptr = cells + y * width + x;

			assert(cell_ptr >= cells);
			assert(cell_ptr < cells + width * height);

			Cell::set_active(*cell_ptr, true);
			Chunk::activate_point(x, y);
		}
	}
}

void Grid::set_cell(Vector2i position, u32 cell_material_idx) {
	ERR_FAIL_COND_MSG(cells == nullptr, "Grid is not initialized");

	if (position.x < 32 || position.x >= width - 32 || position.y < 32 || position.y >= height - 32) {
		return;
	}

	auto cell_ptr = cells + position.y * width + position.x;
	*cell_ptr = cell_material_idx;

	activate_neighbors(position.x, position.y, cell_ptr);
}

void Grid::set_border_cell(Vector2i position, u32 cell_material_idx) {
	ERR_FAIL_COND_MSG(cells == nullptr, "Grid is not initialized");

	if (position.x < 0 || position.x >= 32 || position.y < 0 || position.y >= height) {
		return;
	}

	border_cells[position.y * 32 + position.x] = cell_material_idx;
}

void Grid::step_manual() {
	ERR_FAIL_COND_MSG(cells == nullptr, "Grid is not initialized");

	Step::pre_step();

	for (i32 row_idx = 1; row_idx < chunks_height - 1; row_idx++) {
		Step::step_row(row_idx);
	}
}

void delete_materials() {
	if (Grid::cell_materials != nullptr) {
		print_line("Deleting materials");

		for (i32 i = 0; i < Grid::cell_materials_len; i++) {
			CellMaterial *mat = Grid::cell_materials + i;
			if (mat->reaction_ranges_len > 0) {
				delete[] mat->reaction_ranges;
				delete[] mat->reactions;
			}
		}

		delete[] Grid::cell_materials;
		Grid::cell_materials = nullptr;
		Grid::cell_materials_len = 0;
	}
}

void Grid::init_materials(i32 num_materials) {
	delete_materials();

	if (num_materials > 0) {
		cell_materials = new CellMaterial[num_materials];
		cell_materials_len = num_materials;
	}
}

void Grid::add_material(
		int cell_movement,
		int density,
		int durability,
		int cell_collision,
		float friction,
		// probability, out1, out2
		Array reactions,
		i32 idx) {
	ERR_FAIL_COND_MSG(cell_materials == nullptr, "Materials not initialized");

	CellMaterial mat = CellMaterial();
	mat.cell_movement = cell_movement;
	mat.density = density;
	mat.durability = durability;
	mat.cell_collision = cell_collision;
	mat.friction = friction;

	int num_reaction = 0;
	for (int i = 0; i < reactions.size(); i++) {
		Array r = reactions[i];
		if (!r.is_empty()) {
			mat.reaction_ranges_len = i + 1;
			num_reaction += r.size();
		}
	}
	if (mat.reaction_ranges_len != 0) {
		assert(num_reaction > 0);

		mat.reaction_ranges = new u64[mat.reaction_ranges_len];
		mat.reactions = new CellReaction[num_reaction];

		u64 next_reaction_idx = 0;

		assert(mat.reaction_ranges_len <= reactions.size());
		for (int i = 0; i < mat.reaction_ranges_len; i++) {
			u64 reactions_start = next_reaction_idx;

			assert(reactions.size() >= i);
			Array reactions_with = reactions[i];
			for (int j = 0; j < reactions_with.size(); j++) {
				Array reaction_data = reactions_with[j];
				assert(reaction_data.size() == 3);
				CellReaction reaction = {
					reaction_data[0],
					reaction_data[1],
					reaction_data[2]
				};
				assert(next_reaction_idx < num_reaction);
				mat.reactions[next_reaction_idx] = reaction;

				next_reaction_idx++;
			}

			assert(i < mat.reaction_ranges_len);
			u64 reactions_end = next_reaction_idx;
			if (reactions_start == reactions_end) {
				mat.reaction_ranges[i] = 0;
			} else {
				mat.reaction_ranges[i] = reactions_start | (reactions_end << 32);
			}
		}

		assert(next_reaction_idx == num_reaction);
	}

	assert(idx < cell_materials_len);
	cell_materials[idx] = mat;
}

bool Grid::is_chunk_active(Vector2i position) {
	if (position.x < 0 || position.y < 0 || position.x >= chunks_width || position.y >= chunks_height) {
		return false;
	}

	return *(chunks + position.x * chunks_height + position.y) != 0;
}

void Grid::set_seed(i64 new_seed) {
	Grid::seed = (u64)new_seed;
}

i64 Grid::get_seed() {
	return (i64)seed;
}

void Grid::free_memory() {
	delete_materials();
	delete_grid();
}

i64 Grid::get_tick() {
	return tick;
}

u32 Grid::get_cell_material_idx(Vector2i position) {
	return Cell::material_idx(get_cell_checked(position.x, position.y));
}

void Grid::print_materials() {
	print_line("num materials: ", cell_materials_len);

	for (i32 i = 0; i < cell_materials_len; i++) {
		CellMaterial &mat = cell_materials[i];
		print_line("-----------", i, "-----------");
		print_line("cell_movement ", mat.cell_movement);
		print_line("density ", mat.density);
		print_line("durability ", mat.durability);
		print_line("cell_collision ", mat.cell_collision);
		print_line("friction ", mat.friction);

		print_line("reaction_ranges_len ", mat.reaction_ranges_len);
		for (i32 j = 0; j < mat.reaction_ranges_len; j++) {
			print_line("   reaction_range ", j);
			u64 reaction_range = *(mat.reaction_ranges + j);
			u64 reaction_start = reaction_range & 0xffffffff;
			u64 reaction_end = reaction_range >> 32;
			print_line("       reaction_start ", reaction_start);
			print_line("       reaction_end ", reaction_end);
			for (i32 k = reaction_start; k < reaction_end; k++) {
				CellReaction reaction = *(mat.reactions + k);
				print_line("          reaction ", k);
				print_line("          in1 ", i);
				print_line("          in2 ", i + j);
				print_line("          probability ", reaction.probability);
				print_line("          out1 ", reaction.mat_idx_out1);
				print_line("          out2 ", reaction.mat_idx_out2);
			}
		}
	}
}

namespace Test {

void test_activate_rect() {
	u64 *chunk = new u64;
	*chunk = 0;

	Chunk::unsafe_activate_rect(*chunk, 0, 0, 32, 32);
	assert(*chunk == ~0uLL);
	print_line("activate full rect: OK");

	u64 rng = 12345789;
	for (i32 i = 0; i < 1000; i++) {
		*chunk = 0;

		u32 x_offset = Rng::gen_range_32bit(rng, 0, 32);
		u32 y_offset = Rng::gen_range_32bit(rng, 0, 32);
		u64 width = Rng::gen_range_32bit(rng, 0, 32 - x_offset) + 1;
		u64 height = Rng::gen_range_32bit(rng, 0, 32 - y_offset) + 1;

		Chunk::unsafe_activate_rect(*chunk, x_offset, y_offset, width, height);

		assert(Chunk::active_rect(*chunk).x_start == x_offset);
		assert(Chunk::active_rect(*chunk).y_start == y_offset);
		assert(Chunk::active_rect(*chunk).x_end == x_offset + width);
		assert(Chunk::active_rect(*chunk).y_end == y_offset + height);
	}
	print_line("activate random rects: OK");

	delete chunk;
}

void test_rng() {
	u32 num_tests = 100000;
	u32 num_true = 0;

	u64 rng = 12345789;

	for (u32 i = 0; i < num_tests; i++) {
		if (Rng::gen_bool(rng)) {
			num_true++;
		}
	}
	f64 true_bias = (f64)num_true / (f64)num_tests;
	print_line("rng true bias ", true_bias);
	assert(true_bias > 0.45 && true_bias < 0.55);
	assert(true_bias != 0.5);

	print_line("rng non-bias: OK");
}

} // namespace Test

void Grid::run_tests() {
	print_line("---------- test_activate_rect: STARTED");
	Test::test_activate_rect();

	print_line("---------- test_rng: STARTED");
	Test::test_rng();

	print_line("---------- All tests passed!");
}
