/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _PHYTEC_IMX91_93_SOM_DETECTION_H
#define _PHYTEC_IMX91_93_SOM_DETECTION_H

#include "phytec_som_detection.h"

#define PHYTEC_IMX91_93_SOM	77

enum phytec_imx91_93_option_index {
	PHYTEC_IMX91_93_OPT_DDR,
	PHYTEC_IMX91_93_OPT_EMMC,
	PHYTEC_IMX91_93_OPT_CPU,
	PHYTEC_IMX91_93_OPT_FREQ,
	PHYTEC_IMX91_93_OPT_NPU,
	PHYTEC_IMX91_93_OPT_DISP,
	PHYTEC_IMX91_93_OPT_ETH,
	PHYTEC_IMX91_93_OPT_FEAT,
};

enum phytec_imx91_93_ddr_eeprom_code {
	PHYTEC_IMX91_93_DDR_INVALID = PHYTEC_EEPROM_INVAL,
	PHYTEC_IMX91_93_LPDDR4X_512MB = 0,
	PHYTEC_IMX91_93_LPDDR4X_1GB = 1,
	PHYTEC_IMX91_93_LPDDR4X_2GB = 2,
	PHYTEC_IMX91_93_LPDDR4_512MB = 3,
	PHYTEC_IMX91_93_LPDDR4_1GB = 4,
	PHYTEC_IMX91_93_LPDDR4_2GB = 5,
};

u8 phytec_imx91_93_detect(struct phytec_eeprom_data *data);
u8 phytec_imx91_93_get_opt(struct phytec_eeprom_data *data,
			   enum phytec_imx91_93_option_index index);

#endif