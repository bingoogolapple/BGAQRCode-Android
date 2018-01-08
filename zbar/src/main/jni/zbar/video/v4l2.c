/*------------------------------------------------------------------------
 *  Copyright 2007-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
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
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#include <linux/videodev2.h>

#include "video.h"
#include "image.h"

#define V4L2_FORMATS_MAX 64

static int v4l2_nq (zbar_video_t *vdo,
                    zbar_image_t *img)
{
    if(vdo->iomode == VIDEO_READWRITE)
        return(video_nq_image(vdo, img));

    if(video_unlock(vdo))
        return(-1);

    struct v4l2_buffer vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(vdo->iomode == VIDEO_MMAP) {
        vbuf.memory = V4L2_MEMORY_MMAP;
        vbuf.index = img->srcidx;
    }
    else {
        vbuf.memory = V4L2_MEMORY_USERPTR;
        vbuf.m.userptr = (unsigned long)img->data;
        vbuf.length = img->datalen;
        vbuf.index = img->srcidx; /* FIXME workaround broken drivers */
    }
    if(ioctl(vdo->fd, VIDIOC_QBUF, &vbuf) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "queuing video buffer (VIDIOC_QBUF)"));
    return(0);
}

static zbar_image_t *v4l2_dq (zbar_video_t *vdo)
{
    zbar_image_t *img;
    int fd = vdo->fd;

    if(vdo->iomode != VIDEO_READWRITE) {
        video_iomode_t iomode = vdo->iomode;
        if(video_unlock(vdo))
            return(NULL);

        struct v4l2_buffer vbuf;
        memset(&vbuf, 0, sizeof(vbuf));
        vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if(iomode == VIDEO_MMAP)
            vbuf.memory = V4L2_MEMORY_MMAP;
        else
            vbuf.memory = V4L2_MEMORY_USERPTR;

        if(ioctl(fd, VIDIOC_DQBUF, &vbuf) < 0)
            return(NULL);

        if(iomode == VIDEO_MMAP) {
            assert(vbuf.index >= 0);
            assert(vbuf.index < vdo->num_images);
            img = vdo->images[vbuf.index];
        }
        else {
            /* reverse map pointer back to image (FIXME) */
            assert(vbuf.m.userptr >= (unsigned long)vdo->buf);
            assert(vbuf.m.userptr < (unsigned long)(vdo->buf + vdo->buflen));
            int i = (vbuf.m.userptr - (unsigned long)vdo->buf) / vdo->datalen;
            assert(i >= 0);
            assert(i < vdo->num_images);
            img = vdo->images[i];
            assert(vbuf.m.userptr == (unsigned long)img->data);
        }
    }
    else {
        img = video_dq_image(vdo);
        if(!img)
            return(NULL);

        /* FIXME should read entire image */
        unsigned long datalen = read(fd, (void*)img->data, img->datalen);
        if(datalen < 0)
            return(NULL);
        else if(datalen != img->datalen)
            zprintf(0, "WARNING: read() size mismatch: 0x%lx != 0x%lx\n",
                    datalen, img->datalen);
    }
    return(img);
}

static int v4l2_start (zbar_video_t *vdo)
{
    if(vdo->iomode == VIDEO_READWRITE)
        return(0);

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(vdo->fd, VIDIOC_STREAMON, &type) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "starting video stream (VIDIOC_STREAMON)"));
    return(0);
}

static int v4l2_stop (zbar_video_t *vdo)
{
    if(vdo->iomode == VIDEO_READWRITE)
        return(0);

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(vdo->fd, VIDIOC_STREAMOFF, &type) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "stopping video stream (VIDIOC_STREAMOFF)"));
    return(0);
}

static int v4l2_cleanup (zbar_video_t *vdo)
{
    if(vdo->iomode == VIDEO_READWRITE)
        return(0);

    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(rb));
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(vdo->iomode == VIDEO_MMAP) {
        rb.memory = V4L2_MEMORY_MMAP;
        int i;
        for(i = 0; i < vdo->num_images; i++) {
            zbar_image_t *img = vdo->images[i];
            if(img->data &&
               munmap((void*)img->data, img->datalen))
                err_capture(vdo, SEV_WARNING, ZBAR_ERR_SYSTEM, __func__,
                            "unmapping video frame buffers");
            img->data = NULL;
            img->datalen = 0;
        }
    }
    else
        rb.memory = V4L2_MEMORY_USERPTR;

    /* requesting 0 buffers
     * should implicitly disable streaming
     */
    if(ioctl(vdo->fd, VIDIOC_REQBUFS, &rb) < 0)
        err_capture(vdo, SEV_WARNING, ZBAR_ERR_SYSTEM, __func__,
                    "releasing video frame buffers (VIDIOC_REQBUFS)");


    /* close open device */
    if(vdo->fd >= 0) {
        close(vdo->fd);
        vdo->fd = -1;
    }
    return(0);
}

static int v4l2_mmap_buffers (zbar_video_t *vdo)
{
    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(rb));
    rb.count = vdo->num_images;
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rb.memory = V4L2_MEMORY_MMAP;
    if(ioctl(vdo->fd, VIDIOC_REQBUFS, &rb) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "requesting video frame buffers (VIDIOC_REQBUFS)"));
    zprintf(1, "mapping %u buffers (of %d requested)\n",
            rb.count, vdo->num_images);
    if(!rb.count)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__,
                           "driver returned 0 buffers"));
    if(vdo->num_images > rb.count)
        vdo->num_images = rb.count;

    struct v4l2_buffer vbuf;
    memset(&vbuf, 0, sizeof(vbuf));
    vbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vbuf.memory = V4L2_MEMORY_MMAP;

    int i;
    for(i = 0; i < vdo->num_images; i++) {
        vbuf.index = i;
        if(ioctl(vdo->fd, VIDIOC_QUERYBUF, &vbuf) < 0)
            /* FIXME cleanup */
            return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                               "querying video buffer (VIDIOC_QUERYBUF)"));

        if(vbuf.length < vdo->datalen)
            fprintf(stderr, "WARNING: insufficient v4l2 video buffer size:\n"
                    "\tvbuf[%d].length=%x datalen=%lx image=%d x %d %.4s(%08x)\n",
                    i, vbuf.length, vdo->datalen, vdo->width, vdo->height,
                    (char*)&vdo->format, vdo->format);

        zbar_image_t *img = vdo->images[i];
        img->datalen = vbuf.length;
        img->data = mmap(NULL, vbuf.length, PROT_READ | PROT_WRITE, MAP_SHARED,
                         vdo->fd, vbuf.m.offset);
        if(img->data == MAP_FAILED)
            /* FIXME cleanup */
            return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                               "mapping video frame buffers"));
        zprintf(2, "    buf[%d] 0x%lx bytes @%p\n",
                i, img->datalen, img->data);
    }
    return(0);
}

static int v4l2_set_format (zbar_video_t *vdo,
                            uint32_t fmt)
{
    struct v4l2_format vfmt;
    struct v4l2_pix_format *vpix = &vfmt.fmt.pix;
    memset(&vfmt, 0, sizeof(vfmt));
    vfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    vpix->width = vdo->width;
    vpix->height = vdo->height;
    vpix->pixelformat = fmt;
    vpix->field = V4L2_FIELD_NONE;
    int rc = 0;
    if((rc = ioctl(vdo->fd, VIDIOC_S_FMT, &vfmt)) < 0) {
        /* several broken drivers return an error if we request
         * no interlacing (NB v4l2 spec violation)
         * ...try again with an interlaced request
         */
        zprintf(1, "VIDIOC_S_FMT returned %d(%d), trying interlaced...\n",
                rc, errno);

        /* FIXME this might be _ANY once we can de-interlace */
        vpix->field = V4L2_FIELD_INTERLACED;

        if(ioctl(vdo->fd, VIDIOC_S_FMT, &vfmt) < 0)
            return(err_capture_int(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                                   "setting format %x (VIDIOC_S_FMT)", fmt));

        zprintf(0, "WARNING: broken driver returned error when non-interlaced"
                " format requested\n");
    }

    struct v4l2_format newfmt;
    struct v4l2_pix_format *newpix = &newfmt.fmt.pix;
    memset(&newfmt, 0, sizeof(newfmt));
    newfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(vdo->fd, VIDIOC_G_FMT, &newfmt) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying format (VIDIOC_G_FMT)"));

    if(newpix->field != V4L2_FIELD_NONE)
        err_capture(vdo, SEV_WARNING, ZBAR_ERR_INVALID, __func__,
                    "video driver only supports interlaced format,"
                    " vertical scanning may not work");

    if(newpix->pixelformat != fmt
       /* FIXME bpl/bpp checks? */)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__,
                           "video driver can't provide compatible format"));

    vdo->format = fmt;
    vdo->width = newpix->width;
    vdo->height = newpix->height;
    vdo->datalen = newpix->sizeimage;

    zprintf(1, "set new format: %.4s(%08x) %u x %u (0x%lx)\n",
            (char*)&vdo->format, vdo->format, vdo->width, vdo->height,
            vdo->datalen);
    return(0);
}

static int v4l2_init (zbar_video_t *vdo,
                      uint32_t fmt)
{
    if(v4l2_set_format(vdo, fmt))
        return(-1);
    if(vdo->iomode == VIDEO_MMAP)
        return(v4l2_mmap_buffers(vdo));
    return(0);
}

static int v4l2_probe_iomode (zbar_video_t *vdo)
{
    struct v4l2_requestbuffers rb;
    memset(&rb, 0, sizeof(rb));
    rb.count = vdo->num_images; /* FIXME workaround broken drivers */
    rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(vdo->iomode == VIDEO_MMAP)
        rb.memory = V4L2_MEMORY_MMAP;
    else
        rb.memory = V4L2_MEMORY_USERPTR;

    if(ioctl(vdo->fd, VIDIOC_REQBUFS, &rb) < 0) {
        if(vdo->iomode)
            return(err_capture_int(vdo, SEV_ERROR, ZBAR_ERR_INVALID, __func__,
                                   "unsupported iomode requested (%d)",
                                   vdo->iomode));
        else if(errno != EINVAL)
            return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                               "querying streaming mode (VIDIOC_REQBUFS)"));
#ifdef HAVE_SYS_MMAN_H
        vdo->iomode = VIDEO_MMAP;
#endif
    }
    else {
        if(!vdo->iomode)
            vdo->iomode = VIDEO_USERPTR;
        if(rb.count)
            vdo->num_images = rb.count;
    }
    return(0);
}

static inline int v4l2_probe_formats (zbar_video_t *vdo)
{
    zprintf(2, "enumerating supported formats:\n");
    struct v4l2_fmtdesc desc;
    memset(&desc, 0, sizeof(desc));
    desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for(desc.index = 0; desc.index < V4L2_FORMATS_MAX; desc.index++) {
        if(ioctl(vdo->fd, VIDIOC_ENUM_FMT, &desc) < 0)
            break;
        zprintf(2, "    [%d] %.4s : %s%s\n",
                desc.index, (char*)&desc.pixelformat, desc.description,
                (desc.flags & V4L2_FMT_FLAG_COMPRESSED) ? " COMPRESSED" : "");
        vdo->formats = realloc(vdo->formats,
                               (desc.index + 2) * sizeof(uint32_t));
        vdo->formats[desc.index] = desc.pixelformat;
    }
    if(!desc.index)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "enumerating video formats (VIDIOC_ENUM_FMT)"));
    vdo->formats[desc.index] = 0;

    struct v4l2_format fmt;
    struct v4l2_pix_format *pix = &fmt.fmt.pix;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(vdo->fd, VIDIOC_G_FMT, &fmt) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying current video format (VIDIO_G_FMT)"));

    zprintf(1, "current format: %.4s(%08x) %u x %u%s (line=0x%x size=0x%x)\n",
            (char*)&pix->pixelformat, pix->pixelformat,
            pix->width, pix->height,
            (pix->field != V4L2_FIELD_NONE) ? " INTERLACED" : "",
            pix->bytesperline, pix->sizeimage);

    vdo->format = pix->pixelformat;
    vdo->datalen = pix->sizeimage;
    if(pix->width == vdo->width && pix->height == vdo->height)
        return(0);

    struct v4l2_format maxfmt;
    struct v4l2_pix_format *maxpix = &maxfmt.fmt.pix;
    memcpy(&maxfmt, &fmt, sizeof(maxfmt));
    maxpix->width = vdo->width;
    maxpix->height = vdo->height;

    zprintf(1, "setting requested size: %d x %d\n", vdo->width, vdo->height);
    if(ioctl(vdo->fd, VIDIOC_S_FMT, &maxfmt) < 0) {
        zprintf(1, "set FAILED...trying to recover original format\n");
        /* ignore errors (driver broken anyway) */
        ioctl(vdo->fd, VIDIOC_S_FMT, &fmt);
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(vdo->fd, VIDIOC_G_FMT, &fmt) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying current video format (VIDIOC_G_FMT)"));

    zprintf(1, "final format: %.4s(%08x) %u x %u%s (line=0x%x size=0x%x)\n",
            (char*)&pix->pixelformat, pix->pixelformat,
            pix->width, pix->height,
            (pix->field != V4L2_FIELD_NONE) ? " INTERLACED" : "",
            pix->bytesperline, pix->sizeimage);

    vdo->width = pix->width;
    vdo->height = pix->height;
    vdo->datalen = pix->sizeimage;
    return(0);
}

static inline int v4l2_reset_crop (zbar_video_t *vdo)
{
    /* check cropping */
    struct v4l2_cropcap ccap;
    memset(&ccap, 0, sizeof(ccap));
    ccap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(vdo->fd, VIDIOC_CROPCAP, &ccap) < 0)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "querying crop support (VIDIOC_CROPCAP)"));

    zprintf(1, "crop bounds: %d x %d @ (%d, %d)\n",
            ccap.bounds.width, ccap.bounds.height,
            ccap.bounds.left, ccap.bounds.top);
    zprintf(1, "current crop win: %d x %d @ (%d, %d) aspect %d / %d\n",
            ccap.defrect.width, ccap.defrect.height,
            ccap.defrect.left, ccap.defrect.top,
            ccap.pixelaspect.numerator, ccap.pixelaspect.denominator);

    if(!vdo->width || !vdo->height) {
        vdo->width = ccap.defrect.width;
        vdo->height = ccap.defrect.height;
    }

    /* reset crop parameters */
    struct v4l2_crop crop;
    memset(&crop, 0, sizeof(crop));
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = ccap.defrect;
    if(ioctl(vdo->fd, VIDIOC_S_CROP, &crop) < 0 && errno != EINVAL)
        return(err_capture(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                           "setting default crop window (VIDIOC_S_CROP)"));
    return(0);
}

int _zbar_v4l2_probe (zbar_video_t *vdo)
{
    /* check capabilities */
    struct v4l2_capability vcap;
    memset(&vcap, 0, sizeof(vcap));
    if(ioctl(vdo->fd, VIDIOC_QUERYCAP, &vcap) < 0)
        return(err_capture(vdo, SEV_WARNING, ZBAR_ERR_UNSUPPORTED, __func__,
                           "video4linux version 2 not supported (VIDIOC_QUERYCAP)"));

    
    zprintf(1, "%.32s on %.32s driver %.16s (version %u.%u.%u)\n", vcap.card,
            (vcap.bus_info[0]) ? (char*)vcap.bus_info : "<unknown>",
            vcap.driver, (vcap.version >> 16) & 0xff,
            (vcap.version >> 8) & 0xff, vcap.version & 0xff);
    zprintf(1, "    capabilities:%s%s%s%s\n",
            (vcap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ? " CAPTURE" : "",
            (vcap.capabilities & V4L2_CAP_VIDEO_OVERLAY) ? " OVERLAY" : "",
            (vcap.capabilities & V4L2_CAP_READWRITE) ? " READWRITE" : "",
            (vcap.capabilities & V4L2_CAP_STREAMING) ? " STREAMING" : "");

    if(!(vcap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
       !(vcap.capabilities & (V4L2_CAP_READWRITE | V4L2_CAP_STREAMING)))
        return(err_capture(vdo, SEV_WARNING, ZBAR_ERR_UNSUPPORTED, __func__,
                           "v4l2 device does not support usable CAPTURE"));

    if(v4l2_reset_crop(vdo))
        /* ignoring errors (driver cropping support questionable) */;

    if(!vdo->width || !vdo->height) {
        /* fallback to large size, driver reduces to max available */
        vdo->width = 640 * 64;
        vdo->height = 480 * 64;
    }

    if(v4l2_probe_formats(vdo))
        return(-1);

    /* FIXME report error and fallback to readwrite? (if supported...) */
    if(vdo->iomode != VIDEO_READWRITE &&
       (vcap.capabilities & V4L2_CAP_STREAMING) &&
       v4l2_probe_iomode(vdo))
        return(-1);
    if(!vdo->iomode)
        vdo->iomode = VIDEO_READWRITE;

    zprintf(1, "using I/O mode: %s\n",
            (vdo->iomode == VIDEO_READWRITE) ? "READWRITE" :
            (vdo->iomode == VIDEO_MMAP) ? "MMAP" :
            (vdo->iomode == VIDEO_USERPTR) ? "USERPTR" : "<UNKNOWN>");

    vdo->intf = VIDEO_V4L2;
    vdo->init = v4l2_init;
    vdo->cleanup = v4l2_cleanup;
    vdo->start = v4l2_start;
    vdo->stop = v4l2_stop;
    vdo->nq = v4l2_nq;
    vdo->dq = v4l2_dq;
    return(0);
}
