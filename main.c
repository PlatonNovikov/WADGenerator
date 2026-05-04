#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "vector.h"
#include "types.h"

#define HEADER_OFF 12
#define CELL_SIZE 128

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


	sideddef_t *si = calloc(1, sizeof(sideddef_t));
	*si = (sideddef_t){0, 0, "BRICK1", "BRICK1", "BRICK1", 0};
	vec_append(map->sideddefs, si);

	sector_t *se = calloc(1, sizeof(sector_t));
	*se = (sector_t){0, 160, "BRICK1", "BRICK1", 150, 0};
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
	return (0); //should not happen but whatever
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

	seg_t	*seg = calloc(1, sizeof(seg_t));
	seg->start = l->start;
	seg->end = l->end;
	seg->angle = give_direction(x1, y1, x2, y2);
	seg->linedef = map->linedef->size;
	seg->direction = 0;
	seg->offset = 0;

	vec_append(map->linedef, l);
	vec_append(map->segs, seg);
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

void make_thing(map_t *map, int32_t x, int32_t y, uint16_t type, uint16_t flags)
{
	thing_t	*t = calloc(1, sizeof(thing_t));

	t->x_position = x;
	t->y_position = y;
	t->angle = NORTH;
	t->type = type;
	t->flags = flags;
	vec_append(map->things, t);
}

uint16_t rand_decoration(void)
{
	const uint16_t	decorations[] = {10, 12, 15, 18, 19, 20, 21, 22, 23, 24, 34, 59, 60, 61, 62, 63, 79, 80, 81};
	const size_t	len = 19;

	return (decorations[rand() % len]);
}

uint16_t rand_weapon(void)
{
	const uint16_t	weapons[] = {82, 2001, 2002, 2003, 2004, 2005, 2006, 2011, 2012, 2019, 2018, 2015, 2023, 2008, 2048, 2046, 2049, 2007, 2047, 17, 2010};
	const size_t	len = 21;

	return (weapons[rand() % len]);
}

uint16_t rand_enemy(void)
{
	const uint16_t	enemies[] = {3004, 9, 3001, 3002, 3005};
	const size_t	len = 5;

	return (enemies[rand() % len]);
}

void make_rand_thing(map_t *map, int32_t x, int32_t y)
{
	uint16_t thing_tag = 0;

	switch (rand() % 3)
	{
	case 0:
		thing_tag = rand_decoration();
		break;

	case 1:
		thing_tag = rand_weapon();
		break;

	case 2:
		thing_tag = rand_enemy();
		break;
	}
	make_thing(map, x, y, thing_tag, 7);
}

void placeholder_node(map_t *map)
{
	node_t *n = calloc(1, sizeof(node_t));

	n->x_partition = 0;
	n->y_partition = 0;
	n->change_x_partition = 1;
	n->change_y_partition = 1;
	n->right_box_top = 0;
	n->right_box_bottom = 25600;
	n->right_box_left = 0;
	n->right_box_right = 25600;
	n->left_box_top = 0;
	n->left_box_bottom = 25600;
	n->left_box_left = 0;
	n->left_box_right = 25600;
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

void create_rect(map_t *map, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	make_linedef(map, x1, y2, x1, y1);
	make_linedef(map, x1, y1, x2, y1);
	make_linedef(map, x2, y1, x2, y2);
	make_linedef(map, x2, y2, x1, y2);
}

void convert_input(map_t *map, input_t *input)
{
	size_t i = 0;

	map_init(map, input->width * CELL_SIZE, input->height * CELL_SIZE);
	map->height = input->height;
	map->width = input->width;
	while (input->mapstr[i])
	{
		switch (input->mapstr[i])
		{
		case eEMPTY:
			if (!(rand() % 4))
				make_rand_thing(
					map,
					(i % map->height) * CELL_SIZE + CELL_SIZE / 2,
					(i / map->height) * CELL_SIZE + CELL_SIZE / 2
				);
			break;

		case eWALL:
			create_rect(
				map,
				(i % map->height) * CELL_SIZE,
				(i / map->height) * CELL_SIZE,
				(i % map->height) * CELL_SIZE + CELL_SIZE,
				(i / map->height) * CELL_SIZE + CELL_SIZE
			);
			break;

		case eSPAWN:
				make_thing(
					map,
					(i % map->height) * CELL_SIZE + CELL_SIZE / 2,
					(i / map->height) * CELL_SIZE + CELL_SIZE / 2,
					THING_SPAWN,
					0
				);
				break;
		default:
			break;
		}
		i++;
	}
}

void free_everything(map_t *map)
{
	for (int i = 0; i < map->things->size; i++)
		free(vec_get(map->things, i));
	vec_free(map->things);

	for (int i = 0; i < map->linedef->size; i++)
		free(vec_get(map->linedef, i));
	vec_free(map->linedef);

	for (int i = 0; i < map->sideddefs->size; i++)
		free(vec_get(map->sideddefs, i));
	vec_free(map->sideddefs);

	for (int i = 0; i < map->vertex->size; i++)
		free(vec_get(map->vertex, i));
	vec_free(map->vertex);

	for (int i = 0; i < map->segs->size; i++)
		free(vec_get(map->segs, i));
	vec_free(map->segs);

	for (int i = 0; i < map->ssectors->size; i++)
		free(vec_get(map->ssectors, i));
	vec_free(map->ssectors);

	for (int i = 0; i < map->nodes->size; i++)
		free(vec_get(map->nodes, i));
	vec_free(map->nodes);

	for (int i = 0; i < map->sectors->size; i++)
		free(vec_get(map->sectors, i));
	vec_free(map->sectors);
}

void converter()
{

}

int main(int argc, char **argv)
{
	map_t map = {0};
	char *mapstr = "111111111111111111111100000100000100010000111110111010101011101100010000010001000101101111111111111110101100010000010000010101101010101110101110101101000100000100010001101111111111111011101100010000010000010001111011101110111010111100010001000100010001101110101011101111101101000101000101000101101110101110101011101100010100010100010001111010111010111110111101000101010000010001101111101011111011101120000000000001000001111111111111111111111";
	input_t placeholder = {mapstr, 21, 21};
	convert_input(&map, &placeholder);
	placeholder_node(&map);
	map.file = fopen("map.wad", "w+b");
	write_stuff(&map);
	fclose(map.file);
	free_everything(&map);
	return (0);
}
