#include "grid_iter.h"
#include "cell.hpp"
#include "cell_material.hpp"
#include "core/error/error_macros.h"
#include "core/math/rect2.h"
#include "core/math/rect2i.h"
#include "core/math/vector2i.h"
#include "grid.h"
#include "preludes.h"

void handle_darken(u32 &cell, u32 new_cell_darken_max) {
	if (new_cell_darken_max > 0) {
		Cell::set_darken(cell, Grid::temporal_rng.gen_range_u32(0, new_cell_darken_max));
	}
}

void GridChunkIter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("next"), &GridChunkIter::next);

	ClassDB::bind_method(D_METHOD("set_material_idx", "material_idx"), &GridChunkIter::set_material_idx);
	ClassDB::bind_method(D_METHOD("get_material_idx"), &GridChunkIter::get_material_idx);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &GridChunkIter::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &GridChunkIter::get_color);

	ClassDB::bind_method(D_METHOD("fill_remaining", "material_idx"), &GridChunkIter::fill_remaining);

	ClassDB::bind_method(D_METHOD("reset_iter"), &GridChunkIter::reset_iter);

	ClassDB::bind_method(D_METHOD("chunk_coord"), &GridChunkIter::chunk_coord);
	ClassDB::bind_method(D_METHOD("local_coord"), &GridChunkIter::local_coord);
	ClassDB::bind_method(D_METHOD("coord"), &GridChunkIter::coord);

	ClassDB::bind_method(D_METHOD("randb"), &GridChunkIter::randb);
	ClassDB::bind_method(D_METHOD("randb_probability", "probability"), &GridChunkIter::randb_probability);
	ClassDB::bind_method(D_METHOD("randf"), &GridChunkIter::randf);
	ClassDB::bind_method(D_METHOD("randf_range", "min", "max"), &GridChunkIter::randf_range);
	ClassDB::bind_method(D_METHOD("randi_range", "min", "max"), &GridChunkIter::randi_range);
}

bool GridChunkIter::next() {
	if (chunk == nullptr) {
		is_valid = false;
	} else {
		is_valid = cell_iter.next();
	}

	return is_valid;
}

void GridChunkIter::set_material_idx(u32 material_idx) {
	ERR_FAIL_COND_MSG(!is_valid, "next should return true before using iter");
	ERR_FAIL_COND_MSG(material_idx >= Grid::cell_materials.size(), "material_idx must be less than cell_materials.size");

	CellMaterial &mat = Grid::cell_materials[material_idx];
	if (mat.noise_darken_max > 0) {
		Cell::set_darken(material_idx, rng.gen_range_u32(0, mat.noise_darken_max));
	}
	chunk->set_cell(cell_iter.coord, material_idx);
	modified = true;
}

u32 GridChunkIter::get_material_idx() {
	ERR_FAIL_COND_V_MSG(!is_valid, 0, "next should return true before using iter");

	return Cell::material_idx(chunk->get_cell(cell_iter.coord));
}

void GridChunkIter::set_color(u32 color) {
	ERR_FAIL_COND_MSG(!is_valid, "next should return true before using iter");

	u32 *cell_ptr = chunk->get_cell_ptr(cell_iter.coord);

	const CellMaterial &mat = Grid::get_cell_material(Cell::material_idx(*cell_ptr));
	if (mat.can_color) {
		Cell::set_color(*cell_ptr, color);
	}
}

u32 GridChunkIter::get_color() {
	ERR_FAIL_COND_V_MSG(!is_valid, 0, "next should return true before using iter");

	return Cell::color(chunk->get_cell(cell_iter.coord));
}

void GridChunkIter::fill_remaining(u32 material_idx) {
	ERR_FAIL_COND_MSG(material_idx >= Grid::cell_materials.size(), "material_idx must be less than cell_materials.size");

	CellMaterial &mat = Grid::cell_materials[material_idx];

	if (mat.noise_darken_max > 0) {
		while (next()) {
			u32 cell = material_idx;
			Cell::set_darken(cell, rng.gen_range_u32(0, mat.noise_darken_max));
			chunk->set_cell(cell_iter.coord, cell);
		}
	} else {
		while (next()) {
			chunk->set_cell(cell_iter.coord, material_idx);
		}
	}

	modified = true;
}

void GridChunkIter::reset_iter() {
	cell_iter = Iter2D(Vector2i(0, 0), Vector2i(32, 32));
	is_valid = false;
}

Vector2i GridChunkIter::chunk_coord() {
	return _chunk_coord;
}

Vector2i GridChunkIter::local_coord() {
	if (is_valid) {
		return cell_iter.coord;
	} else {
		return Vector2i(0, 0);
	}
}

Vector2i GridChunkIter::coord() {
	return chunk_coord() * 32 + local_coord();
}

bool GridChunkIter::randb() {
	return rng.gen_bool();
}

bool GridChunkIter::randb_probability(f32 probability) {
	return rng.gen_probability_f32(probability);
}

f32 GridChunkIter::randf() {
	return rng.gen_f32();
}

f32 GridChunkIter::randf_range(f32 min, f32 max) {
	return rng.gen_range_f32(min, max);
}

i32 GridChunkIter::randi_range(i32 min, i32 max) {
	return rng.gen_range_i32(min, max);
}

void GridChunkIter::set_chunk(Vector2i chunk_coord) {
	modified = false;
	is_valid = false;
	_chunk_coord = chunk_coord;
	cell_iter = Iter2D(Vector2i(0, 0), Vector2i(32, 32));
	chunk = Grid::get_chunk(chunk_coord);
	rng = Grid::get_static_rng(chunk_coord);
}

void GridChunkIter::activate() {
	if (modified) {
		IterChunk _chunk_iter = IterChunk(Rect2(
				_chunk_coord * 32 - Vector2i(1, 1),
				Vector2i(34, 34)));

		while (_chunk_iter.next()) {
			Chunk *c = Grid::get_chunk(_chunk_iter.chunk_coord);
			if (c == nullptr) {
				continue;
			}

			TEST_ASSERT(_chunk_iter.local_rect().has_area(), "local_rect has no area");
			c->activate_rect(_chunk_iter.local_rect());

			Iter2D _cell_iter = _chunk_iter.local_iter();
			while (_cell_iter.next()) {
				u32 *cell = c->get_cell_ptr(_cell_iter.coord);
				Cell::set_active(*cell, true);
			}
		}
	}
}

void GridRectIter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("next"), &GridRectIter::next);

	ClassDB::bind_method(D_METHOD("set_material_idx", "material_idx"), &GridRectIter::set_material_idx);
	ClassDB::bind_method(D_METHOD("get_material_idx"), &GridRectIter::get_material_idx);

	ClassDB::bind_method(D_METHOD("set_color", "color"), &GridRectIter::set_color);
	ClassDB::bind_method(D_METHOD("get_color"), &GridRectIter::get_color);

	ClassDB::bind_method(D_METHOD("fill_remaining", "material_idx"), &GridRectIter::fill_remaining);

	ClassDB::bind_method(D_METHOD("reset_iter"), &GridRectIter::reset_iter);

	ClassDB::bind_method(D_METHOD("chunk_coord"), &GridRectIter::chunk_coord);
	ClassDB::bind_method(D_METHOD("local_coord"), &GridRectIter::local_coord);
	ClassDB::bind_method(D_METHOD("coord"), &GridRectIter::coord);
}

bool GridRectIter::next() {
	if (cell_iter.next()) {
		is_valid = true;
	} else {
		while (chunk_iter.next()) {
			chunk = Grid::get_chunk(chunk_iter.chunk_coord);
			if (chunk == nullptr) {
				continue;
			}

			cell_iter = chunk_iter.local_iter();
			if (cell_iter.next()) {
				is_valid = true;
				return true;
			}
		}

		is_valid = false;
	}

	return is_valid;
}

void GridRectIter::set_material_idx(u32 material_idx) {
	ERR_FAIL_COND_MSG(!is_valid, "next should return true before using iter");
	ERR_FAIL_COND_MSG(material_idx >= Grid::cell_materials.size(), "material_idx must be less than cell_materials.size");

	CellMaterial &mat = Grid::cell_materials[material_idx];
	if (mat.noise_darken_max > 0) {
		Cell::set_darken(material_idx, Grid::temporal_rng.gen_range_u32(0, mat.noise_darken_max));
	}

	chunk->set_cell(cell_iter.coord, material_idx);
	modified = true;
}

u32 GridRectIter::get_material_idx() {
	ERR_FAIL_COND_V_MSG(!is_valid, 0, "next should return true before using iter");

	return Cell::material_idx(chunk->get_cell(cell_iter.coord));
}

void GridRectIter::set_color(u32 color) {
	ERR_FAIL_COND_MSG(!is_valid, "next should return true before using iter");

	u32 *cell_ptr = chunk->get_cell_ptr(cell_iter.coord);

	const CellMaterial &mat = Grid::get_cell_material(Cell::material_idx(*cell_ptr));
	if (mat.can_color) {
		Cell::set_color(*cell_ptr, color);
	}
}

u32 GridRectIter::get_color() {
	ERR_FAIL_COND_V_MSG(!is_valid, 0, "next should return true before using iter");

	return Cell::color(chunk->get_cell(cell_iter.coord));
}

void GridRectIter::fill_remaining(u32 material_idx) {
	ERR_FAIL_COND_MSG(material_idx >= Grid::cell_materials.size(), "material_idx must be less than cell_materials.size");

	CellMaterial &mat = Grid::cell_materials[material_idx];

	if (mat.noise_darken_max > 0) {
		while (next()) {
			u32 cell = material_idx;
			Cell::set_darken(cell, Grid::temporal_rng.gen_range_u32(0, mat.noise_darken_max));
			chunk->set_cell(cell_iter.coord, cell);
		}
	} else {
		while (next()) {
			chunk->set_cell(cell_iter.coord, material_idx);
		}
	}

	modified = true;
}

void GridRectIter::reset_iter() {
	chunk_iter.reset();
	cell_iter = Iter2D();
	chunk = nullptr;
	is_valid = false;
}

Vector2i GridRectIter::chunk_coord() {
	return chunk_iter.chunk_coord;
}

Vector2i GridRectIter::local_coord() {
	if (is_valid) {
		return cell_iter.coord;
	} else {
		return Vector2i(0, 0);
	}
}

Vector2i GridRectIter::coord() {
	return chunk_coord() * 32 + local_coord();
}

void GridRectIter::set_rect(Rect2i rect) {
	modified = false;
	is_valid = false;
	chunk_iter = IterChunk(rect);
	cell_iter = Iter2D();
	chunk = nullptr;
}

void GridRectIter::activate() {
	if (modified) {
		Vector2i start = chunk_iter._start.coord() - Vector2i(1, 1);
		IterChunk _chunk_iter = IterChunk(Rect2i(
				start,
				chunk_iter._end.coord() + Vector2i(1, 1) - start));

		while (_chunk_iter.next()) {
			Chunk *c = Grid::get_chunk(_chunk_iter.chunk_coord);
			if (c == nullptr) {
				continue;
			}

			// if (!_chunk_iter.local_rect().has_area()) {
			// 	print_line(_chunk_iter.local_rect());
			// 	print_line(_chunk_iter._start.chunk_coord);
			// 	print_line(_chunk_iter._start.local_coord);
			// 	print_line(_chunk_iter._end.chunk_coord);
			// 	print_line(_chunk_iter._end.local_coord);
			// }

			TEST_ASSERT(_chunk_iter.local_rect().has_area(), "local_rect has no area");
			c->activate_rect(_chunk_iter.local_rect());

			Iter2D _cell_iter = _chunk_iter.local_iter();
			while (_cell_iter.next()) {
				u32 *cell = c->get_cell_ptr(_cell_iter.coord);
				Cell::set_active(*cell, true);
			}
		}
	}
}

void GridLineIter::_bind_methods() {
	ClassDB::bind_method(D_METHOD("next"), &GridLineIter::next);

	ClassDB::bind_method(D_METHOD("set_material_idx", "material_idx"), &GridLineIter::set_material_idx);
	ClassDB::bind_method(D_METHOD("get_material_idx"), &GridLineIter::get_material_idx);

	ClassDB::bind_method(D_METHOD("get_color"), &GridLineIter::get_color);
	ClassDB::bind_method(D_METHOD("set_color", "color"), &GridLineIter::set_color);

	ClassDB::bind_method(D_METHOD("fill_remaining", "material_idx"), &GridLineIter::fill_remaining);

	ClassDB::bind_method(D_METHOD("reset_iter"), &GridLineIter::reset_iter);

	ClassDB::bind_method(D_METHOD("coord"), &GridLineIter::coord);
}

bool GridLineIter::next() {
	while (line_iter.next()) {
		ChunkLocalCoord new_coord = ChunkLocalCoord(line_iter.currenti() + start);
		if (new_coord.chunk_coord != current.chunk_coord) {
			chunk = Grid::get_chunk(new_coord.chunk_coord);
		}
		current = new_coord;

		if (chunk == nullptr) {
			continue;
		} else {
			is_valid = true;
			return true;
		}
	}

	is_valid = false;

	return false;
}

void GridLineIter::set_material_idx(u32 material_idx) {
	ERR_FAIL_COND_MSG(!is_valid, "next should return true before using iter");
	ERR_FAIL_COND_MSG(material_idx >= Grid::cell_materials.size(), "material_idx must be less than cell_materials.size");

	CellMaterial &mat = Grid::cell_materials[material_idx];
	if (mat.noise_darken_max > 0) {
		Cell::set_darken(material_idx, Grid::temporal_rng.gen_range_u32(0, mat.noise_darken_max));
	}

	chunk->set_cell(current.local_coord, material_idx);

	// Activate neighboring cells.
	IterChunk _chunk_iter = IterChunk(Rect2(
			current.coord() - Vector2i(1, 1),
			Vector2i(3, 3)));
	while (_chunk_iter.next()) {
		Chunk *c;
		if (_chunk_iter.chunk_coord == current.chunk_coord) {
			c = chunk;
		} else {
			c = Grid::get_chunk(_chunk_iter.chunk_coord);
		}

		if (c == nullptr) {
			continue;
		}

		TEST_ASSERT(_chunk_iter.local_rect().has_area(), "local_rect has no area");
		c->activate_rect(_chunk_iter.local_rect());

		Iter2D _cell_iter = _chunk_iter.local_iter();
		while (_cell_iter.next()) {
			u32 *cell = c->get_cell_ptr(_cell_iter.coord);
			Cell::set_active(*cell, true);
		}
	}
}

u32 GridLineIter::get_material_idx() {
	ERR_FAIL_COND_V_MSG(!is_valid, 0, "next should return true before using iter");

	return Cell::material_idx(chunk->get_cell(current.local_coord));
}

void GridLineIter::set_color(u32 color) {
	ERR_FAIL_COND_MSG(!is_valid, "next should return true before using iter");

	u32 *cell_ptr = chunk->get_cell_ptr(current.local_coord);

	const CellMaterial &mat = Grid::get_cell_material(Cell::material_idx(*cell_ptr));
	if (mat.can_color) {
		Cell::set_color(*cell_ptr, color);
	}
}

u32 GridLineIter::get_color() {
	ERR_FAIL_COND_V_MSG(!is_valid, 0, "next should return true before using iter");

	return Cell::color(chunk->get_cell(current.local_coord));
}

void GridLineIter::fill_remaining(u32 material_idx) {
	ERR_FAIL_COND_MSG(material_idx >= Grid::cell_materials.size(), "material_idx must be less than cell_materials.size");

	while (next()) {
		set_material_idx(material_idx);
	}
}

void GridLineIter::reset_iter() {
	line_iter.reset();
	chunk = nullptr;
	is_valid = false;
}

Vector2i GridLineIter::coord() {
	return current.coord();
}

void GridLineIter::set_line(Vector2i p_start, Vector2i end) {
	is_valid = false;
	start = p_start;
	line_iter = IterLine(end - start);
	current = ChunkLocalCoord(start);
	chunk = Grid::get_chunk(current.chunk_coord);
}
