// SPDX-License-Identifier: GPL-2.0-or-later

#include <asm/arch/sys_proto.h>

#include "imx91_93_som_detection.h"

extern struct phytec_eeprom_data eeprom_data;

u8 phytec_imx91_93_detect(struct phytec_eeprom_data *data)
{
	if (!data)
		data = &eeprom_data;

	if (data->api_rev < PHYTEC_API_REV2)
		return 1;

	if (data->data.data_api2.som_no == PHYTEC_IMX91_93_SOM &&
	    (is_imx91() || is_imx93()))
		return 0;

	pr_err("SoM ID does not match phyCORE-i.MX91/93\n");
	return 1;
}

u8 phytec_imx91_93_get_opt(struct phytec_eeprom_data *data,
			   enum phytec_imx91_93_option_index index)
{
	char *options;

	if (!data)
		data = &eeprom_data;

	if (data->api_rev < PHYTEC_API_REV2)
		return PHYTEC_EEPROM_INVAL;

	options = phytec_get_opt(data);
	if (!options || index >= PHYTEC_MAX_OPTIONS)
		return PHYTEC_EEPROM_INVAL;

	return PHYTEC_GET_OPTION(options[index]);
}