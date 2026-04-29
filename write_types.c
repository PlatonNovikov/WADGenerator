#include <inttypes.h>
#include <stdio.h>

#include "vector.h"
#include "types.h"

static void	write16rev(FILE *file, uint16_t v)
{
	uint8_t	tmp;

	tmp = v & 255;
	fwrite(&tmp, sizeof(int8_t), 1, file);
	tmp = (v >> 8) & 255;
	fwrite(&tmp, sizeof(int8_t), 1, file);
}

static void	write32rev(FILE *file, uint32_t v)
{
	uint8_t	tmp;

	tmp = v & 255;
	fwrite(&tmp, sizeof(int8_t), 1, file);
	tmp = (v >> 8) & 255;
	fwrite(&tmp, sizeof(int8_t), 1, file);
	tmp = (v >> 16) & 255;
	fwrite(&tmp, sizeof(int8_t), 1, file);
	tmp = (v >>24) & 255;
	fwrite(&tmp, sizeof(int8_t), 1, file);
}

void	write_header(FILE *file, wad_header_t *h)
{
	fwrite(&h->name, sizeof(uint8_t), 4, file);
	write32rev(file, h->entries);
	write32rev(file, h->offset);
}

void	write_vertex(FILE *file, vertex_t *v)
{
	write16rev(file, v->x);
	write16rev(file, v->y);
}

void	write_linedef(FILE *file, linedef_t *l)
{
	write16rev(file, l->start);
	write16rev(file, l->end);
	write16rev(file, l->flags);
	write16rev(file, l->type);
	write16rev(file, l->sector_tag);
	write16rev(file, l->r_sidedef);
	write16rev(file, l->l_sidedef);
}

void	write_filelump(FILE *file, filelump_t *f)
{
	write32rev(file, f->filepos);
	write32rev(file, f->size);
	fwrite(&f->name, sizeof(uint8_t), 8, file);
}

void	write_sideddef(FILE *file, sideddef_t *s)
{
	write16rev(file, s->x_offset);
	write16rev(file, s->y_offset);
	fwrite(&s->upper_texture, sizeof(uint8_t), 8, file);
	fwrite(&s->lower_texture, sizeof(uint8_t), 8, file);
	fwrite(&s->middle_texture, sizeof(uint8_t), 8, file);
	write16rev(file, s->sector_id);
}

void	write_sector(FILE *file, sector_t *s)
{
	write16rev(file, s->floor_height);
	write16rev(file, s->ceiling_height);
	fwrite(&s->floor_texture, sizeof(uint8_t), 8, file);
	fwrite(&s->ceiling_texture, sizeof(uint8_t), 8, file);
	write16rev(file, s->light_level);
	write16rev(file, s->type);
	write16rev(file, s->tag);
}

void	write_ssector(FILE *file, ssector_t *s)
{
	write16rev(file, s->seg_count);
	write16rev(file, s->seg_start);
}

void	write_seg(FILE *file, seg_t *s)
{
	write16rev(file, s->start);
	write16rev(file, s->end);
	write16rev(file, s->angle);
	write16rev(file, s->linedef);
	write16rev(file, s->direction);
	write16rev(file, s->offset);
}

void	write_node(FILE *file, node_t *n)
{
	write16rev(file, n->x_partition);
	write16rev(file, n->y_partition);
	write16rev(file, n->change_x_partition);
	write16rev(file, n->change_y_partition);
	write16rev(file, n->right_box_top);
	write16rev(file, n->right_box_bottom);
	write16rev(file, n->right_box_left);
	write16rev(file, n->right_box_right);
	write16rev(file, n->left_box_top);
	write16rev(file, n->left_box_bottom);
	write16rev(file, n->left_box_left);
	write16rev(file, n->right_box_right);
	write16rev(file, n->right_child_id);
	write16rev(file, n->left_child_id);
}
