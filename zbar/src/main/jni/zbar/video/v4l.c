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
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "video.h"

extern int _zbar_v4l1_probe(zbar_video_t*);
extern int _zbar_v4l2_probe(zbar_video_t*);

int _zbar_video_open (zbar_video_t *vdo,
                      const char *dev)
{
    vdo->fd = open(dev, O_RDWR);
    if(vdo->fd < 0)
        return(err_capture_str(vdo, SEV_ERROR, ZBAR_ERR_SYSTEM, __func__,
                               "opening video device '%s'", dev));
    zprintf(1, "opened camera device %s (fd=%d)\n", dev, vdo->fd);

    int rc = -1;
#ifdef HAVE_LINUX_VIDEODEV2_H
    if(vdo->intf != VIDEO_V4L1)
        rc = _zbar_v4l2_probe(vdo);
#endif
#ifdef HAVE_LINUX_VIDEODEV_H
    if(rc && vdo->intf != VIDEO_V4L2)
        rc = _zbar_v4l1_probe(vdo);
#endif

    if(rc && vdo->fd >= 0) {
        close(vdo->fd);
        vdo->fd = -1;
    }
    return(rc);
}
