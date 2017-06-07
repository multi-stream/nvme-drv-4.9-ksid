KERNROOT ?= /lib/modules/`uname -r`/build

all: nvme nvmet

nvme:
	$(MAKE) -C $(KERNROOT) M=`pwd`/drivers/nvme/host
nvmet:
	$(MAKE) -C $(KERNROOT) M=`pwd`/drivers/nvme/target

clean:
	$(MAKE) -C $(KERNROOT) M=`pwd`/drivers/nvme/host clean
	$(MAKE) -C $(KERNROOT) M=`pwd`/drivers/nvme/target clean

.PHONY: all clean nvme nvmet
