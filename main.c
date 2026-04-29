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

void	make_linedef(map_t *map, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	linedef_t *l = calloc(1, sizeof(linedef_t));

	l->start = make_vertex(map, x1, y1);
	l->end = make_vertex(map, x2, y2);
	l->flags = 0b100000001;
	//TODO: rest of linedef

	vec_append(map->linedef, l);
}

uint32_t	dir_offset(map_t *map)
{
	return (
		HEADER_OFF +
		MAP_LUMP_SIZE +
		map->vertex->size * sizeof(vertex_t) +
		map->linedef->size * sizeof(linedef_t) +
		map->sideddefs->size * sizeof(sideddef_t) +
		map->sectors->size * sizeof(sector_t) +
		map->ssectors->size * sizeof(ssector_t) +
		map->segs->size * sizeof(seg_t) +
		map->nodes->size * sizeof(node_t));
}

uint32_t	linedef_offset(map_t *map)
{
	return (
		HEADER_OFF +
		MAP_LUMP_SIZE +
		map->vertex->size * sizeof(vertex_t)
	);
}

void write_dir(map_t *map)
{
	filelump_t tmp = {0};

	fseek(map->file, dir_offset(map), SEEK_SET);

	tmp = (filelump_t){HEADER_OFF, MAP_LUMP_SIZE, {0, 0, 0, 0, 'M', 'A', 'Z', 'E'}};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){HEADER_OFF + MAP_LUMP_SIZE, map->vertex->size * sizeof(vertex_t),"VERTEXES"};
	write_filelump(map->file, &tmp);

	tmp = (filelump_t){linedef_offset(map), map->linedef->size * sizeof(linedef_t),"LINEDEFS"};
	write_filelump(map->file, &tmp);
}

void write_stuff(map_t *map)
{
	uint32_t	offset = HEADER_OFF;

	wad_header_t h = {"PWAD", 3, dir_offset(map)};
	write_header(map->file, &h);

	fseek(map->file, HEADER_OFF, SEEK_SET);
	fwrite("skibidi sigma", sizeof(int8_t), 14, map->file);

	fseek(map->file, HEADER_OFF + MAP_LUMP_SIZE, SEEK_SET);

	for (int i = 0; i < map->vertex->size; i++)
		write_vertex(map->file, vec_get(map->vertex, i));

	for (int i = 0; i < map->linedef->size; i++)
		write_linedef(map->file, vec_get(map->linedef, i));

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
