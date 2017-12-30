#include "building_tiles.h"

#include "building/building.h"
#include "building/industry.h"
#include "core/direction.h"
#include "graphics/image.h"
#include "map/aqueduct.h"
#include "map/building.h"
#include "map/grid.h"
#include "map/image.h"
#include "map/property.h"
#include "map/random.h"
#include "map/sprite.h"
#include "map/terrain.h"

#include "Data/State.h"
#include "../TerrainGraphics.h"

void map_building_tiles_add(int building_id, int x, int y, int size, int image_id, int terrain)
{
    if (!map_grid_is_inside(x, y, size)) {
        return;
    }
    int x_leftmost, y_leftmost;
    switch (Data_State.map.orientation) {
        case DIR_0_TOP:
            x_leftmost = 0;
            y_leftmost = size - 1;
            break;
        case DIR_2_RIGHT:
            x_leftmost = y_leftmost = 0;
            break;
        case DIR_4_BOTTOM:
            x_leftmost = size - 1;
            y_leftmost = 0;
            break;
        case DIR_6_LEFT:
            x_leftmost = y_leftmost = size - 1;
            break;
        default:
            return;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
            map_terrain_add(grid_offset, terrain);
            map_building_set(grid_offset, building_id);
            map_property_clear_constructing(grid_offset);
            map_property_set_multi_tile_size(grid_offset, size);
            map_image_set(grid_offset, image_id);
            map_property_set_multi_tile_xy(grid_offset, dx, dy,
                dx == x_leftmost && dy == y_leftmost);
        }
    }
}

static void set_crop_tile(int building_id, int x, int y, int dx, int dy, int crop_image_id, int growth)
{
    int grid_offset = map_grid_offset(x + dx, y + dy);
    map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
    map_terrain_add(grid_offset, TERRAIN_BUILDING);
    map_building_set(grid_offset, building_id);
    map_property_clear_constructing(grid_offset);
    map_property_set_multi_tile_xy(grid_offset, dx, dy, 1);
    map_image_set(grid_offset, crop_image_id + (growth < 4 ? growth : 4));
}

void map_building_tiles_add_farm(int building_id, int x, int y, int crop_image_id, int progress)
{
    if (!map_grid_is_inside(x, y, 3)) {
        return;
    }
    // farmhouse
    int x_leftmost, y_leftmost;
    switch (Data_State.map.orientation) {
        case DIR_0_TOP:
            x_leftmost = 0;
            y_leftmost = 1;
            break;
        case DIR_2_RIGHT:
            x_leftmost = 0;
            y_leftmost = 0;
            break;
        case DIR_4_BOTTOM:
            x_leftmost = 1;
            y_leftmost = 0;
            break;
        case DIR_6_LEFT:
            x_leftmost = 1;
            y_leftmost = 1;
            break;
        default:
            return;
    }
    for (int dy = 0; dy < 2; dy++) {
        for (int dx = 0; dx < 2; dx++) {
            int gridOffset = map_grid_offset(x + dx, y + dy);
            map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
            map_terrain_add(gridOffset, TERRAIN_BUILDING);
            map_building_set(gridOffset, building_id);
            map_property_clear_constructing(gridOffset);
            map_property_set_multi_tile_size(gridOffset, 2);
            map_image_set(gridOffset, image_group(GROUP_BUILDING_FARM_HOUSE));
            map_property_set_multi_tile_xy(gridOffset, dx, dy,
                dx == x_leftmost && dy == y_leftmost);
        }
    }
    // crop tile 1
    int growth = progress / 10;
    set_crop_tile(building_id, x, y, 0, 2, crop_image_id, growth);
    
    // crop tile 2
    growth -= 4;
    if (growth < 0) {
        growth = 0;
    }
    set_crop_tile(building_id, x, y, 1, 2, crop_image_id, growth);
    
    // crop tile 3
    growth -= 4;
    if (growth < 0) {
        growth = 0;
    }
    set_crop_tile(building_id, x, y, 2, 2, crop_image_id, growth);
    
    // crop tile 4
    growth -= 4;
    if (growth < 0) {
        growth = 0;
    }
    set_crop_tile(building_id, x, y, 2, 1, crop_image_id, growth);
    
    // crop tile 5
    growth -= 4;
    if (growth < 0) {
        growth = 0;
    }
    set_crop_tile(building_id, x, y, 2, 0, crop_image_id, growth);
}

static int north_tile_grid_offset(int x, int y, int *size)
{
    int grid_offset = map_grid_offset(x, y);
    *size = map_property_multi_tile_size(grid_offset);
    for (int i = 0; i < *size && map_property_multi_tile_x(grid_offset); i++) {
        grid_offset += map_grid_delta(-1, 0);
    }
    for (int i = 0; i < *size && map_property_multi_tile_y(grid_offset); i++) {
        grid_offset += map_grid_delta(0, -1);
    }
    return grid_offset;
}

void map_building_tiles_remove(int building_id, int x, int y)
{
    if (!map_grid_is_inside(x, y, 1)) {
        return;
    }
    int size;
    int baseGridOffset = north_tile_grid_offset(x, y, &size);
    x = map_grid_offset_to_x(baseGridOffset);
    y = map_grid_offset_to_y(baseGridOffset);
    if (map_terrain_get(baseGridOffset) == TERRAIN_ROCK) {
        return;
    }
    building *b = building_get(building_id);
    if (building_id && building_is_farm(b->type)) {
        size = 3;
    }
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int gridOffset = map_grid_offset(x + dx, y + dy);
            if (building_id && map_building_at(gridOffset) != building_id) {
                continue;
            }
            if (building_id && b->type != BUILDING_BURNING_RUIN) {
                map_set_rubble_building_type(gridOffset, b->type);
            }
            map_property_clear_constructing(gridOffset);
            map_property_set_multi_tile_size(gridOffset, 1);
            map_property_clear_multi_tile_xy(gridOffset);
            map_property_mark_draw_tile(gridOffset);
            map_aqueduct_set(gridOffset, 0);
            map_building_set(gridOffset, 0);
            map_building_damage_clear(gridOffset);
            map_sprite_clear_tile(gridOffset);
            if (map_terrain_is(gridOffset, TERRAIN_WATER)) {
                map_terrain_set(gridOffset, TERRAIN_WATER); // clear other flags
                TerrainGraphics_setTileWater(x + dx, y + dy);
            } else {
                map_image_set(gridOffset,
                    image_group(GROUP_TERRAIN_UGLY_GRASS) +
                    (map_random_get(gridOffset) & 7));
                map_terrain_remove(gridOffset, TERRAIN_CLEARABLE);
            }
        }
    }
    TerrainGraphics_updateRegionEmptyLand(x, y, x + size, y + size);
    TerrainGraphics_updateRegionMeadow(x, y, x + size, y + size);
    TerrainGraphics_updateRegionRubble(x, y, x + size, y + size);
}

void map_building_tiles_set_rubble(int building_id, int x, int y, int size)
{
    if (!map_grid_is_inside(x, y, size)) {
        return;
    }
    building *b = building_get(building_id);
    for (int dy = 0; dy < size; dy++) {
        for (int dx = 0; dx < size; dx++) {
            int grid_offset = map_grid_offset(x + dx, y + dy);
            if (map_building_at(grid_offset) != building_id) {
                continue;
            }
            if (building_id && building_get(map_building_at(grid_offset))->type != BUILDING_BURNING_RUIN) {
                map_set_rubble_building_type(grid_offset, b->type);
            }
            map_property_clear_constructing(grid_offset);
            map_property_set_multi_tile_size(grid_offset, 1);
            map_aqueduct_set(grid_offset, 0);
            map_building_set(grid_offset, 0);
            map_building_damage_clear(grid_offset);
            map_sprite_clear_tile(grid_offset);
            map_property_set_multi_tile_xy(grid_offset, 0, 0, 1);
            if (map_terrain_is(grid_offset, TERRAIN_WATER)) {
                map_terrain_set(grid_offset, TERRAIN_WATER); // clear other flags
                TerrainGraphics_setTileWater(x + dx, y + dy);
            } else {
                map_terrain_remove(grid_offset, TERRAIN_CLEARABLE);
                map_terrain_add(grid_offset, TERRAIN_RUBBLE);
                map_image_set(grid_offset, image_group(GROUP_TERRAIN_RUBBLE) + (map_random_get(grid_offset) & 7));
            }
        }
    }
}