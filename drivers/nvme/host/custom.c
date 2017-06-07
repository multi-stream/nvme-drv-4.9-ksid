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


#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include "ksid.h"

static int custom_reset(struct gendisk *disk)
{
	return 0;
}

u16 custom_get_stream_id(struct gendisk *disk)
{
	return 0xbeef;
}

static const struct custom_stream_id_ops c_ops = {
        .reset      = custom_reset,
        .get_id     = custom_get_stream_id,
};

int custom_sid_init(void)
{
	struct ksid_custom *custom = ksid_get_custom();
	if (custom)
		custom->custom_ops = &c_ops;
	return 0;
}

void custom_sid_exit(void)
{
	struct ksid_custom *custom = ksid_get_custom();
	if (custom)
		custom->custom_ops = NULL;
}

module_init(custom_sid_init);
module_exit(custom_sid_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hingkwan Huen <kwan.huen@ssi.samsung.com>");
MODULE_DESCRIPTION("Example custom stream ID generation for ksid");

