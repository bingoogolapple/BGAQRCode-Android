/*------------------------------------------------------------------------
 *  Copyright 2007-2011 (c) Jeff Brown <spadix@users.sourceforge.net>
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

#include <config.h>
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <linux/videodev.h>

#include "video.h"
#include "image.h"

typedef struct v4l1_format_s {
    uint32_t format;
    uint8_t bpp;
} v4l1_format_t;

/* static v4l1 "palette" mappings
 * documentation for v4l1 formats is terrible...
 */
static const v4l1_format_t v4l1_formats[17] = {
    /* format                   bpp */
    { 0,                         0 },
    { fourcc('G','R','E','Y'),   8 }, /* GREY */
    { fourcc('H','I','2','4'),   8 }, /* HI240 (BT848) */

    /* component ordering for RGB palettes is unspecified,
     * convention appears to place red in the most significant bits
     * FIXME is this true for other drivers? big endian machines?
     */
    { fourcc('R','G','B','P'),  16 }, /* RGB565 */
    { fourcc('B','G','R','3'),  24 }, /* RGB24 */
    { fourcc('B','G','R','4'),  32 }, /* RGB32 */
    { fourcc('R','G','B','O'),  16 }, /* RGB555 */
    { fourcc('Y','U','Y','2'),  16 }, /* YUV422 (8 bpp?!) */
    { fourcc('Y','U','Y','V'),  16 }, /* YUYV */
    { fourcc('U','Y','V','Y'),  16 }, /* UYVY */
    { 0,                        12 }, /* YUV420 (24 bpp?) FIXME?! */
    { fourcc('Y','4','1','P'),  12 }, /* YUV411 */
    { 0,                         0 }, /* Bt848 raw */
    { fourcc('4','2','2','P'),  16 }, /* YUV422P (24 bpp?) */
    { fourcc('4','1','1','P'),  12 }, /* YUV411P */
    { fourcc('Y','U','1','2'),  12 }, /* YUV420P */
    { fourcc('Y','U','V','9'),   9 }, /* YUV410P */
};

static int v4l1_nq (zbar_video_t *vdo,
                    zbar_image_t *img)
{
    if(video_nq_image(vdo, img))
        return(-1);

    if(vdo->iomode != VIDEO_MMAP)
        return(0);

    struct video_mmap vmap;
    vmap.frame = img->srcidx;
    vmap.width = vdo->width;
    vmap.height = vdo->height;
    vmap.format = vdo->palette;
    if(ioctl(vdo->fd, VIDIOCMCAPTURE, &vmap) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "initiating video capture (VIDIOCMCAPTURE)"));

    return(0);
}

static zbar_image_t *v4l1_dq (zbar_video_t *vdo)
{
    video_iomode_t iomode = vdo->iomode;
    int fd = vdo->fd;
    zbar_image_t *img = video_dq_image(vdo);
    if(!img)
        return(NULL);

    if(iomode == VIDEO_MMAP) {
        int frame = img->srcidx;
        if(ioctl(fd, VIDIOCSYNC, &frame) < 0)
            return(NULL);
    }
    else if(read(fd, (void*)img->data, img->datalen) != img->datalen)
        return(NULL);

    return(img);
}

static int v4l1_mmap_buffers (zbar_video_t *vdo)
{
#ifdef HAVE_SYS_MMAN_H
    /* map camera image to memory */
    struct video_mbuf vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    if(ioctl(vdo->fd, VIDIOCGMBUF, &vbuf) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying video frame buffers (VIDIOCGMBUF)"));
    assert(vbuf.frames && vbuf.size);

    zprintf(1, "mapping %d buffers size=0x%x\n", vbuf.frames, vbuf.size);
    vdo->buflen = vbuf.size;
    vdo->buf = mmap(0, vbuf.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    vdo->fd, 0);
    if(vdo->buf == MAP_FAILED)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "mapping video frame buffers"));

    int i;
    for(i = 0; i < vbuf.frames; i++) {
        zbar_image_t *img = vdo->images[i];
        zprintf(2, "    [%02d] @%08x\n", img->srcidx, vbuf.offsets[i]);
        img->data = vdo->buf + vbuf.offsets[i];
        img->datalen = vdo->datalen;
        int next_offset = ((i + 1 < vdo->num_images)
                           ? vbuf.offsets[i + 1]
                           : vbuf.size);
        if(next_offset < vbuf.offsets[i] + vdo->datalen)
            fprintf(stderr, "WARNING: insufficient v4l1 video buffer size:\n"
                    "\tvbuf[%d]=%x vbuf[%d]=%x datalen=%lx\n"
                    "\timage=%d x %d %.4s(%08x) palette=%d\n",
                    i, vbuf.offsets[i], i + 1, next_offset,
                    vdo->datalen, vdo->width, vdo->height,
                    (char*)&vdo->format, vdo->format, vdo->palette);
    }
    return(0);
#else
    return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_UNSUPPORTED, __func__,
                       "memory mapping not supported"));
#endif
}

static int v4l1_start (zbar_video_t *vdo)
{
    return(0);
}

static int v4l1_stop (zbar_video_t *vdo)
{
    return(0);
}

static inline int v4l1_set_format (zbar_video_t *vdo,
                                   uint32_t fmt)
{
    struct video_picture vpic;
    memset(&vpic, 0, sizeof(vpic));
    if(ioctl(vdo->fd, VIDIOCGPICT, &vpic) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying video format (VIDIOCGPICT)"));

    vdo->palette = 0;
    int ifmt;
    for(ifmt = 1; ifmt <= VIDEO_PALETTE_YUV410P; ifmt++)
        if(v4l1_formats[ifmt].format == fmt)
            break;
    if(!fmt || ifmt >= VIDEO_PALETTE_YUV410P)
        return(err_capture_int(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__,
                               "invalid v4l1 format: %x", fmt));

    vpic.palette = ifmt;
    vpic.depth = v4l1_formats[ifmt].bpp;
    if(ioctl(vdo->fd, VIDIOCSPICT, &vpic) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "setting format (VIDIOCSPICT)"));

    memset(&vpic, 0, sizeof(vpic));
    if(ioctl(vdo->fd, VIDIOCGPICT, &vpic) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying video format (VIDIOCGPICT)"));

    if(vpic.palette != ifmt || vpic.depth != v4l1_formats[ifmt].bpp) {
        fprintf(stderr,
                "WARNING: set v4l1 palette %d which should have depth %d bpp\n"
                "    but probed palette %d with depth %d bpp?"
                "  ...continuing anyway\n",
                ifmt, v4l1_formats[ifmt].bpp, vpic.palette, vpic.depth);
        err_capture_int(vdo, SEV_WARNING, ZBAR_ERR_INVALID, __func__,
                        "driver format (%x) inconsistency", fmt);
    }
    vdo->format = fmt;
    vdo->palette = ifmt;
    vdo->datalen = (vdo->width * vdo->height * v4l1_formats[ifmt].bpp + 7) >> 3;

    zprintf(1, "set new format: %.4s(%08x) depth=%d palette=%d size=0x%lx\n",
            (char*)&vdo->format, vdo->format, vpic.depth, vdo->palette,
            vdo->datalen);
    return(0);
}

static int v4l1_init (zbar_video_t *vdo,
                      uint32_t fmt)
{
    if(v4l1_set_format(vdo, fmt))
        return(-1);
    if(vdo->iomode == VIDEO_MMAP && v4l1_mmap_buffers(vdo))
        return(-1);
    return(0);
}

static int v4l1_cleanup (zbar_video_t *vdo)
{
#ifdef HAVE_SYS_MMAN_H
    /* FIXME should avoid holding onto mmap'd buffers so long? */
    if(vdo->iomode == VIDEO_MMAP && vdo->buf) {
        if(munmap(vdo->buf, vdo->buflen))
            return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                               "unmapping video frame buffers"));
        vdo->buf = NULL;
        /* FIXME reset image */
    }
#endif

    /* close open device */
    if(vdo->fd >= 0) {
        close(vdo->fd);
        vdo->fd = -1;
    }
    return(0);
}

static int v4l1_probe_iomode (zbar_video_t *vdo)
{
    vdo->iomode = VIDEO_READWRITE;
#ifdef HAVE_SYS_MMAN_H
    struct video_mbuf vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    if(ioctl(vdo->fd, VIDIOCGMBUF, &vbuf) < 0) {
        if(errno != EINVAL)
            return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                               "querying video frame buffers (VIDIOCGMBUF)"));
        /* not supported */
        return(0);
    }
    if(!vbuf.frames || !vbuf.size)
        return(0);
    vdo->iomode = VIDEO_MMAP;
    if(vdo->num_images > vbuf.frames)
        vdo->num_images = vbuf.frames;
#endif
    zprintf(1, "using %d images in %s mode\n", vdo->num_images,
            (vdo->iomode == VIDEO_READWRITE) ? "READ" : "MMAP");
    return(0);
}

static inline int v4l1_probe_formats (zbar_video_t *vdo)
{
    struct video_picture vpic;
    memset(&vpic, 0, sizeof(vpic));
    if(ioctl(vdo->fd, VIDIOCGPICT, &vpic) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying format (VIDIOCGPICT)"));

    vdo->format = 0;
    if(vpic.palette <= VIDEO_PALETTE_YUV410P)
        vdo->format = v4l1_formats[vpic.palette].format;

    zprintf(1, "current format: %.4s(%08x) depth=%d palette=%d\n",
            (char*)&vdo->format, vdo->format, vpic.depth, vpic.palette);

    vdo->formats = calloc(16, sizeof(uint32_t));
    if(!vdo->formats)
        return(err_capture(vdo, SEV_FATAL, ZBAR_ERR_NOMEM, __func__,
                           "allocating format list"));

    int num_formats = 0;
    zprintf(2, "probing supported formats:\n");
    int i;
    for(i = 1; i <= VIDEO_PALETTE_YUV410P; i++) {
        if(!v4l1_formats[i].format)
            continue;
        vpic.depth = v4l1_formats[i].bpp;
        vpic.palette = i;
        if(ioctl(vdo->fd, VIDIOCSPICT, &vpic) < 0) {
            zprintf(2, "    [%02d] %.4s...no (set fails)\n",
                    i, (char*)&v4l1_formats[i].format);
            continue;
        }
        if(ioctl(vdo->fd, VIDIOCGPICT, &vpic) < 0 ||
           vpic.palette != i) {
            zprintf(2, "    [%02d] %.4s...no (set ignored)\n",
                    i, (char*)&v4l1_formats[i].format);
            continue;
        }
        zprintf(2, "    [%02d] %.4s...yes\n",
                i, (char*)&v4l1_formats[i].format);
        vdo->formats[num_formats++] = v4l1_formats[i].format;
    }
    vdo->formats = realloc(vdo->formats, (num_formats + 1) * sizeof(uint32_t));
    assert(vdo->formats);

    return(v4l1_set_format(vdo, vdo->format));
}

static inline int v4l1_init_window (zbar_video_t *vdo)
{
    struct video_window vwin;
    memset(&vwin, 0, sizeof(vwin));
    if(ioctl(vdo->fd, VIDIOCGWIN, &vwin) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying video window settings (VIDIOCGWIN)"));

    zprintf(1, "current window: %d x %d @(%d, %d)%s\n",
            vwin.width, vwin.height, vwin.x, vwin.y,
            (vwin.flags & 1) ? " INTERLACE" : "");

    if(vwin.width == vdo->width && vwin.height == vdo->height)
        /* max window already set */
        return(0);

    struct video_window maxwin;
    memcpy(&maxwin, &vwin, sizeof(maxwin));
    maxwin.width = vdo->width;
    maxwin.height = vdo->height;

    zprintf(1, "setting max win: %d x %d @(%d, %d)%s\n",
            maxwin.width, maxwin.height, maxwin.x, maxwin.y,
            (maxwin.flags & 1) ? " INTERLACE" : "");
    if(ioctl(vdo->fd, VIDIOCSWIN, &maxwin) < 0) {
        zprintf(1, "set FAILED...trying to recover original window\n");
        /* ignore errors (driver broken anyway) */
        ioctl(vdo->fd, VIDIOCSWIN, &vwin);
    }

    /* re-query resulting parameters */
    memset(&vwin, 0, sizeof(vwin));
    if(ioctl(vdo->fd, VIDIOCGWIN, &vwin) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying video window settings (VIDIOCGWIN)"));

    zprintf(1, "    final window: %d x %d @(%d, %d)%s\n",
            vwin.width, vwin.height, vwin.x, vwin.y,
            (vwin.flags & 1) ? " INTERLACE" : "");
    vdo->width = vwin.width;
    vdo->height = vwin.height;
    return(0);
}

int _zbar_v4l1_probe (zbar_video_t *vdo)
{
    /* check capabilities */
    struct video_capability vcap;
    memset(&vcap, 0, sizeof(vcap));
    if(ioctl(vdo->fd, VIDIOCGCAP, &vcap) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_UNSUPPORTED, __func__,
                           "video4linux version 1 not supported (VIDIOCGCAP)"));

    zprintf(1, "%s (%sCAPTURE) (%d x %d) - (%d x %d)\n",
            vcap.name, (vcap.type & VID_TYPE_CAPTURE) ? "" : "*NO* ",
            vcap.minwidth, vcap.minheight, vcap.maxwidth, vcap.maxheight);

    if(!(vcap.type & VID_TYPE_CAPTURE))
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_UNSUPPORTED, __func__,
                           "v4l1 device does not support CAPTURE"));

    if(!vdo->width || !vdo->height) {
        vdo->width = vcap.maxwidth;
        vdo->height = vcap.maxheight;
    }

    if(v4l1_init_window(vdo) ||
       v4l1_probe_formats(vdo) ||
       v4l1_probe_iomode(vdo))
        return(-1);

    vdo->intf = VIDEO_V4L1;
    vdo->init = v4l1_init;
    vdo->cleanup = v4l1_cleanup;
    vdo->start = v4l1_start;
    vdo->stop = v4l1_stop;
    vdo->nq = v4l1_nq;
    vdo->dq = v4l1_dq;
    return(0);
}
