/*
 * NVM Express streams directive driver
 * Copyright (c) 2016-2017, Samsung Semiconductor, Inc.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include "nvme.h"
#include "ksid.h"

/* default Qemu device */
#define    VENDOR_ID    0x8086 
#define    PROD_ID      0x5845 

#ifndef PCI_CLASS_STORAGE_EXPRESS
#define PCI_CLASS_STORAGE_EXPRESS       0x010802
#endif

#define MOD_NAME "nvme-ksid"

#define KSID_ENABLE  NVME_DIR_ENDIR
#define KSID_DISABLE 0

static int id_mode = 0;
module_param(id_mode, int, 0644);
MODULE_PARM_DESC(id_mode, "mode select for stream id");

static int vendor_id = VENDOR_ID;
module_param(vendor_id, int, 0644);
MODULE_PARM_DESC(vendor_id, "vendor id for target nvme");

static int prod_id = PROD_ID;
module_param(prod_id, int, 0644);
MODULE_PARM_DESC(prod_id, "product id for target nvme");

static struct ksid_custom custom;
static int ksid_control_set(unsigned en);

struct ksid_custom *ksid_get_custom(void)
{
	return &custom;
}
EXPORT_SYMBOL_GPL(ksid_get_custom);

static int ksid_directive_support(
		struct nvme_ctrl *dev,
		unsigned nsid,
		unsigned dir,
		unsigned en)
{
        struct nvme_command c = { };
        c.directive.opcode = nvme_admin_directive_send;
        c.directive.nsid = cpu_to_le32(nsid);
	c.directive.doper = NVME_DIR_SND_ID_OP_ENABLE;
        c.directive.dtype = NVME_DIR_IDENTIFY;
        c.directive.tdtype = dir;
        c.directive.endir = en;

        return nvme_submit_sync_cmd(dev->admin_q, &c, NULL, 0);
}

static int ksid_reset(struct nvme_ctrl *ctrl)
{
	return ksid_control_set(KSID_ENABLE);
}

static u16 ksid_get_stream_id(struct nvme_ns *ns, struct bio *bio)
{
	u16 id;
	switch(id_mode)
	{
		case 0: /* app assigned id */
			if (bio)
				id = bio->bi_streamid & 0xFFFF;
			else
				id = 0;
			break;
		case 1: /* pid from block layer */
			if (bio)
				id = bio->bi_streamid >> 16 & 0xFFFF;
			else
				id = 0;
			break;
		case 2: /* reserved for custom algorithm */
			if (custom.custom_ops)
				id = custom.custom_ops->get_id(ns->disk);
			else
				id = 0;
			break;
		default:
			id = 0;
	}
	return id;
}

static const struct nvme_ksid_ops ksid_ops = {
        .reset      = ksid_reset,
        .get_id     = ksid_get_stream_id,
};

static int ksid_control_set(unsigned en)
{
    int dev_cnt = 0;
    struct pci_dev *pdev = NULL;
    struct nvme_ctrl *ctrl;

    /* Finding the device by Vendor/Device ID Pair*/
    pdev = pci_get_device(vendor_id, prod_id, pdev);
    while (pdev != NULL) {
        printk(KERN_ALERT "%s: NVMe device PID:VID %.4x:%.4x found at %s\n",
			 MOD_NAME, vendor_id, prod_id, pci_name(pdev));
	dev_cnt++;

	ctrl = nvme_pci_to_ctrl(pdev);
	if (ctrl && (ctrl->oacs & NVME_CTRL_OACS_DIRECTIVE)) {
		if (ksid_directive_support(ctrl, 0xffffffff,
				NVME_DIR_STREAMS, en))
		{
			printk(KERN_ALERT
			   "%s: Failed %s streams\n", 
			   MOD_NAME, en?"enable":"disable");
		}
		else
		{
			if (en)
				ctrl->ksid_ops = &ksid_ops;
			else
				ctrl->ksid_ops = NULL;
		}
	}
	pdev = pci_get_device(vendor_id, prod_id, pdev);
    }
    printk(KERN_ALERT "%s: %s PID:VID %.4x:%.4x count: %d\n",
			MOD_NAME, en?"enable":"disable",
			vendor_id, prod_id, dev_cnt);

    /* TODO: 	use following to detect nvme ssd. Then check for
		streams directive support per nvme v1.3 spec. */

    /* Finding the device by its class code */
#if 0
    pdev = pci_get_class(PCI_CLASS_STORAGE_EXPRESS, pdev);
    while (pdev != NULL) {
	printk(KERN_ALERT "%s: NVMe Storage Class device %.4x:%.4x found!\n",
		MOD_NAME, pdev->vendor, pdev->device);
	dev_cnt++;
	pdev = pci_get_class(PCI_CLASS_STORAGE_EXPRESS, pdev);
    }
    printk(KERN_ALERT "%s: NVMe Storage Class device count: %d\n",
		MOD_NAME, dev_cnt);
#endif

    if (dev_cnt == 0)
	return -ENODEV;
    return 0;
}

static int ksid_init(void)
{
	return ksid_control_set(KSID_ENABLE);
}

static void ksid_exit(void)
{
	ksid_control_set(KSID_DISABLE);
}

module_init(ksid_init);
module_exit(ksid_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hingkwan Huen <kwan.huen@ssi.samsung.com>");
MODULE_DESCRIPTION("Provide write stream ID to streams directive enabled device");
