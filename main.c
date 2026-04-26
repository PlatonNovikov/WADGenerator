#include <ctype.h>
#include "vector.h"

#define CELL_WIDTH 48

typedef struct
{
	__uint32_t	width;
	__uint32_t	height;
	
	// considering that the player spawns at (0, 0), this is needed to normalise coordinates
	__uint32_t player_offset_x;
	__uint32_t player_offset_y;

	char	name[8];
	t_vec	*vertex; // (vertex_t *)
	t_vec	*linedef; // (linedef_t *)
	t_vec	*sideddefs;
	t_vec	*sectors;
	t_vec	*ssectors;
	t_vec	*segs;
	t_vec	*nodes;

	// ill ignore those for now
	t_vec	*things;
	t_vec	*reject;
	t_vec	*blockmap;
} map_t;

typedef struct
{
	__uint8_t	name[4]; //IWAD or PWAD
	__uint32_t	entries; //The number entries in the directory
	__uint32_t	offset; //Offset in bytes to the directory in the WAD file.
} wad_header_t;

typedef struct
{
	__int16_t	x; //X Position
	__int16_t	y; //Y Position
} vertex_t;

typedef struct
{
	__uint16_t	start; //Start vertex
	__uint16_t	end; //End vertex
	__uint16_t	flags;
	__uint16_t	type; //Line type / Action
	__uint16_t	sector_tag; //Sector tag
	__uint16_t	r_sidedef; //Right sidedef ( 0xFFFF side not present )
	__uint16_t	l_sidedef; //Left sidedef ( 0xFFFF side not present )
} linedef_t;

typedef struct
{
	__uint32_t	filepos; // An integer holding a pointer to the start of the lump's data in the file.
	__uint32_t	size; // An integer representing the size of the lump in bytes.
	__uint8_t	name[8];
} filelump_t;

// Linedef Flags Values

// Note all lines (walls) are to be drawn. Some have special behaviors.
// Bit 	Description
// 0 	Blocks players and monsters
// 1 	Blocks monsters
// 2 	Two sided
// 3 	Upper texture is unpegged (will research this later)
// 4 	Lower texture is unpegged (will research this later)
// 5 	Secret (shows as one-sided on automap)
// 6 	Blocks sound
// 7 	Never shows on automap
// 8 	Always shows on automap

struct WADSidedef
{
    __int16_t x_offset;
    __int16_t y_offset;
    __int8_t upper_texture[8];
    __int8_t lower_texture[8];
    __int8_t middle_texture[8];
    __uint16_t sector_id;
};

typedef struct
{
	__uint16_t	seg_count; //The number of segs that compose this sub-segment
	__uint16_t	seg_start; //The index of the first seg in the seg list
} ssector_t;

typedef struct
{
	__uint16_t	start; //Start vertex
	__uint16_t	end; //End vertex
	__uint16_t	angle;
	__uint16_t	linedef; //Linedef index
	__uint16_t	direction; //0 (same as linedef) or 1 (opposite of linedef)
	__uint16_t	offset; //distance along linedef to start of seg
} seg_t;

typedef struct
{
	__int16_t x_partition; //X coordinate of the splitter
	__int16_t y_partition; //Y coordinate of the splitter
	__int16_t change_x_partition; //The amount to move in X to reach end of splitter
	__int16_t change_y_partition; //The amount to move in y to reach end of splitter

	__int16_t right_box_top; //First corner of right box (Y coordinate)
	__int16_t right_box_bottom; //Second corner of right box (Y coordinate)
	__int16_t right_box_left; //First corner of right box (X coordinate)
	__int16_t right_box_right; //Second corner of right box (X coordinate)

	__int16_t left_box_top; //First corner of left box (Y coordinate)
	__int16_t left_box_bottom; //Second corner of left box (Y coordinate)
	__int16_t left_box_left; //First corner of left box (X coordinate)
	__int16_t left_box_right; //Second corner of left box (X coordinate)

	__uint16_t right_child_id; //Index of the right child + sub-sector indicator
	__uint16_t left_child_id; //Index of the left child + sub-sector indicator
} node_t;

enum EMAPLUMPSINDEX
{
    eTHINGS = 1,
    eLINEDEFS,
    eSIDEDDEFS,
    eVERTEXES,
    eSEGS,
    eSSECTORS,
    eNODES,
    eSECTORS,
    eREJECT,
    eBLOCKMAP,
    eCOUNT
};

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
__uint16_t	*find_vertex(map_t *map, __uint16_t x, __uint16_t y)
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
bool	check_vertex(map_t *map, __uint16_t x, __uint16_t y)
{
	for (size_t	i = 0; i < map->vertex->size; i++)
	{
		vertex_t *v = vec_get(map->vertex, i);
		if ((v->x == x) && (v->y == y))
			return (true);
	}
	return (false);
}

__uint16_t	make_vertex(map_t *map, __uint16_t x, __uint16_t y)
{
	vertex_t	*v;

	if (check_vertex(map, x, y))
		return find_vertex(map, x, y);

	v = calloc(1, sizeof(vertex_t));
	v->x = x;
	v->y = y;
	vec_append(map->vertex, v);
	return (map->vertex.size - 1);
}

void	make_linedef(map_t *map, __uint16_t x1, __uint16_t y1, __int16_t x2, __uint16_t y2)
{
	linedef_t *l = calloc(1, sizeof(linedef_t));

	l->start = make_vertex(map, x1, y1);
	l->end = make_vertex(map, x2, y2);
	l->flags = 0b100000001;
	//TODO: rest of linedef

	vec_append(map->linedef, l);
}

__uint32_t	calc_dir_offset(map_t *map)
{
	return (
		map->vertex->size * sizeof(vertex_t) +
		map->linedef->size * sizeof(linedef_t) +
		map->sideddefs->size * sizeof(sideddef_t) +
		map->sectors->size * sizeof(sector_t) +
		map->ssectors->size * sizeof(ssector_t) +
		map->segs->size * sizeof(seg_t) +
		map->nodes->size * sizeof(node_t));
}

int main()
{

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