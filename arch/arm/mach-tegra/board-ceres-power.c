/*
 * arch/arm/mach-tegra/board-ceres-power.c
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
 */

#include <linux/i2c.h>
#include <linux/pda_power.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/io.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/max77660/max77660-core.h>
#include <linux/mfd/max77660/max77660-regulator.h>

#include <asm/mach-types.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/gpio-tegra.h>

#include "pm.h"
#include "board.h"
#include "board-ceres.h"
#include "tegra11_soctherm.h"

#define PMC_CTRL                0x0
#define PMC_CTRL_INTR_LOW       (1 << 17)
#define MAX77660_IRQ_BASE	TEGRA_NR_IRQS

/* max77660 consumer rails */
static struct regulator_consumer_supply max77660_buck1_supply[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
	REGULATOR_SUPPLY("vdd_core_dbg", NULL),
};

static struct regulator_consumer_supply max77660_buck2_supply[] = {
	REGULATOR_SUPPLY("vdd_cpu", NULL),
	REGULATOR_SUPPLY("vdd_cpu_dbg", NULL),
};

static struct regulator_consumer_supply max77660_buck3_supply[] = {
	REGULATOR_SUPPLY("vdd2_lpddr3", NULL),
	REGULATOR_SUPPLY("vddca_lpddr3", NULL),
	REGULATOR_SUPPLY("vrefca_lpddr3", NULL),
	REGULATOR_SUPPLY("vddio_hsic", "tegra-ehci.1"),
	REGULATOR_SUPPLY("vddio_hsic", "tegra-ehci.2"),
	REGULATOR_SUPPLY("vddio_hsic_modem2", NULL),
};

static struct regulator_consumer_supply max77660_buck4_supply[] = {
	REGULATOR_SUPPLY("vdd_bb", NULL),
};

static struct regulator_consumer_supply max77660_buck5_supply[] = {
	REGULATOR_SUPPLY("avdd_osc", NULL),
	REGULATOR_SUPPLY("vddio_sys", NULL),
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.3"),
	REGULATOR_SUPPLY("avdd_usb_pll", "tegra-udc.0"),
	REGULATOR_SUPPLY("avdd_usb_pll", "tegra-ehci.0"),
	REGULATOR_SUPPLY("avdd_usb_pll", "tegra-ehci.1"),
	REGULATOR_SUPPLY("vddio_cam", "tegra_camera"),
	REGULATOR_SUPPLY("vddio_audio", NULL),
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.0"),
	REGULATOR_SUPPLY("vddio_uart", NULL),
	REGULATOR_SUPPLY("vddio_bb", NULL),
	REGULATOR_SUPPLY("vdd_1v8_pmic", NULL),
	REGULATOR_SUPPLY("vdd_1v8_cpu_reg", NULL),
	REGULATOR_SUPPLY("vdd_sys_mb", NULL),
	REGULATOR_SUPPLY("vdd_1v8_eeprom", NULL),
	REGULATOR_SUPPLY("vdd_1v8_hdmi", "tegradc.1"),
	REGULATOR_SUPPLY("vdd_1v8_com", NULL),
	REGULATOR_SUPPLY("vddio_sim_bb", NULL),
	REGULATOR_SUPPLY("vdd_nfc", NULL),
	REGULATOR_SUPPLY("vdd_dtv", NULL),
	REGULATOR_SUPPLY("vdd_ts", NULL),
	REGULATOR_SUPPLY("vdd_modem2", NULL),
	REGULATOR_SUPPLY("vdd_dbg", NULL),
};

static struct regulator_consumer_supply max77660_buck6_supply[] = {
	REGULATOR_SUPPLY("vdd_1v7_rf", NULL),
};

static struct regulator_consumer_supply max77660_buck7_supply[] = {
	REGULATOR_SUPPLY("vdd_2v65_rf", NULL),
};

static struct regulator_consumer_supply max77660_ldo1_supply[] = {
	REGULATOR_SUPPLY("vdd_rtc", NULL),
};

static struct regulator_consumer_supply max77660_ldo2_supply[] = {
	REGULATOR_SUPPLY("avdd_2v8_cam_af", NULL),
	REGULATOR_SUPPLY("avdd_cam2", NULL),
	REGULATOR_SUPPLY("vana", "2-0036"),
	REGULATOR_SUPPLY("vdd", "2-000e"),
};

static struct regulator_consumer_supply max77660_ldo3_supply[] = {
	REGULATOR_SUPPLY("avdd_cam1", NULL),
	REGULATOR_SUPPLY("vana", "2-0010"),
};

static struct regulator_consumer_supply max77660_ldo4_supply[] = {
	 REGULATOR_SUPPLY("avdd_dis_lcd", NULL),
	 REGULATOR_SUPPLY("avdd_dis_ts", NULL),
	 REGULATOR_SUPPLY("vin", "1-004d"),
};

static struct regulator_consumer_supply max77660_ldo5_supply[] = {
	REGULATOR_SUPPLY("avdd_aud", NULL),
};

static struct regulator_consumer_supply max77660_ldo6_supply[] = {
	REGULATOR_SUPPLY("vdd_cam_1v2", NULL),
	REGULATOR_SUPPLY("vdig", "2-0010"),
	REGULATOR_SUPPLY("vdig", "2-0036"),
};

static struct regulator_consumer_supply max77660_ldo7_supply[] = {
	REGULATOR_SUPPLY("avdd_plla_m_p", NULL),
	REGULATOR_SUPPLY("avdd_pllc", NULL),
	REGULATOR_SUPPLY("avdd_ddr_hs", NULL),
	REGULATOR_SUPPLY("avdd_hdmi_pll", "tegradc.1"),
	REGULATOR_SUPPLY("avdd_mipi_pllu", NULL),

};

static struct regulator_consumer_supply max77660_ldo8_supply[] = {
	REGULATOR_SUPPLY("dvdd_bb_pll", NULL),
	REGULATOR_SUPPLY("avdd_bb_pll", NULL),
};

static struct regulator_consumer_supply max77660_ldo9_supply[] = {
	REGULATOR_SUPPLY("vdd_mb", NULL),
	REGULATOR_SUPPLY("vdd_temp", NULL),
	REGULATOR_SUPPLY("vdd_nfc_2v8", NULL),
	REGULATOR_SUPPLY("vdd_irled", NULL),
	REGULATOR_SUPPLY("vdd_sensor_2v8", NULL),
	REGULATOR_SUPPLY("vdd_pm_2v8", NULL),
	REGULATOR_SUPPLY("vdd", "0-0069"),
};

static struct regulator_consumer_supply max77660_ldo10_supply[] = {
	REGULATOR_SUPPLY("vpp_fuse", NULL),
	REGULATOR_SUPPLY("vpp_bb_fuse", NULL),
};

static struct regulator_consumer_supply max77660_ldo11_supply[] = {
	REGULATOR_SUPPLY("avdd_usb", "tegra-udc.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.1"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.2"),
	REGULATOR_SUPPLY("avdd_hdmi", "tegradc.1"),
	REGULATOR_SUPPLY("vdd_sys_dtv_3v3", NULL),
};

static struct regulator_consumer_supply max77660_ldo12_supply[] = {
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.2"),
};

static struct regulator_consumer_supply max77660_ldo13_supply[] = {
	REGULATOR_SUPPLY("vddio_sd_slot", "sdhci-tegra.2"),
};

static struct regulator_consumer_supply max77660_ldo14_supply[] = {
	REGULATOR_SUPPLY("vddio_sd_slot", "sdhci-tegra.3"),
};

static struct regulator_consumer_supply max77660_ldo15_supply[] = {
	REGULATOR_SUPPLY("vddio_sim0", NULL),
};

static struct regulator_consumer_supply max77660_ldo16_supply[] = {
	REGULATOR_SUPPLY("vddio_sim1", NULL),
};

static struct regulator_consumer_supply max77660_ldo17_supply[] = {
	REGULATOR_SUPPLY("avdd_1v8_rf", NULL),
};

static struct regulator_consumer_supply max77660_ldo18_supply[] = {
	REGULATOR_SUPPLY("avdd_2v7_rf", NULL),
};

static struct regulator_consumer_supply max77660_sw1_supply[] = {
	 REGULATOR_SUPPLY("vdd_dis_lcd", NULL),
	 REGULATOR_SUPPLY("vdd_dis_ts", NULL),
	 REGULATOR_SUPPLY("vdd_lcd_1v8_s", NULL),
};

static struct regulator_consumer_supply max77660_sw2_supply[] = {
	REGULATOR_SUPPLY("vdd_cam_1v8", NULL),
	REGULATOR_SUPPLY("vif", "2-0010"),
	REGULATOR_SUPPLY("vif", "2-0036"),
	REGULATOR_SUPPLY("vdd_i2c", "2-000e"),
};

static struct regulator_consumer_supply max77660_sw3_supply[] = {
	REGULATOR_SUPPLY("vdd_aud_dgtl", NULL),
	REGULATOR_SUPPLY("vdd_aud_anlg", NULL),
	REGULATOR_SUPPLY("vdd_aud_mic", NULL),
	REGULATOR_SUPPLY("vdd", "0-0044"),
	REGULATOR_SUPPLY("vlogic", "0-0069"),
};

static struct regulator_consumer_supply max77660_sw4_supply[] = {
	REGULATOR_SUPPLY("vdd_rtc_alt1", NULL),
};

static struct regulator_consumer_supply max77660_sw5_supply[] = {
	REGULATOR_SUPPLY("vddio_ddr", NULL),
	REGULATOR_SUPPLY("vddq_lpddr3", NULL),
	REGULATOR_SUPPLY("vrefdq_lpddr3", NULL),
	REGULATOR_SUPPLY("avdd_dsi_csi", "tegradc.0"),
	REGULATOR_SUPPLY("avdd_dsi_csi", "tegradc.1"),
	REGULATOR_SUPPLY("avdd_dsi_csi", "tegra_camera"),
	REGULATOR_SUPPLY("vdd_1v2_lcd", NULL),
	REGULATOR_SUPPLY("vdd_1v2_cdc", NULL),
};

static struct max77660_regulator_fps_cfg max77660_fps_cfgs[] = {
};

#define MAX77660_PDATA_INIT(_rid, _id, _min_uV, _max_uV, _supply_reg,	\
		_always_on, _boot_on, _apply_uV,			\
		_fps_src, _fps_pu_period, _fps_pd_period, _flags)	\
	static struct regulator_init_data max77660_regulator_idata_##_id = {   \
		.supply_regulator = _supply_reg,			\
		.constraints = {					\
			.name = max77660_rails(_id),			\
			.min_uV = _min_uV*1000,				\
			.max_uV = _max_uV*1000,				\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |	\
					     REGULATOR_MODE_STANDBY),	\
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |	\
					   REGULATOR_CHANGE_STATUS |	\
					   REGULATOR_CHANGE_VOLTAGE),	\
			.always_on = _always_on,			\
			.boot_on = _boot_on,				\
			.apply_uV = _apply_uV,				\
		},							\
		.num_consumer_supplies =				\
			ARRAY_SIZE(max77660_##_id##_supply),		\
		.consumer_supplies = max77660_##_id##_supply,		\
	};								\
static struct max77660_regulator_platform_data max77660_regulator_pdata_##_id =\
{									\
		.reg_init_data = &max77660_regulator_idata_##_id,	\
		.id = MAX77660_REGULATOR_ID_##_rid,			\
		.fps_src = _fps_src,					\
		.fps_pu_period = _fps_pu_period,			\
		.fps_pd_period = _fps_pd_period,			\
		.num_fps_cfgs = ARRAY_SIZE(max77660_fps_cfgs),		\
		.fps_cfgs = max77660_fps_cfgs,				\
		.flags = _flags,					\
	}

MAX77660_PDATA_INIT(BUCK1, buck1,  900, 1400, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK2, buck2,  900, 1300, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK3, buck3,  1200, 1200, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK4, buck4,  1000, 1000, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK5, buck5,  1800, 1800, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK6, buck6,  1700, 1700, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK7, buck7,  2650, 2650, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO1, ldo1, 1100, 1100, max77660_rails(buck3),
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO2, ldo2, 2800, 2800, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO3, ldo3, 2800, 2800, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO4, ldo4, 3000, 3000, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO5, ldo5, 1800, 1800, NULL,
		0, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO6, ldo6, 1200, 1200, max77660_rails(buck5),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO7, ldo7, 1050, 1050, max77660_rails(buck3),
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO8, ldo8, 900, 900, max77660_rails(buck3),
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO9, ldo9, 2800, 2800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO10, ldo10, 1800, 1800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO11, ldo11, 3300, 3300, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO12, ldo12, 1800, 3300, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO13, ldo13, 2850, 2850, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO14, ldo14, 2800, 2800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO15, ldo15, 1200, 1200, NULL,
		0, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO16, ldo16, 3000, 3000, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO17, ldo17, 1800, 1800, max77660_rails(buck7),
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO18, ldo18, 2700, 2700, NULL,
		0, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW1, sw1, 1800, 1800, max77660_rails(buck5),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW2, sw2, 1800, 1800, max77660_rails(buck5),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW3, sw3, 1800, 1800, max77660_rails(buck5),
		0, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW4, sw4, 1100, 1100, max77660_rails(buck1),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW5, sw5, 1200, 1200, max77660_rails(buck3),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

#define MAX77660_REG(_id, _data) (&max77660_regulator_pdata_##_data)

static struct max77660_regulator_platform_data *max77660_reg_pdata[] = {
	MAX77660_REG(BUCK1, buck1),
	MAX77660_REG(BUCK2, buck2),
	MAX77660_REG(BUCK3, buck3),
	MAX77660_REG(BUCK4, buck4),
	MAX77660_REG(BUCK5, buck5),
	MAX77660_REG(BUCK6, buck6),
	MAX77660_REG(BUCK7, buck7),
	MAX77660_REG(LDO1, ldo1),
	MAX77660_REG(LDO2, ldo2),
	MAX77660_REG(LDO3, ldo3),
	MAX77660_REG(LDO4, ldo4),
	MAX77660_REG(LDO5, ldo5),
	MAX77660_REG(LDO6, ldo6),
	MAX77660_REG(LDO7, ldo7),
	MAX77660_REG(LDO8, ldo8),
	MAX77660_REG(LDO9, ldo9),
	MAX77660_REG(LDO10, ldo10),
	MAX77660_REG(LDO11, ldo11),
	MAX77660_REG(LDO12, ldo12),
	MAX77660_REG(LDO13, ldo13),
	MAX77660_REG(LDO14, ldo14),
	MAX77660_REG(LDO15, ldo15),
	MAX77660_REG(LDO16, ldo16),
	MAX77660_REG(LDO17, ldo17),
	MAX77660_REG(LDO18, ldo18),
	MAX77660_REG(SW1, sw1),
	MAX77660_REG(SW2, sw2),
	MAX77660_REG(SW3, sw3),
	MAX77660_REG(SW4, sw4),
	MAX77660_REG(SW5, sw5),
};

static struct max77660_gpio_config max77660_gpio_cfgs[] = {
	{
		.gpio = MAX77660_GPIO0,
		.dir = GPIO_DIR_OUT,
		.dout = GPIO_DOUT_LOW,
		.out_drv = GPIO_OUT_DRV_PUSH_PULL,
		.alternate = GPIO_ALT_DISABLE,
	},
	{
		.gpio = MAX77660_GPIO1,
		.dir = GPIO_DIR_IN,
		.dout = GPIO_DOUT_HIGH,
		.out_drv = GPIO_OUT_DRV_OPEN_DRAIN,
		.pull_up = GPIO_PU_ENABLE,
		.alternate = GPIO_ALT_DISABLE,
	},
	{
		.gpio = MAX77660_GPIO2,
		.dir = GPIO_DIR_IN,
		.dout = GPIO_DOUT_HIGH,
		.out_drv = GPIO_OUT_DRV_OPEN_DRAIN,
		.pull_up = GPIO_PU_ENABLE,
		.alternate = GPIO_ALT_DISABLE,
	},
	{
		.gpio = MAX77660_GPIO3,
		.dir = GPIO_DIR_OUT,
		.dout = GPIO_DOUT_HIGH,
		.out_drv = GPIO_OUT_DRV_OPEN_DRAIN,
		.pull_up = GPIO_PU_ENABLE,
		.alternate = GPIO_ALT_DISABLE,
	},
	{
		.gpio = MAX77660_GPIO4,
		.dir = GPIO_DIR_OUT,
		.dout = GPIO_DOUT_HIGH,
		.out_drv = GPIO_OUT_DRV_PUSH_PULL,
		.alternate = GPIO_ALT_ENABLE,
	},
	{
		.gpio = MAX77660_GPIO5,
		.dir = GPIO_DIR_OUT,
		.dout = GPIO_DOUT_LOW,
		.out_drv = GPIO_OUT_DRV_PUSH_PULL,
		.alternate = GPIO_ALT_DISABLE,
	},
	{
		.gpio = MAX77660_GPIO6,
		.dir = GPIO_DIR_OUT,
		.dout = GPIO_DOUT_LOW,
		.out_drv = GPIO_OUT_DRV_PUSH_PULL,
		.alternate = GPIO_ALT_DISABLE,
	},
	{
		.gpio = MAX77660_GPIO7,
		.dir = GPIO_DIR_OUT,
		.dout = GPIO_DOUT_LOW,
		.out_drv = GPIO_OUT_DRV_OPEN_DRAIN,
		.alternate = GPIO_ALT_ENABLE,
	},
	{
		.gpio = MAX77660_GPIO8,
		.dir = GPIO_DIR_IN,
		.dout = GPIO_DOUT_HIGH,
		.out_drv = GPIO_OUT_DRV_OPEN_DRAIN,
		.pull_up = GPIO_PU_ENABLE,
		.alternate = GPIO_ALT_ENABLE,
	},
	{
		.gpio = MAX77660_GPIO9,
		.dir = GPIO_DIR_IN,
		.dout = GPIO_DOUT_HIGH,
		.out_drv = GPIO_OUT_DRV_OPEN_DRAIN,
		.pull_up = GPIO_PU_ENABLE,
		.alternate = GPIO_ALT_DISABLE,
	},
};

static struct max77660_platform_data max77660_pdata = {
	.irq_base	= MAX77660_IRQ_BASE,
	.gpio_base	= MAX77660_GPIO_BASE,

	.num_gpio_cfgs	= ARRAY_SIZE(max77660_gpio_cfgs),
	.gpio_cfgs	= max77660_gpio_cfgs,

	.regulator_pdata = max77660_reg_pdata,
	.num_regulator_pdata = ARRAY_SIZE(max77660_reg_pdata),

	.flags	= 0x00,
	.use_power_off	= true,
};

static struct i2c_board_info __initdata max77660_regulators[] = {
	{
		/* The I2C address was determined by OTP factory setting */
		I2C_BOARD_INFO("max77660", MAX77660_PWR_I2C_ADDR),
		.irq		= INT_EXTERNAL_PMU,
		.platform_data	= &max77660_pdata,
	},
};

int __init ceres_regulator_init(void)
{
	void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	u32 pmc_ctrl;

	/* configure the power management controller to trigger PMU
	 * interrupts when low */
	pmc_ctrl = readl(pmc + PMC_CTRL);
	writel(pmc_ctrl | PMC_CTRL_INTR_LOW, pmc + PMC_CTRL);

	i2c_register_board_info(4, max77660_regulators,
			ARRAY_SIZE(max77660_regulators));

	return 0;
}

/* Always ON /Battery regulator */
static struct regulator_consumer_supply fixed_reg_battery_supply[] = {
	REGULATOR_SUPPLY("vdd_sys_bl", NULL),
};

/* LCD_AVDD_EN From PMU GP6 */
static struct regulator_consumer_supply fixed_reg_avdd_lcd_supply[] = {
	REGULATOR_SUPPLY("avdd_lcd", NULL),
};

/* Macro for defining fixed regulator sub device data */
#define FIXED_SUPPLY(_name) "fixed_reg_"#_name
#define FIXED_REG(_id, _var, _name, _in_supply, _always_on, _boot_on,	\
	_gpio_nr, _open_drain, _active_high, _boot_state, _millivolts)	\
	static struct regulator_init_data ri_data_##_var =		\
	{								\
		.supply_regulator = _in_supply,				\
		.num_consumer_supplies =				\
			ARRAY_SIZE(fixed_reg_##_name##_supply),		\
		.consumer_supplies = fixed_reg_##_name##_supply,	\
		.constraints = {					\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |	\
					REGULATOR_MODE_STANDBY),	\
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |	\
					REGULATOR_CHANGE_STATUS |	\
					REGULATOR_CHANGE_VOLTAGE),	\
			.always_on = _always_on,			\
			.boot_on = _boot_on,				\
		},							\
	};								\
	static struct fixed_voltage_config fixed_reg_##_var##_pdata =	\
	{								\
		.supply_name = FIXED_SUPPLY(_name),			\
		.microvolts = _millivolts * 1000,			\
		.gpio = _gpio_nr,					\
		.gpio_is_open_drain = _open_drain,			\
		.enable_high = _active_high,				\
		.enabled_at_boot = _boot_state,				\
		.init_data = &ri_data_##_var,				\
	};								\
	static struct platform_device fixed_reg_##_var##_dev = {	\
		.name = "reg-fixed-voltage",				\
		.id = _id,						\
		.dev = {						\
			.platform_data = &fixed_reg_##_var##_pdata,	\
		},							\
	}

FIXED_REG(0,	battery,	battery,
	NULL,	0,	0,
	-1,	false, true,	0,	3300);

FIXED_REG(1,	avdd_lcd,	avdd_lcd,
	NULL,	0,	0,
	MAX77660_GPIO_BASE + MAX77660_GPIO6,	true,	true,	1,	2800);

/*
 * Creating the fixed regulator device tables
 */

#define ADD_FIXED_REG(_name)    (&fixed_reg_##_name##_dev)

#define CERES_COMMON_FIXED_REG		\
	ADD_FIXED_REG(battery),		\
	ADD_FIXED_REG(avdd_lcd),		\

/* Gpio switch regulator platform data for Ceres E1680 */
static struct platform_device *fixed_reg_devs_e1680[] = {
	CERES_COMMON_FIXED_REG
};

static int __init ceres_fixed_regulator_init(void)
{
	if (!of_machine_is_compatible("nvidia,ceres"))
		return 0;

	return platform_add_devices(fixed_reg_devs_e1680,
				ARRAY_SIZE(fixed_reg_devs_e1680));
}
subsys_initcall_sync(ceres_fixed_regulator_init);

static struct tegra_suspend_platform_data ceres_suspend_data = {
	.cpu_timer	= 300,
	.cpu_off_timer	= 300,
	.suspend_mode	= TEGRA_SUSPEND_LP0,
	.core_timer	= 0x157e,
	.core_off_timer = 2000,
	.corereq_high	= true,
	.sysclkreq_high	= true,
};


int __init ceres_suspend_init(void)
{
	tegra_init_suspend(&ceres_suspend_data);
	return 0;
}

static struct soctherm_platform_data ceres_soctherm_data = {
	.soctherm_clk_rate = 136000000,
	.tsensor_clk_rate = 500000,
	.sensor_data = {
		[TSENSE_CPU0] = {
			.sensor_enable = true,
			.zone_enable = false,
			.tall = 16300,
			.tiddq = 1,
			.ten_count = 1,
			.tsample = 163,
			.pdiv = 10,
		},
		[TSENSE_CPU1] = {
			.sensor_enable = true,
			.zone_enable = false,
			.tall = 16300,
			.tiddq = 1,
			.ten_count = 1,
			.tsample = 163,
			.pdiv = 10,
		},
		[TSENSE_CPU2] = {
			.sensor_enable = true,
			.zone_enable = false,
			.tall = 16300,
			.tiddq = 1,
			.ten_count = 1,
			.tsample = 163,
			.pdiv = 10,
		},
		[TSENSE_CPU3] = {
			.sensor_enable = true,
			.zone_enable = false,
			.tall = 16300,
			.tiddq = 1,
			.ten_count = 1,
			.tsample = 163,
			.pdiv = 10,
		},
		[TSENSE_GPU] = {
			.sensor_enable = true,
			.zone_enable = false,
			.tall = 16300,
			.tiddq = 1,
			.ten_count = 1,
			.tsample = 163,
			.pdiv = 10,
		},
		[TSENSE_PLLX] = {
			.sensor_enable = true,
			.zone_enable = false,
			.tall = 16300,
			.tiddq = 1,
			.ten_count = 1,
			.tsample = 163,
			.pdiv = 10,
		},
	},
	.therm = {
		[THERM_CPU] = {
			.zone_enable = true,
			.cdev_type = "tegra-balanced",
			.thermtrip = 115,
			.trip_temp = 85000,
			.passive_delay = 1000,
			.hysteresis = 3000,
		},
		[THERM_GPU] = {
			.zone_enable = true,
		},
		[THERM_PLL] = {
			.zone_enable = true,
		},
	},
};

int __init ceres_soctherm_init(void)
{
	return tegra11_soctherm_init(&ceres_soctherm_data);
}
