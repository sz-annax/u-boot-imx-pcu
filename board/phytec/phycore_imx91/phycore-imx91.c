// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <env.h>

int board_init(void)
{
	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
	switch (get_boot_device()) {
	case SD2_BOOT:
		env_set_ulong("mmcdev", 1);
		if (!env_get("boot_targets"))
			env_set("boot_targets", "mmc1 mmc0 ethernet");
		break;
	case MMC1_BOOT:
		env_set_ulong("mmcdev", 0);
		break;
	default:
		break;
	}

	return 0;
}