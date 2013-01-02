/*
 * arch/arm/mach-tegra/board-e1853-sdhci.c
 *
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>
#include <mach/board_id.h>
#include <linux/i2c.h>

#include "gpio-names.h"
#include "board.h"
#include "board-e1853.h"
#include "devices.h"

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data1 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.ddr_clk_limit = 30000000,
	.is_8bit = false,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data2 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.is_8bit = 1,
	.ddr_clk_limit = 30000000,
	.mmc_data = {
		.built_in = 1,
	}
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data3 = {
	.cd_gpio = TEGRA_GPIO_PN6,
	.wp_gpio = TEGRA_GPIO_PD4,
	.power_gpio = TEGRA_GPIO_PN7,
	.is_8bit = false,
	.ddr_clk_limit = 30000000,
	.mmc_data = {
		.ocr_mask = MMC_OCR_2V8_MASK,
	}
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data4 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.is_8bit = true,
	.ddr_clk_limit = 51000000,
};

int __init e1853_sdhci_init(void)
{
	int is_e1860 = 0;
	tegra_sdhci_device1.dev.platform_data = &tegra_sdhci_platform_data1;
	tegra_sdhci_device2.dev.platform_data = &tegra_sdhci_platform_data2;
	tegra_sdhci_device3.dev.platform_data = &tegra_sdhci_platform_data3;
	tegra_sdhci_device4.dev.platform_data = &tegra_sdhci_platform_data4;

	is_e1860 = tegra_is_board(NULL, "61860", NULL, NULL, NULL);
	if (is_e1860){
		tegra_sdhci_platform_data3.mmc_data.ocr_mask = MMC_OCR_3V2_MASK;
	}

	platform_device_register(&tegra_sdhci_device1);
	platform_device_register(&tegra_sdhci_device2);
	platform_device_register(&tegra_sdhci_device3);
	platform_device_register(&tegra_sdhci_device4);

	return 0;
}
