#
# Makefile for the sensor drivers.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_SENSOR_GP2AP)	+= gp2ap002.o
obj-$(CONFIG_SENSOR_BMA150)	+= bma150.o
obj-$(CONFIG_SENSOR_BMA250) += bma250_driver.o
obj-$(CONFIG_SENSOR_K3DH)	+= k3dh.o
ifeq ($(CONFIG_MACH_MSM7X27_ALESSI), y)
obj-$(CONFIG_SENSOR_KR3DM)	+= kr3dm_alessi.o
else
obj-$(CONFIG_SENSOR_KR3DM)	+= kr3dm.o
endif
obj-$(CONFIG_SENSOR_KR3DH)	+= kr3dh.o
obj-$(CONFIG_SENSOR_AK8973)	+= akm8973.o
obj-$(CONFIG_SENSOR_AMI602) += ami602.o
ifeq ($(CONFIG_MACH_MSM7X27_ALESSI), y)
obj-$(CONFIG_SENSOR_AMI304) += ami304_alessi.o
else
obj-$(CONFIG_SENSOR_AMI304) += ami304.o
endif
obj-$(CONFIG_SENSOR_APDS9190) += apds9190.o
obj-$(CONFIG_SENSORS_APDS9801) += apds9801.o
