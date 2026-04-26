#include <ctype.h>
#include "vector.h"

typedef struct
{
	char	name[8];
	t_vec	*vertex; // (vertex_t *)
	t_vec	*linedef; // (linedef_t *)
	t_vec	*sideddefs;
	t_vec	*sectors;
	t_vec	*ssectors;
	t_vec	*segs;
	t_vec	*nodes;
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

void	make_vertex(map_t *map, __uint16_t x, __uint16_t y)
{
	vertex_t	*v;

	if (check_vertex(map, x, y))
		return ;

	v = calloc(1, sizeof(vertex_t));
	v->x = x;
	v->y = y;
	vec_append(map->vertex, v);
}

int main()
{

}
