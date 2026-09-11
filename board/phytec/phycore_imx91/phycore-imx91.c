// SPDX-License-Identifier: (GPL-2.0+ OR MIT)

#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <env.h>
#include <net.h>

static void migrate_fec_mac_address(void)
{
	u8 enetaddr[ARP_HLEN];
	int ret;

	if (env_get("ethaddr") ||
	    !eth_env_get_enetaddr("eth1addr", enetaddr))
		return;

	ret = eth_env_set_enetaddr("ethaddr", enetaddr);
	if (ret) {
		printf("Failed to set ethaddr from eth1addr: %d\n", ret);
		return;
	}

	ret = env_save();
	if (ret)
		printf("Failed to save migrated ethaddr: %d\n", ret);
}

int board_init(void)
{
	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int mmc_map_to_kernel_blk(int devno)
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

	migrate_fec_mac_address();

	return 0;
}