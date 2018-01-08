/*------------------------------------------------------------------------
 *  Copyright 2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <stdio.h>
#include <jpeglib.h>
#include <jerror.h>
#include <setjmp.h>
#include <assert.h> /* FIXME tmp debug */

#undef HAVE_STDLIB_H
#include <zbar.h>
#include "image.h"
#include "video.h"

#define HAVE_LONGJMP
#ifdef HAVE_LONGJMP

typedef struct errenv_s {
    struct jpeg_error_mgr err;
    int valid;
    jmp_buf env;
} errenv_t;

void zbar_jpeg_error (j_common_ptr cinfo)
{
    errenv_t *jerr = (errenv_t*)cinfo->err;
    assert(jerr->valid);
    jerr->valid = 0;
    longjmp(jerr->env, 1);
    assert(0);
}

#endif

typedef struct zbar_src_mgr_s {
    struct jpeg_source_mgr src;
    const zbar_image_t *img;
} zbar_src_mgr_t;

static const JOCTET fake_eoi[2] = { 0xff, JPEG_EOI };

void init_source (j_decompress_ptr cinfo)
{
    /* buffer/length refer to compressed data */
    /* FIXME find img */
    const zbar_image_t *img = ((zbar_src_mgr_t*)cinfo->src)->img;
    cinfo->src->next_input_byte = img->data;
    cinfo->src->bytes_in_buffer = img->datalen;
}

int fill_input_buffer (j_decompress_ptr cinfo)
{
    /* buffer underrun error case */
    cinfo->src->next_input_byte = fake_eoi;
    cinfo->src->bytes_in_buffer = 2;
    return(1);
}

void skip_input_data (j_decompress_ptr cinfo,
                      long num_bytes)
{
    if(num_bytes > 0) {
        if (num_bytes < cinfo->src->bytes_in_buffer) {
            cinfo->src->next_input_byte += num_bytes;
            cinfo->src->bytes_in_buffer -= num_bytes;
        }
        else {
            fill_input_buffer(cinfo);
        }
    }
}

void term_source (j_decompress_ptr cinfo)
{
    /* nothing todo */
}

struct jpeg_decompress_struct * _zbar_jpeg_decomp_create (void)
{
    j_decompress_ptr cinfo = calloc(1, sizeof(struct jpeg_decompress_struct));
    if(!cinfo)
        return(NULL);

    errenv_t *jerr = calloc(1, sizeof(errenv_t));
    if(!jerr) {
        free(cinfo);
        return(NULL);
    }

    cinfo->err = jpeg_std_error(&jerr->err);
    jerr->err.error_exit = zbar_jpeg_error;

    jerr->valid = 1;
    if(setjmp(jerr->env)) {
        jpeg_destroy_decompress(cinfo);

        /* FIXME TBD save error to errinfo_t */
        (*cinfo->err->output_message)((j_common_ptr)cinfo);

        free(jerr);
        free(cinfo);
        return(NULL);
    }

    jpeg_create_decompress(cinfo);

    jerr->valid = 0;
    return(cinfo);
}

void _zbar_jpeg_decomp_destroy (struct jpeg_decompress_struct *cinfo)
{
    if(cinfo->err) {
        free(cinfo->err);
        cinfo->err = NULL;
    }
    if(cinfo->src) {
        free(cinfo->src);
        cinfo->src = NULL;
    }
    /* FIXME can this error? */
    jpeg_destroy_decompress(cinfo);
    free(cinfo);
}

/* invoke libjpeg to decompress JPEG format to luminance plane */
void _zbar_convert_jpeg_to_y (zbar_image_t *dst,
                              const zbar_format_def_t *dstfmt,
                              const zbar_image_t *src,
                              const zbar_format_def_t *srcfmt)
{
    /* create decompressor, or use cached video stream decompressor */
    errenv_t *jerr = NULL;
    j_decompress_ptr cinfo;
    if(!src->src)
        cinfo = _zbar_jpeg_decomp_create();
    else {
        cinfo = src->src->jpeg;
        assert(cinfo);
    }
    if(!cinfo)
        goto error;

    jerr = (errenv_t*)cinfo->err;
    jerr->valid = 1;
    if(setjmp(jerr->env)) {
        /* FIXME TBD save error to src->src->err */
        (*cinfo->err->output_message)((j_common_ptr)cinfo);
        if(dst->data) {
            free((void*)dst->data);
            dst->data = NULL;
        }
        dst->datalen = 0;
        goto error;
    }

    /* setup input image */
    if(!cinfo->src) {
        cinfo->src = calloc(1, sizeof(zbar_src_mgr_t));
        cinfo->src->init_source = init_source;
        cinfo->src->fill_input_buffer = fill_input_buffer;
        cinfo->src->skip_input_data = skip_input_data;
        cinfo->src->resync_to_restart = jpeg_resync_to_restart;
        cinfo->src->term_source = term_source;
    }
    cinfo->src->next_input_byte = NULL;
    cinfo->src->bytes_in_buffer = 0;
    ((zbar_src_mgr_t*)cinfo->src)->img = src;

    int rc = jpeg_read_header(cinfo, TRUE);
    zprintf(30, "header: %s\n", (rc == 2) ? "tables-only" : "normal");

    /* supporting color with jpeg became...complicated,
     * so we skip that for now
     */
    cinfo->out_color_space = JCS_GRAYSCALE;

    /* FIXME set scaling based on dst->{width,height}
     * then pass bigger buffer...
     */

    jpeg_start_decompress(cinfo);

    /* adjust dst image parameters to match(?) decompressor */
    if(dst->width < cinfo->output_width) {
        dst->width = cinfo->output_width;
        if(dst->crop_x + dst->crop_w > dst->width)
            dst->crop_w = dst->width - dst->crop_x;
    }
    if(dst->height < cinfo->output_height) {
        dst->height = cinfo->output_height;
        if(dst->crop_y + dst->crop_h > dst->height)
            dst->crop_h = dst->height - dst->crop_y;
    }
    unsigned long datalen = (cinfo->output_width *
                             cinfo->output_height *
                             cinfo->out_color_components);

    zprintf(24, "dst=%dx%d %lx src=%dx%d %lx dct=%x\n",
            dst->width, dst->height, dst->datalen,
            src->width, src->height, src->datalen, cinfo->dct_method);
    if(!dst->data) {
        dst->datalen = datalen;
        dst->data = malloc(dst->datalen);
        dst->cleanup = zbar_image_free_data;
    }
    else
        assert(datalen <= dst->datalen);
    if(!dst->data) return;

    unsigned bpl = dst->width * cinfo->output_components;
    JSAMPROW buf = (void*)dst->data;
    JSAMPARRAY line = &buf;
    for(; cinfo->output_scanline < cinfo->output_height; buf += bpl) {
        jpeg_read_scanlines(cinfo, line, 1);
        /* FIXME pad out to dst->width */
    }

    /* FIXME always do this? */
    jpeg_finish_decompress(cinfo);

 error:
    if(jerr)
        jerr->valid = 0;
    if(!src->src && cinfo)
        /* cleanup only if we allocated locally */
        _zbar_jpeg_decomp_destroy(cinfo);
}
