/*
 * arch/arm/mach-tegra/board-ceres-powermon.c
 *
 * Copyright (c) 2013, NVIDIA CORPORATION. All rights reserved.
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

#include <linux/i2c.h>
#include <linux/ina219.h>
#include <linux/platform_data/ina230.h>
#include <linux/i2c/pca954x.h>

#include "board.h"
#include "board-ceres.h"
#include "tegra-board-id.h"

#define PRECISION_MULTIPLIER_CERES	1000

enum {
	UNUSED_RAIL,
};

enum {
	VDD_SYS_CELL,
	VDD_SYS_BUCK1,
	VDD_SOC,
	VDD_SYS_BUCK2,
	VDD_CPU_AP,
	VDD_1V8_BUCK5,
	VDD_SYS_BUCK3,
	VDD_SW_1V2_DSI_CSI_AP,
};

enum {
	VDD_SYS_BL,
	AVDD_DIS_LDO4,
};

/* following rails are present on Ceres-FFD */
enum {
	VDD_CELL,
	VDD_CPU_BUCK2,
	VDD_SOC_BUCK1,
};

/* following rails are present on Atlantis-ERS */
enum {
	ATL_VDD_SYS_CELL,
	ATL_VDD_SYS_SMPS8,
	ATL_VDD_SYS_SMPS6,
	ATL_VDD_SOC,
	ATL_VDD_SYS_SMPS1_2,
	ATL_VDD_CPU,
	ATL_VDD_SW_1V2_DSI_CSI_AP,
	ATL_VDD_1V8_SMPS9,
};

enum {
	ATL_VDD_SYS_BL,
	ATL_AVDD_DIS_LDO1,
};

static struct ina219_platform_data power_mon_info_0[] = {
	/* All unused INA219 devices use below data*/
	[UNUSED_RAIL] = {
		.calibration_data = 0x369C,
		.power_lsb = 3.051979018 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "unused_rail",
		.divisor = 20,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},
};

static struct ina230_platform_data power_mon_info_1[] = {
	[VDD_SYS_CELL] = {
		.calibration_data  = 0x1062,
		.power_lsb = 3.051979018 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_CELL",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_SYS_BUCK1] = {
		.calibration_data  = 0x3E79,
		.power_lsb = 0.800350153 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_BUCK1",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_SOC] = {
		.calibration_data  = 0x7FFF,
		.power_lsb = 3.906369213 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SOC",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_SYS_BUCK2] = {
		.calibration_data  = 0x3A90,
		.power_lsb = 1.707577375 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_BUCK2",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_CPU_AP] = {
		.calibration_data  = 0x6665,
		.power_lsb = 4.883073284 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_CPU_AP",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_1V8_BUCK5] = {
		.calibration_data  = 0x5692,
		.power_lsb = 0.577565202 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_1V8_BUCK5",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_SYS_BUCK3] = {
		.calibration_data  = 0xE90,
		.power_lsb = 0.343347639 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_BUCK3",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_SW_1V2_DSI_CSI_AP] = {
		.calibration_data  = 0x7FFF,
		.power_lsb = 0.019531846 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SW_1V2_DSI_CSI_AP",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},
};

static struct ina230_platform_data power_mon_info_2[] = {
	[VDD_SYS_BL] = {
		.calibration_data  = 0x4188,
		.power_lsb = 0.152598951 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_BL",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[AVDD_DIS_LDO4] = {
		.calibration_data  = 0x48D0,
		.power_lsb = 0.068669528 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "AVDD_DIS_LDO4",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},
};

static struct ina230_platform_data power_mon_info_ffd[] = {
	[VDD_CELL] = {
		.calibration_data  = 0x20C4,
		.power_lsb = 3.051979018 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_CELL",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_CPU_BUCK2] = {
		.calibration_data  = 0x1D48,
		.power_lsb = 1.707577375 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_CPU_BUCK2",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[VDD_SOC_BUCK1] = {
		.calibration_data  = 0x3E79,
		.power_lsb = 0.800350153 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SOC_BUCK1",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},
};

/* following are the power monitor parameters for Atlantis */
static struct ina230_platform_data atl_power_mon_info_1[] = {
	[ATL_VDD_SYS_CELL] = {
		.calibration_data  = 0x1062,
		.power_lsb = 3.051979018 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_CELL",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_SYS_SMPS8] = {
		.calibration_data  = 0xB42,
		.power_lsb = 0.444136017 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_SMPS8",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_SYS_SMPS6] = {
		.calibration_data  = 0x2ED7,
		.power_lsb = 1.067467267 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_SMPS6",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_SOC] = {
		.calibration_data  = 0x7FFF,
		.power_lsb = 3.906369213 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SOC",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_SYS_SMPS1_2] = {
		.calibration_data  = 0x176B,
		.power_lsb = 2.135112594 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_SMPS1_2",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_CPU] = {
		.calibration_data  = 0x51EA,
		.power_lsb = 6.103958035 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_CPU",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_SW_1V2_DSI_CSI_AP] = {
		.calibration_data  = 0x7FFF,
		.power_lsb = 0.019531846 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SW_1V2_DSI_CSI_AP",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_VDD_1V8_SMPS9] = {
		.calibration_data  = 0x6240,
		.power_lsb = 0.508905852 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_1V8_SMPS9",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},
};

static struct ina230_platform_data atl_power_mon_info_2[] = {
	[ATL_VDD_SYS_BL] = {
		.calibration_data  = 0x4188,
		.power_lsb = 0.152598951 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "VDD_SYS_BL",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},

	[ATL_AVDD_DIS_LDO1] = {
		.calibration_data  = 0x48D0,
		.power_lsb = 0.068669528 * PRECISION_MULTIPLIER_CERES,
		.rail_name = "AVDD_DIS_LDO1",
		.divisor = 25,
		.precision_multiplier = PRECISION_MULTIPLIER_CERES,
	},
};

enum {
	INA_I2C_2_0_ADDR_40,
	INA_I2C_2_0_ADDR_41,
	INA_I2C_2_0_ADDR_42,
};

enum {
	INA_I2C_2_1_ADDR_40,
	INA_I2C_2_1_ADDR_41,
	INA_I2C_2_1_ADDR_42,
	INA_I2C_2_1_ADDR_43,
	INA_I2C_2_1_ADDR_44,
	INA_I2C_2_1_ADDR_45,
	INA_I2C_2_1_ADDR_46,
	INA_I2C_2_1_ADDR_47,
};

enum {
	INA_I2C_2_2_ADDR_41,
	INA_I2C_2_2_ADDR_44,
};

/*
 * the I2C device addresses on branch 2 are different between
 * Ceres and Atlantis
 */
enum {
	ATL_INA_I2C_2_2_ADDR_49,
	ATL_INA_I2C_2_2_ADDR_4C,
};

/* i2c addresses of rails present on Ceres-FFD */
enum {
	INA_I2C_2_ADDR_40,
	INA_I2C_2_ADDR_41,
	INA_I2C_2_ADDR_42,
};

static struct i2c_board_info ceres_i2c2_0_ina219_board_info[] = {
	[INA_I2C_2_0_ADDR_40] = {
		I2C_BOARD_INFO("ina219", 0x40),
		.platform_data = &power_mon_info_0[UNUSED_RAIL],
		.irq = -1,
	},

	[INA_I2C_2_0_ADDR_41] = {
		I2C_BOARD_INFO("ina219", 0x41),
		.platform_data = &power_mon_info_0[UNUSED_RAIL],
		.irq = -1,
	},

	[INA_I2C_2_0_ADDR_42] = {
		I2C_BOARD_INFO("ina219", 0x42),
		.platform_data = &power_mon_info_0[UNUSED_RAIL],
		.irq = -1,
	},
};

static struct i2c_board_info ceres_i2c2_1_ina230_board_info[] = {
	[INA_I2C_2_1_ADDR_40] = {
		I2C_BOARD_INFO("ina230", 0x40),
		.platform_data = &power_mon_info_1[VDD_SYS_CELL],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_41] = {
		I2C_BOARD_INFO("ina230", 0x41),
		.platform_data = &power_mon_info_1[VDD_SYS_BUCK3],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_42] = {
		I2C_BOARD_INFO("ina230", 0x42),
		.platform_data = &power_mon_info_1[VDD_SYS_BUCK1],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_43] = {
		I2C_BOARD_INFO("ina230", 0x43),
		.platform_data = &power_mon_info_1[VDD_SOC],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_44] = {
		I2C_BOARD_INFO("ina230", 0x44),
		.platform_data = &power_mon_info_1[VDD_SYS_BUCK2],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_45] = {
		I2C_BOARD_INFO("ina230", 0x45),
		.platform_data = &power_mon_info_1[VDD_CPU_AP],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_46] = {
		I2C_BOARD_INFO("ina230", 0x46),
		.platform_data = &power_mon_info_1[VDD_SW_1V2_DSI_CSI_AP],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_47] = {
		I2C_BOARD_INFO("ina230", 0x47),
		.platform_data = &power_mon_info_1[VDD_1V8_BUCK5],
		.irq = -1,
	},
};

static struct i2c_board_info ceres_i2c2_2_ina230_board_info[] = {
	[INA_I2C_2_2_ADDR_41] = {
		I2C_BOARD_INFO("ina230", 0x41),
		.platform_data = &power_mon_info_2[VDD_SYS_BL],
		.irq = -1,
	},

	[INA_I2C_2_2_ADDR_44] = {
		I2C_BOARD_INFO("ina230", 0x44),
		.platform_data = &power_mon_info_2[AVDD_DIS_LDO4],
		.irq = -1,
	},
};

static struct i2c_board_info ceres_i2c2_ina230_board_info_ffd[] = {
	[INA_I2C_2_ADDR_40] = {
		I2C_BOARD_INFO("ina230", 0x40),
		.platform_data = &power_mon_info_ffd[VDD_CELL],
		.irq = -1,
	},

	[INA_I2C_2_ADDR_41] = {
		I2C_BOARD_INFO("ina230", 0x41),
		.platform_data = &power_mon_info_ffd[VDD_CPU_BUCK2],
		.irq = -1,
	},

	[INA_I2C_2_ADDR_42] = {
		I2C_BOARD_INFO("ina230", 0x42),
		.platform_data = &power_mon_info_ffd[VDD_SOC_BUCK1],
		.irq = -1,
	},
};

/* following are the i2c board info for Atlantis */
static struct i2c_board_info atlantis_i2c2_1_ina230_board_info[] = {
	[INA_I2C_2_1_ADDR_40] = {
		I2C_BOARD_INFO("ina230", 0x40),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_SYS_CELL],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_41] = {
		I2C_BOARD_INFO("ina230", 0x41),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_SYS_SMPS8],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_42] = {
		I2C_BOARD_INFO("ina230", 0x42),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_SYS_SMPS6],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_43] = {
		I2C_BOARD_INFO("ina230", 0x43),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_SOC],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_44] = {
		I2C_BOARD_INFO("ina230", 0x44),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_SYS_SMPS1_2],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_45] = {
		I2C_BOARD_INFO("ina230", 0x45),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_CPU],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_46] = {
		I2C_BOARD_INFO("ina230", 0x46),
		.platform_data =
			&atl_power_mon_info_1[ATL_VDD_SW_1V2_DSI_CSI_AP],
		.irq = -1,
	},

	[INA_I2C_2_1_ADDR_47] = {
		I2C_BOARD_INFO("ina230", 0x47),
		.platform_data = &atl_power_mon_info_1[ATL_VDD_1V8_SMPS9],
		.irq = -1,
	},
};

static struct i2c_board_info atlantis_i2c2_2_ina230_board_info[] = {
	[ATL_INA_I2C_2_2_ADDR_49] = {
		I2C_BOARD_INFO("ina230", 0x49),
		.platform_data = &atl_power_mon_info_2[ATL_VDD_SYS_BL],
		.irq = -1,
	},

	[ATL_INA_I2C_2_2_ADDR_4C] = {
		I2C_BOARD_INFO("ina230", 0x4C),
		.platform_data = &atl_power_mon_info_2[ATL_AVDD_DIS_LDO1],
		.irq = -1,
	},
};

static struct pca954x_platform_mode ceres_pca954x_modes[] = {
	{ .adap_id = PCA954x_I2C_BUS0, .deselect_on_exit = true, },
	{ .adap_id = PCA954x_I2C_BUS1, .deselect_on_exit = true, },
	{ .adap_id = PCA954x_I2C_BUS2, .deselect_on_exit = true, },
	{ .adap_id = PCA954x_I2C_BUS3, .deselect_on_exit = true, },
};

static struct pca954x_platform_data ceres_pca954x_data = {
	.modes    = ceres_pca954x_modes,
	.num_modes      = ARRAY_SIZE(ceres_pca954x_modes),
};

static const struct i2c_board_info ceres_i2c2_board_info[] = {
	{
		I2C_BOARD_INFO("pca9546", 0x71),
		.platform_data = &ceres_pca954x_data,
	},
};

int __init ceres_pmon_init(void)
{
	struct board_info bi;

	tegra_get_board_info(&bi);

	/* There are only 3 devices on Ceres-FFD
	 * so register only those if board is E1690
	 */
	if (bi.board_id == BOARD_E1690) {
		i2c_register_board_info(1, ceres_i2c2_ina230_board_info_ffd,
			ARRAY_SIZE(ceres_i2c2_ina230_board_info_ffd));
	} else {
		i2c_register_board_info(1, ceres_i2c2_board_info,
			ARRAY_SIZE(ceres_i2c2_board_info));

		i2c_register_board_info(PCA954x_I2C_BUS0,
			ceres_i2c2_0_ina219_board_info,
			ARRAY_SIZE(ceres_i2c2_0_ina219_board_info));

		if (bi.board_id == BOARD_E1670 || bi.board_id == BOARD_E1671) {
			i2c_register_board_info(PCA954x_I2C_BUS1,
				atlantis_i2c2_1_ina230_board_info,
				ARRAY_SIZE(atlantis_i2c2_1_ina230_board_info));

			i2c_register_board_info(PCA954x_I2C_BUS2,
				atlantis_i2c2_2_ina230_board_info,
				ARRAY_SIZE(atlantis_i2c2_2_ina230_board_info));
		} else {
			i2c_register_board_info(PCA954x_I2C_BUS1,
				ceres_i2c2_1_ina230_board_info,
				ARRAY_SIZE(ceres_i2c2_1_ina230_board_info));

			i2c_register_board_info(PCA954x_I2C_BUS2,
				ceres_i2c2_2_ina230_board_info,
				ARRAY_SIZE(ceres_i2c2_2_ina230_board_info));
		}
	}

	return 0;
}

