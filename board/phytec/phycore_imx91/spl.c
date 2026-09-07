// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

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
#include <linux/errno.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <spl.h>

DECLARE_GLOBAL_DATA_PTR;

#define PCA9450_REG_PWRCTRL_TOFF_DEB	BIT(5)

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
	ddr_init(&dram_timing);
}

int power_init_board(void)
{
	struct udevice *dev;
	unsigned int buck_val;
	int pwr_ctrl;
	int ret;

	ret = pmic_get("pmic@25", &dev);
	if (ret) {
		printf("ERROR: Get PMIC PCA9451A failed: %d\n", ret);
		return ret;
	}

	ret = pmic_reg_write(dev, PCA9450_BUCK123_DVS, 0x29);
	if (ret)
		return ret;

	ret = pmic_reg_write(dev, PCA9450_BUCK1CTRL, 0x59);
	if (ret)
		return ret;

	pwr_ctrl = pmic_reg_read(dev, PCA9450_PWR_CTRL);
	if (pwr_ctrl < 0)
		return pwr_ctrl;

	if (IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		buck_val = pwr_ctrl & PCA9450_REG_PWRCTRL_TOFF_DEB ? 0x0c : 0x10;
	else
		buck_val = pwr_ctrl & PCA9450_REG_PWRCTRL_TOFF_DEB ? 0x14 : 0x18;

	ret = pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS0, buck_val);
	if (ret)
		return ret;

	ret = pmic_reg_write(dev, PCA9450_BUCK3OUT_DVS0, buck_val);
	if (ret)
		return ret;

	ret = pmic_reg_write(dev, PCA9450_BUCK2OUT_DVS0, 0x28);
	if (ret)
		return ret;

	ret = pmic_reg_read(dev, PCA9450_BUCK2OUT_DVS0);
	if (ret < 0)
		return ret;
	if ((ret & PCA9450_DVS_BUCK_RUN_MASK) != 0x28) {
		printf("ERROR: PMIC BUCK2 readback is %#x, expected 0x28\n", ret);
		return -EIO;
	}
	puts("PMIC: DDR VDDQ set to 1.1V\n");

	ret = pmic_reg_write(dev, PCA9450_BUCK1OUT_DVS1,
			     pwr_ctrl & PCA9450_REG_PWRCTRL_TOFF_DEB ? 0x0 : 0x4);
	if (ret)
		return ret;

	return pmic_reg_write(dev, 0xa, 0x3);
}

void board_init_f(ulong dummy)
{
	int ret;

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

	ret = power_init_board();
	if (ret) {
		printf("ERROR: PMIC initialization failed: %d\n", ret);
		hang();
	}

	if (!IS_ENABLED(CONFIG_IMX9_LOW_DRIVE_MODE))
		set_arm_core_max_clk();

	soc_power_init();
	trdc_init();
	spl_dram_init();
	board_init_r(NULL, 0);
}