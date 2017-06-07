/*
 * Copyright (c) 2016-2017, Samsung Semiconductor.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#ifndef _KSID_H
#define _KSID_H

#include <linux/blkdev.h>

struct ksid_custom {
	const struct custom_stream_id_ops *custom_ops;
};

struct custom_stream_id_ops {
        int (*reset)(struct gendisk *disk);
        u16 (*get_id)(struct gendisk *disk);
};

struct ksid_custom *ksid_get_custom(void);

#endif /* _KSID_H */
