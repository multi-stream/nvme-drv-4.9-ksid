* Description
This folder contains NVMe driver module from v4.9 kernel. It was extended with two kernel modules providing the kernel stream ID assignment frame work as described below. You will need to obtain the matching Linux kernel from https://github.com/multi-stream/linux-ksid.git to test this.

** nvme-ksid.ko 
This is the stream ID assignment control module. Upon module initialization it finds all NVMe SSD(s) in the system and enables them with streams direct if the SSD supports it according to NVMe v1.3 specification. Then it assigns ksid related operations that allows nvme core to request stream ID from nvme-ksid. 
This module allows 3 module load parameters: vendor_id, prod_id, and id_mode. The stream ID returned by nvme-ksid is based on the id_mode value. Current supported id_mode are: 
   1 - application assigned stream ID (support in linux-ksid kernel); 
   2 - use kernel process ID (pid) that issues the write request as stream ID (support in linux-ksid kernel);
   3 - stream ID generated from custom algorithm.

** custom-ksid.ko
This is an example implementation for custom ID generation. This only provides a reference framework for a custom algorithm to source the stream ID and does not contain any algorithm implementation itself. If id_mode=2 in nvme-ksid.ko, this module always returns a stream ID of 0xbeef.

* Module load example:
The nvme modules will need to be loaded first.
# sudo insmod ./drivers/nvme/host/nvme-core.ko
# sudo insmod ./drivers/nvme/host/nvme.ko

Then load the nvme-ksid module. Example below loads the module for a qemu NVMe simulated drive. The id_mode selects the stream ID sourced from the ksid-custom module.
# sudo insmod ./drivers/nvme/host/nvme-ksid.ko vendor_id=0x8086 prod_id=0x5845 id_mode=2

Then finally load the ksid-custom module.
# sudo insmod ./drivers/nvme/host/ksid-custom.ko

