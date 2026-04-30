#include <ctype.h>
#include <stdio.h>
#include <inttypes.h>

#include "vector.h"
#include "types.h"

#define HEADER_OFF 12
#define CELL_WIDTH 48

char *output_name = "out.wad";

void	map_init(map_t	*map)
{
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
	*si = (sideddef_t){0, 0, {0}, "\0\0BRICK1", {0}, 0};
	vec_append(map->sideddefs, si);

	sector_t *se = calloc(1, sizeof(sector_t));
	*se = (sector_t){0, 0, "\0\0BRICK1", "\0\0BRICK1", 100, 0};
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
	seg2->start = l->start;
	seg2->end = l->end;
	seg2->angle = give_direction(x1, y1, x2, y2);
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
		map->things->size * sizeof(thing_t)
	);
}

uint32_t	sidedefs_offset(map_t *map)
{
	return (
		linedef_offset(map) +
		map->linedef->size * sizeof(linedef_t)
	);
}

uint32_t	vertex_offset(map_t *map)
{
	return (
		sidedefs_offset(map) +
		map->sideddefs->size * sizeof(sideddef_t)
	);
}

uint32_t	segs_offset(map_t *map)
{
	return (
		vertex_offset(map) +
		map->vertex->size * sizeof(vertex_t)
	);
}

uint32_t	ssectors_offset(map_t *map)
{
	return (
		segs_offset(map) +
		map->segs->size * sizeof(seg_t)
	);
}

uint32_t	nodes_offset(map_t *map)
{
	return (
		ssectors_offset(map) +
		map->ssectors->size * sizeof(ssector_t)
	);
}

uint32_t	sectors_offset(map_t *map)
{
	return (
		nodes_offset(map) +
		map->nodes->size * sizeof(node_t)
	);
}

uint32_t	dir_offset(map_t *map)
{
	return (
		sectors_offset(map) +
		map->sectors->size * sizeof(sector_t)
	);
}

void write_dir(map_t *map)
{
	filelump_t tmp = {0};

	fseek(map->file, dir_offset(map), SEEK_SET);

	tmp = (filelump_t){HEADER_OFF, MAP_LUMP_SIZE, {0, 0, 0, 'M', 'A', 'P', '0', '1'}};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){things_offset(map), map->things->size * sizeof(thing_t), "\0\0THINGS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){linedef_offset(map), map->linedef->size * sizeof(linedef_t),"LINEDEFS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){sidedefs_offset(map), map->sideddefs->size * sizeof(sideddef_t),"SIDEDEFS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){vertex_offset(map), map->vertex->size * sizeof(vertex_t),"VERTEXES"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){segs_offset(map), map->segs->size * sizeof(seg_t),"\0\0\0\0SEGS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){ssectors_offset(map), map->ssectors->size * sizeof(ssector_t),"SSECTORS"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){nodes_offset(map), map->nodes->size * sizeof(node_t),"\0\0\0NODES"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){sectors_offset(map), map->sectors->size * sizeof(sector_t),"\0SECTORS"};
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
	map_init(&map);
	map.file = fopen("map.wad", "r+b");

	create_square(&map, 10, 10, 20, 20);
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
