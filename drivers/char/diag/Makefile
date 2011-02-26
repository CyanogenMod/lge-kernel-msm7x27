obj-$(CONFIG_DIAG_CHAR) := diagchar.o
obj-$(CONFIG_DIAG_SDIO_PIPE) += diagfwd_sdio.o
diagchar-objs := diagchar_core.o diagchar_hdlc.o diagfwd.o diagmem.o
ifeq ($(CONFIG_LGE_DIAGTEST), y)
diagchar-objs += lg_diag_kernel_service.o
endif
