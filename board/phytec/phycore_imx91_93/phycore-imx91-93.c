// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (C) 2023 PHYTEC Messtechnik GmbH
 * Author: Christoph Stoidner <c.stoidner@phytec.de>
 * Copyright (C) 2024 Mathieu Othacehe <m.othacehe@gmail.com>
 */

#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <env.h>

#include "../common/imx91_93_som_detection.h"

#define EEPROM_ADDR	0x50

int board_init(void)
{
	int ret;

	ret = phytec_eeprom_data_setup(NULL, CONFIG_PHYTEC_EEPROM_BUS,
				      EEPROM_ADDR);
	if (ret)
		printf("EEPROM data init failed\n");
	else if (!phytec_imx91_93_detect(NULL))
		phytec_print_som_info(NULL);

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
