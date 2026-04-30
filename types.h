#include <inttypes.h>
#include <stdio.h>

#include "vector.h"

#define MAP_LUMP_SIZE 0
#define EAST	0
#define NORTH	0x4000
#define	WEST	0x8000
#define SOUTH	0xC000
#define	THING_SPAWN 1

#define SIZE_THING 10
#define SIZE_LINEDEF 14
#define SIZE_SIDEDEF 30
#define SIZE_VERTEX 4
#define SIZE_SEG 12
#define SIZE_SSECTOR 4
#define SIZE_NODE 28
#define SIZE_SECTOR 26

typedef struct
{
	uint32_t	width;
	uint32_t	height;

	// considering that the player spawns at (0, 0), this is needed to normalise coordinates
	int16_t	player_offset_x;
	int16_t player_offset_y;

	char	name[8];


	// original order of stuff
	t_vec	*things;
	t_vec	*linedef; // (linedef_t *)
	t_vec	*sideddefs;
	t_vec	*vertex; // (vertex_t *)
	t_vec	*segs;
	t_vec	*ssectors;
	t_vec	*nodes;
	t_vec	*sectors;

	// ill ignore those for now
	t_vec	*reject;
	t_vec	*blockmap;

	FILE	*file;
} map_t;

typedef struct
{
	uint8_t		name[4]; //IWAD or PWAD
	uint32_t	entries; //The number entries in the directory
	uint32_t	offset; //Offset in bytes to the directory in the WAD file.
} wad_header_t;

typedef struct
{
    int16_t x_position;
    int16_t y_position;
    uint16_t angle;
    uint16_t type;
    uint16_t flags;
} thing_t;

typedef struct
{
	int16_t	x; //X Position
	int16_t	y; //Y Position
} vertex_t;

typedef struct
{
	uint16_t	start; //Start vertex
	uint16_t	end; //End vertex
	uint16_t	flags;
	uint16_t	type; //Line type / Action
	uint16_t	sector_tag; //Sector tag
	uint16_t	r_sidedef; //Right sidedef ( 0xFFFF side not present )
	uint16_t	l_sidedef; //Left sidedef ( 0xFFFF side not present )
} linedef_t;

typedef struct
{
	uint32_t	filepos; // An integer holding a pointer to the start of the lump's data in the file.
	uint32_t	size; // An integer representing the size of the lump in bytes.
	uint8_t		name[8];
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

typedef struct
{
    int16_t		x_offset;
    int16_t		y_offset;
    int8_t		upper_texture[8];
    int8_t		lower_texture[8];
    int8_t		middle_texture[8];
    uint16_t	sector_id;
} sideddef_t;

typedef struct
{
    int16_t		floor_height;
    int16_t		ceiling_height;
    int8_t		floor_texture[8];
    int8_t		ceiling_texture[8];
    uint16_t	light_level;
    uint16_t	type;
    uint16_t	tag;
} sector_t;

typedef struct
{
	uint16_t	seg_count; //The number of segs that compose this sub-segment
	uint16_t	seg_start; //The index of the first seg in the seg list
} ssector_t;

typedef struct
{
	uint16_t	start; //Start vertex
	uint16_t	end; //End vertex
	uint16_t	angle;
	uint16_t	linedef; //Linedef index
	uint16_t	direction; //0 (same as linedef) or 1 (opposite of linedef)
	uint16_t	offset; //distance along linedef to start of seg
} seg_t;

typedef struct
{
	int16_t x_partition; //X coordinate of the splitter
	int16_t y_partition; //Y coordinate of the splitter
	int16_t change_x_partition; //The amount to move in X to reach end of splitter
	int16_t change_y_partition; //The amount to move in y to reach end of splitter

	int16_t right_box_top; //First corner of right box (Y coordinate)
	int16_t right_box_bottom; //Second corner of right box (Y coordinate)
	int16_t right_box_left; //First corner of right box (X coordinate)
	int16_t right_box_right; //Second corner of right box (X coordinate)

	int16_t left_box_top; //First corner of left box (Y coordinate)
	int16_t left_box_bottom; //Second corner of left box (Y coordinate)
	int16_t left_box_left; //First corner of left box (X coordinate)
	int16_t left_box_right; //Second corner of left box (X coordinate)

	uint16_t right_child_id; //Index of the right child + sub-sector indicator
	uint16_t left_child_id; //Index of the left child + sub-sector indicator
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

void	write_header(FILE *file, wad_header_t *h);
void	write_vertex(FILE *file, vertex_t *v);
void	write_linedef(FILE *file, linedef_t *l);
void	write_filelump(FILE *file, filelump_t *f);
void	write_sideddef(FILE *file, sideddef_t *s);
void	write_sector(FILE *file, sector_t *s);
void	write_ssector(FILE *file, ssector_t *s);
void	write_seg(FILE *file, seg_t *s);
void	write_node(FILE *file, node_t *n);
void	write_thing(FILE *file, thing_t *t);
