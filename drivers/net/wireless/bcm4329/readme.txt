1. kernel/drivers/net/wireless/Kconfig 수정
- source "drivers/net/wireless/bcm4329/Kconfig" 추가

2. kernel/drivers/net/wireless/Makefile 수정
- obj-$(CONFIG_BCM4329)		+= bcm4329/ 추가

3. kernel/AndroidKernel.mk
- $(TARGET_PREBUILT_KERNEL): $(KERNEL_OUT) $(KERNEL_CONFIG)
        $(MAKE) -C kernel O=../$(KERNEL_OUT) ARCH=arm CROSS_COMPILE=arm-eabi-
        cp $(KERNEL_OUT)/drivers/net/wireless/bcm4329/wireless.ko vendor/lge/hardware/wifi_driver

위와 같이 wireless.ko를 copy 하도록 추가함.
- AndroidKernel.mk에 cp를 넣을려고 고려한것은 vendor/qcom/msm7627_surf/AndroidBoard.mk 에서 "include kernel/AndroidKernel.mk" 되어 있는 것을 보고 AndroidKernel.mk에서 복사하도록 함.

4. menuconfig 에서
- Networing support -> Wireless -> Wireless extensions

- Device Drivers -> MMC/SD/SDIO card support -> Allow unsafe resume
- Device Drivers -> MMC/SD/SDIO card support -> MMC block device driver
- Device Drivers -> MMC/SD/SDIO card support -> Use bounce buffer for simple hosts
- Device Drivers -> MMC/SD/SDIO card support -> Chec card status on resume

- Device Drivers -> MMC/SD/SDIO card support -> Qualcomm SDCC Controller Support
- Device Drivers -> MMC/SD/SDIO card support -> Qualcomm MSM SDIO support
- Device Drivers -> MMC/SD/SDIO card support -> Qualcomm MMC Hardware detection support
- Device Drivers -> MMC/SD/SDIO card support -> Qualcomm SDC1 support

- Device Drivers -> Network device support -> Wireless LAN -> Wireless LAN (802.11)
- (M) Device Drivers -> Network device support -> Wireless LAN -> Broadcom BCM4329 wireless support
- Device Drivers -> Network device support -> Wireless LAN -> GPIO BT_RESET, GPIO_WL_RESET, GPIO_WL_WAKE


