// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Christoph Stoidner <c.stoidner@phytec.de>
 * Copyright (C) 2024 Mathieu Othacehe <m.othacehe@gmail.com>
 */

#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/mu.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/trdc.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/ele_api.h>
#include <asm/sections.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <spl.h>

#include "../common/imx91_93_som_detection.h"

DECLARE_GLOBAL_DATA_PTR;

#define EEPROM_ADDR	0x50

/*
 * Will be part of drivers/power/regulator/pca9450.c
 * when pca9451a support is added.
 */
#define PCA9450_REG_PWRCTRL_TOFF_DEB    BIT(5)

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_board_init(void)
{
	int ret;

	ret = ele_start_rng();
	if (ret)
		printf("Fail to start RNG: %d\n", ret);

	puts("Normal Boot\n");
}

void spl_dram_init(void)
{
	if (IS_ENABLED(CONFIG_IMX91)) {
		int ret;
		u8 ddr_option = PHYTEC_IMX91_93_LPDDR4_1GB;

		ret = phytec_eeprom_data_setup(NULL, CONFIG_PHYTEC_EEPROM_BUS,
					      EEPROM_ADDR);
		if (!ret && !phytec_imx91_93_detect(NULL)) {
			phytec_print_som_info(NULL);
			if (!IS_ENABLED(CONFIG_PHYCORE_IMX91_93_RAM_TYPE_FIX))
				ddr_option = phytec_imx91_93_get_opt(NULL,
						PHYTEC_IMX91_93_OPT_DDR);
		}

		if (ddr_option != PHYTEC_IMX91_93_LPDDR4_1GB)
			puts("Unsupported RAM option; using LPDDR4 1GB timing\n");
	}

	ddr_init(&dram_timing);
}

int power_init_board(void)
{
	struct udevice *dev;
	int ret;
	unsigned int val = 0;

	ret = pmic_get("pmic@25", &dev);
	if (ret == -ENODEV) {
		puts("No pca9450@25\n");
		return 0;
	}

	if (ret != 0)
		return ret;

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);

	/* enable DVS control through PMIC_STBY_REQ */
	pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);

	ret = pmic_reg_read(dev, PCA9450_PWR_CTRL);
	if (ret < 0)
		return ret;
	val = ret;

	if (IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE)) {
		/* 0.8v for Low drive mode */
		if (val & PCA9450_REG_PWRCTRL_TOFF_DEB) {
			pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x0c);
			pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x0c);
		} else {
			pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x10);
			pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x10);
		}
	} else {
		/* 0.9v for Over drive mode */
		if (val & PCA9450_REG_PWRCTRL_TOFF_DEB) {
			pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x14);
			pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x14);
		} else {
			pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, 0x18);
			pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, 0x18);
		}
	}

	if (IS_ENABLED(CONFIG_IMX91)) {
		ret = pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x28);
		if (ret)
			return ret;

		ret = pmic_reg_read(dev, PCA9450_BUCK2OUT_DVS0);
		if (ret < 0)
			return ret;
		if ((ret & PCA9450_DVS_BUCK_RUN_MASK) != 0x28) {
			printf("PMIC: DDR VDDQ configuration failed: %#x\n", ret);
			return -EIO;
		}
	}

	/* set standby voltage to 0.65v */
	if (val & PCA9450_REG_PWRCTRL_TOFF_DEB)
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x0);
	else
		pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1, 0x4);

	/* I2C_LT_EN*/
	pmic_reg_write(dev, 0xa, 0x3);

	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	timer_init();

	arch_cpu_init();

	spl_early_init();

	preloader_console_init();

	ret = imx9_probe_mu();
	if (ret) {
		printf("Fail to init ELE API\n");
	} else {
		printf("SOC: 0x%x\n", gd->arch.soc_rev);
		printf("LC: 0x%x\n", gd->arch.lifecycle);
	}

	clock_init_late();

	power_init_board();

	if (!IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		set_arm_core_max_clk();

	/* Init power of mix */
	soc_power_init();

	/* Setup TRDC for DDR access */
	trdc_init();

	/* DDR initialization */
	spl_dram_init();

	if (IS_ENABLED(CONFIG_IMX93)) {
		/* Put M33 into CPUWAIT for following kick */
		ret = m33_prepare();
		if (!ret)
			printf("M33 prepare ok\n");
	}

	board_init_r(NULL, 0);
}
