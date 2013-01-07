/*
 * arch/arm/mach-tegra/board-ceres.h
 *
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _MACH_TEGRA_BOARD_CERES_H
#define _MACH_TEGRA_BOARD_CERES_H

#include <mach/gpio-tegra.h>
#include <mach/irqs.h>
#include <linux/mfd/palmas.h>
#include "gpio-names.h"

/* I2C related GPIOs */
#define TEGRA_GPIO_I2C1_SCL		TEGRA_GPIO_PC4
#define TEGRA_GPIO_I2C1_SDA		TEGRA_GPIO_PC5
#define TEGRA_GPIO_I2C2_SCL		TEGRA_GPIO_PT5
#define TEGRA_GPIO_I2C2_SDA		TEGRA_GPIO_PT6
#define TEGRA_GPIO_I2C3_SCL		TEGRA_GPIO_PBB1
#define TEGRA_GPIO_I2C3_SDA		TEGRA_GPIO_PBB2
#define TEGRA_GPIO_I2C4_SCL		TEGRA_GPIO_PV4
#define TEGRA_GPIO_I2C4_SDA		TEGRA_GPIO_PV5
#define TEGRA_GPIO_I2C5_SCL		TEGRA_GPIO_PZ6
#define TEGRA_GPIO_I2C5_SDA		TEGRA_GPIO_PZ7

/* Camera related GPIOs */
#define CAM_RSTN			TEGRA_GPIO_PBB3
#define CAM_FLASH_STROBE		TEGRA_GPIO_PBB4
#define CAM1_POWER_DWN_GPIO		TEGRA_GPIO_PBB6
#define CAM2_POWER_DWN_GPIO		TEGRA_GPIO_PBB5
#define CAM_AF_PWDN			TEGRA_GPIO_PBB7
#define CAM_TORCH_EN			TEGRA_GPIO_PCC1

int ceres_sensors_init(void);
int ceres_keys_init(void);
int ceres_sdhci_init(void);
int ceres_regulator_init(void);
int ceres_suspend_init(void);
int ceres_pinmux_init(void);
int ceres_panel_init(void);
int ceres_soctherm_init(void);
#endif
