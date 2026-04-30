#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>

#include "vector.h"
#include "types.h"

#define HEADER_OFF 12
#define CELL_WIDTH 48

char *output_name = "out.wad";

void	map_init(map_t	*map, uint32_t width, uint32_t height)
{
	map->height = height;
	map->width = width;

	map->vertex = vec_init();
	map->linedef = vec_init();
	map->sideddefs = vec_init();
	map->sectors = vec_init();
	map->ssectors = vec_init();
	map->segs = vec_init();
	map->nodes = vec_init();
	map->things = vec_init();
	map->reject = vec_init();
	map->blockmap = vec_init();


	sideddef_t *si = calloc(1, sizeof(sideddef_t));
	*si = (sideddef_t){0, 0, {0}, {0}, "BRICK1", 0};
	vec_append(map->sideddefs, si);

	sector_t *se = calloc(1, sizeof(sector_t));
	*se = (sector_t){0, 0, "BRICK1", "BRICK1", 100, 0};
	vec_append(map->sectors, se);
}

//returns tag of a vertex
//given vertex SHOULD exist
uint16_t	find_vertex(map_t *map, int16_t x, int16_t y)
{
	for (size_t	i = 0; i < map->vertex->size; i++)
	{
		vertex_t *v = vec_get(map->vertex, i);
		if ((v->x == x) && (v->y == y))
			return (i);
	}
	return (0);
}

//checks if vertex exist
bool	check_vertex(map_t *map, int16_t x, int16_t y)
{
	for (size_t	i = 0; i < map->vertex->size; i++)
	{
		vertex_t *v = vec_get(map->vertex, i);
		if ((v->x == x) && (v->y == y))
			return (true);
	}
	return (false);
}

uint16_t	make_vertex(map_t *map, int16_t x, int16_t y)
{
	vertex_t	*v;

	if (check_vertex(map, x, y))
		return find_vertex(map, x, y);

	v = calloc(1, sizeof(vertex_t));
	v->x = x;
	v->y = y;
	vec_append(map->vertex, v);
	return (map->vertex->size - 1);
}

__uint16_t	give_direction(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	if (y1 > y2)
		return (SOUTH);
	if (y2 > y1)
		return (NORTH);
	if (x1 > x2)
		return (EAST);
	if (x2 > x1)
		return (WEST);
	return (0);
}

void	make_linedef(map_t *map, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	linedef_t *l = calloc(1, sizeof(linedef_t));

	l->start = make_vertex(map, x1, y1);
	l->end = make_vertex(map, x2, y2);
	l->flags = 0b100000001;
	l->type = 0;
	l->sector_tag = 0;
	l->l_sidedef = 0;
	l->r_sidedef = 0;

	//two segs for both sides
	seg_t	*seg1 = calloc(1, sizeof(seg_t));
	seg1->start = l->start;
	seg1->end = l->end;
	seg1->angle = give_direction(x1, y1, x2, y2);
	seg1->linedef = map->linedef->size;
	seg1->direction = 0;
	seg1->offset = 0;

	seg_t	*seg2 = calloc(1, sizeof(seg_t));
	seg2->start = l->end;
	seg2->end = l->start;
	seg2->angle = give_direction(x2, y2, x1, y1);
	seg2->linedef = map->linedef->size;
	seg2->direction = 1;
	seg2->offset = 0;

	vec_append(map->linedef, l);
	vec_append(map->segs, seg1);
	vec_append(map->segs, seg2);
}

uint32_t	things_offset(map_t *map)
{
	return (
		HEADER_OFF +
		MAP_LUMP_SIZE
	);
}

uint32_t	linedef_offset(map_t *map)
{
	return (
		things_offset(map) +
		map->things->size * SIZE_THING
	);
}

uint32_t	sidedefs_offset(map_t *map)
{
	return (
		linedef_offset(map) +
		map->linedef->size * SIZE_LINEDEF
	);
}

uint32_t	vertex_offset(map_t *map)
{
	return (
		sidedefs_offset(map) +
		map->sideddefs->size * SIZE_SIDEDEF
	);
}

uint32_t	segs_offset(map_t *map)
{
	return (
		vertex_offset(map) +
		map->vertex->size * SIZE_VERTEX
	);
}

uint32_t	ssectors_offset(map_t *map)
{
	return (
		segs_offset(map) +
		map->segs->size * SIZE_SEG
	);
}

uint32_t	nodes_offset(map_t *map)
{
	return (
		ssectors_offset(map) +
		map->ssectors->size * SIZE_SSECTOR
	);
}

uint32_t	sectors_offset(map_t *map)
{
	return (
		nodes_offset(map) +
		map->nodes->size * SIZE_NODE
	);
}

uint32_t	dir_offset(map_t *map)
{
	return (
		sectors_offset(map) +
		map->sectors->size * SIZE_SECTOR
	);
}

void set_spawn(map_t *map, int32_t x, int32_t y)
{
	thing_t	*t = calloc(1, sizeof(thing_t));

	t->x_position = x;
	t->y_position = y;
	t->angle = NORTH;
	t->type = THING_SPAWN;
	t->flags = 0;
	vec_append(map->things, t);
}

void placeholder_node(map_t *map)
{
	node_t *n = calloc(1, sizeof(node_t));

	n->x_partition = 0;
	n->y_partition = 0;
	n->change_x_partition = 256;
	n->change_y_partition = 256;
	n->right_box_top = 0;
	n->right_box_bottom = 256;
	n->right_box_left = 0;
	n->right_box_right = 256;
	n->left_box_top = 0;
	n->left_box_bottom = 256;
	n->left_box_left = 0;
	n->left_box_right = 256;
	n->right_child_id = 0b1000000000000000;
	n->left_child_id = 0b1000000000000000;

	ssector_t *s = calloc(1, sizeof(ssector_t));
	s->seg_count = map->segs->size;
	s->seg_start = 0;
	vec_append(map->nodes, n);
	vec_append(map->ssectors, s);
}

void write_dir(map_t *map)
{
	filelump_t tmp = {0};

	fseek(map->file, dir_offset(map), SEEK_SET);

	tmp = (filelump_t){HEADER_OFF, MAP_LUMP_SIZE, "MAP01"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){things_offset(map), map->things->size * SIZE_THING, "THINGS\0\0"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){linedef_offset(map), map->linedef->size * SIZE_LINEDEF,"LINEDEFS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){sidedefs_offset(map), map->sideddefs->size * SIZE_SIDEDEF,"SIDEDEFS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){vertex_offset(map), map->vertex->size * SIZE_VERTEX,"VERTEXES"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){segs_offset(map), map->segs->size * SIZE_SEG,"SEGS\0\0\0\0"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){ssectors_offset(map), map->ssectors->size * SIZE_SSECTOR,"SSECTORS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){nodes_offset(map), map->nodes->size * SIZE_NODE,"NODES\0\0\0"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){sectors_offset(map), map->sectors->size * SIZE_SECTOR,"SECTORS\0"};
	write_filelump(map->file, &tmp);
}

void write_stuff(map_t *map)
{
	uint32_t	offset = HEADER_OFF;

	wad_header_t h = {"PWAD", 9, dir_offset(map)};
	write_header(map->file, &h);

	fseek(map->file, HEADER_OFF, SEEK_SET);
	fwrite("version 2.3\0BM", sizeof(int8_t), 14, map->file);

	fseek(map->file, things_offset(map), SEEK_SET);
	for (int i = 0; i < map->things->size; i++)
		write_thing(map->file, vec_get(map->things, i));

	fseek(map->file, linedef_offset(map), SEEK_SET);
	for (int i = 0; i < map->linedef->size; i++)
		write_linedef(map->file, vec_get(map->linedef, i));

	fseek(map->file, sidedefs_offset(map), SEEK_SET);
	for (int i = 0; i < map->sideddefs->size; i++)
		write_sideddef(map->file, vec_get(map->sideddefs, i));

	fseek(map->file, vertex_offset(map), SEEK_SET);
	for (int i = 0; i < map->vertex->size; i++)
		write_vertex(map->file, vec_get(map->vertex, i));

	fseek(map->file, segs_offset(map), SEEK_SET);
	for (int i = 0; i < map->segs->size; i++)
		write_seg(map->file, vec_get(map->segs, i));

	fseek(map->file, ssectors_offset(map), SEEK_SET);
	for (int i = 0; i < map->ssectors->size; i++)
		write_ssector(map->file, vec_get(map->ssectors, i));

	fseek(map->file, nodes_offset(map), SEEK_SET);
	for (int i = 0; i < map->nodes->size; i++)
		write_node(map->file, vec_get(map->nodes, i));

	fseek(map->file, sectors_offset(map), SEEK_SET);
	for (int i = 0; i < map->sectors->size; i++)
		write_sector(map->file, vec_get(map->sectors, i));

	write_dir(map);
}

void create_square(map_t *map, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	make_linedef(map, x1, y1, x1, y2);
	make_linedef(map, x1, y1, x2, y1);
	make_linedef(map, x2, y1, x2, y2);
	make_linedef(map, x1, y2, x2, y2);
}

int main()
{
	map_t map = {0};
	map_init(&map, 256, 256);
	set_spawn(&map, 128, 128);
	create_square(&map, 32, 32, 64, 64);
	placeholder_node(&map);
	map.file = fopen("map.wad", "w+b");
	write_stuff(&map);
	return (0);
}


// 1111101111
// 1000000001
// 1101011011
// 1000011111
// 1111001001
// 1000011011
// 1011000001
// 1010011011
// 1011111111

//starting coord (1, 1)
