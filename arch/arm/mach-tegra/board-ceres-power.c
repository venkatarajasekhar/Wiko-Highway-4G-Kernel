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
#include <linux/gpio.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/max77660/max77660-core.h>
#include <linux/platform_data/lp8755.h>

#include <asm/mach-types.h>

#include <mach/iomap.h>
#include <mach/edp.h>
#include <mach/irqs.h>
#include <mach/gpio-tegra.h>

#include "cpu-tegra.h"
#include "pm.h"
#include "board.h"
#include "board-common.h"
#include "board-ceres.h"
#include "tegra11_soctherm.h"
#include "tegra-board-id.h"
#include "tegra_cl_dvfs.h"
#include "devices.h"

#define PMC_CTRL                0x0
#define PMC_CTRL_INTR_LOW       (1 << 17)
#define MAX77660_IRQ_BASE	TEGRA_NR_IRQS

/* max77660 consumer rails */
static struct regulator_consumer_supply max77660_unused_supply[] = {
	REGULATOR_SUPPLY("unused_reg", NULL),
};

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
	 REGULATOR_SUPPLY("avdd_lcd", NULL),
	 REGULATOR_SUPPLY("avdd", "spi2.0"),
	 REGULATOR_SUPPLY("vin", "2-004a"),
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
	REGULATOR_SUPPLY("vdd", "0-004c"),
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
	 REGULATOR_SUPPLY("dvdd", "spi2.0"),
	 REGULATOR_SUPPLY("vdd_lcd_1v8_s", NULL),
};

static struct regulator_consumer_supply max77660_sw2_supply[] = {
	REGULATOR_SUPPLY("vdd_cam_1v8", NULL),
	REGULATOR_SUPPLY("vif", "2-0010"),
	REGULATOR_SUPPLY("vif", "2-0036"),
	REGULATOR_SUPPLY("vdd_i2c", "2-000e"),
	REGULATOR_SUPPLY("vdd", "2-004a"),
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
	REGULATOR_SUPPLY("vdd_prox", "0-0044"),
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
		1, 1, 0, FPS_SRC_3, 0, 0, 0);

MAX77660_PDATA_INIT(BUCK3, buck3,  1200, 1200, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK4, buck4,  600, 1500, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, ENABLE_EN3 | DISABLE_DVFS);

MAX77660_PDATA_INIT(BUCK5, buck5,  1800, 1800, NULL,
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK6, buck6,  1700, 1700, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(BUCK7, buck7,  2650, 2650, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO1, ldo1, 1100, 1100, max77660_rails(buck3),
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO2, ldo2, 2800, 2800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO3, ldo3, 2800, 2800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO4, ldo4, 3000, 3000, NULL,
		1, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO5, ldo5, 1800, 1800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO6, ldo6, 1200, 1200, max77660_rails(buck5),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO7, ldo7, 1050, 1050, max77660_rails(buck3),
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO8, ldo8, 600, 2175, max77660_rails(buck3),
		0, 0, 0, FPS_SRC_NONE, -1, -1, ENABLE_EN3);

MAX77660_PDATA_INIT(LDO9, ldo9, 2800, 2800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO10, ldo10, 1800, 1800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO11, ldo11, 3300, 3300, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO12, ldo12, 1800, 3300, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO13, ldo13, 2850, 2850, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO14, ldo14, 2800, 2800, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO15, ldo15, 1800, 3000, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO16, ldo16, 1800, 3000, NULL,
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO17, ldo17, 1800, 1800, max77660_rails(buck7),
		0, 1, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(LDO18, ldo18, 3100, 3100, NULL,
		0, 0, 1, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW1, sw1, 1800, 1800, max77660_rails(buck5),
		1, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW2, sw2, 1800, 1800, max77660_rails(buck5),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW3, sw3, 1800, 1800, max77660_rails(buck5),
		0, 1, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW4, sw4, 1100, 1100, max77660_rails(buck1),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

MAX77660_PDATA_INIT(SW5, sw5, 1200, 1200, max77660_rails(buck3),
		0, 0, 0, FPS_SRC_NONE, -1, -1, 0);

#define MAX77660_REG(_id, _data) 	\
	[MAX77660_REGULATOR_ID_##_id] = (&max77660_regulator_pdata_##_data)

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

#define MAX77660_INIT_PINS(_id, _gpio, _od, _up_dn, _flags)	\
{								\
	.pin_id			= MAX77660_PINS_##_id,		\
	.gpio_pin_mode		= _gpio,			\
	.open_drain		= _od,				\
	.pullup_dn_normal	= MAX77660_PIN_##_up_dn,	\
	.gpio_init_flag		= _flags,			\
}

static struct max77660_pinctrl_platform_data max77660_pinctrl_pdata[] = {
	MAX77660_INIT_PINS(GPIO0, 0, 1, DEFAULT, 0),
	MAX77660_INIT_PINS(GPIO1, 1, 0, PULL_NORMAL, GPIOF_IN),
	MAX77660_INIT_PINS(GPIO2, 1, 1, PULL_UP, GPIOF_OUT_INIT_HIGH),
	MAX77660_INIT_PINS(GPIO3, 1, 0, PULL_NORMAL, GPIOF_IN),
	MAX77660_INIT_PINS(GPIO4, 1, 0, PULL_NORMAL, GPIOF_IN),
	MAX77660_INIT_PINS(GPIO5, 1, 1, PULL_NORMAL, GPIOF_IN),
	MAX77660_INIT_PINS(GPIO6, 1, 0, PULL_NORMAL, GPIOF_IN),
	MAX77660_INIT_PINS(GPIO7, 0, 0, PULL_NORMAL, 0),
	MAX77660_INIT_PINS(GPIO8, 0, 1, PULL_UP, 0),
	MAX77660_INIT_PINS(GPIO9, 1, 1, PULL_UP, GPIOF_IN),
};

static struct regulator_consumer_supply max77660_vbus_sypply[] = {
	REGULATOR_SUPPLY("usb_vbus", "tegra-ehci.0"),
};

static struct regulator_init_data vbus_reg_init_data = {
	.constraints = {
		.name = "max77660-vbus",
		.min_uV = 0,
		.max_uV = 5000000,
		.valid_modes_mask = (REGULATOR_MODE_NORMAL |
					REGULATOR_MODE_STANDBY),
		.valid_ops_mask = (REGULATOR_CHANGE_MODE |
					REGULATOR_CHANGE_STATUS |
					REGULATOR_CHANGE_VOLTAGE),
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = max77660_vbus_sypply,
};

static struct max77660_charger_platform_data max77660_charger_pdata = {
	.vbus_reg_init_data = &vbus_reg_init_data,
};

struct max77660_adc_platform_data max77660_adc_pdata = {
	.adc_current_uA = 10,
	.adc_avg_sample = 2,
	.adc_ref_enabled = 1,
};

static struct max77660_platform_data max77660_pdata = {
	.irq_base	= MAX77660_IRQ_BASE,
	.gpio_base	= MAX77660_GPIO_BASE,

	.pinctrl_pdata	= max77660_pinctrl_pdata,
	.num_pinctrl	= ARRAY_SIZE(max77660_pinctrl_pdata),

	.charger_pdata = &max77660_charger_pdata,

	.adc_pdata = &max77660_adc_pdata,

	.flags	= 0x00,
	.en_clk32out1 = true,
	.en_clk32out2 = true,
	.use_power_off	= true,
	.system_watchdog_timeout = 32,
	.dvfs_pd = {
		.en_pwm = false,
		.step_voltage_uV = 25000,
		.default_voltage_uV = 900000,
		.base_voltage_uV = 600000,
		.max_voltage_uV = 1100000,
	},
};

static struct i2c_board_info __initdata max77660_regulators[] = {
	{
		/* The I2C address was determined by OTP factory setting */
		I2C_BOARD_INFO("max77660", MAX77660_PWR_I2C_ADDR),
		.irq		= INT_EXTERNAL_PMU,
		.platform_data	= &max77660_pdata,
	},
};

/* LP8755 DC-DC converter */
static struct regulator_consumer_supply lp8755_buck0_supply[] = {
	REGULATOR_SUPPLY("vdd_cpu", NULL),
};

#define lp8755_rail(_id) "lp8755_"#_id

#define LP8755_PDATA_INIT(_name, _minmv, _maxmv, _supply_reg, _always_on, \
	_boot_on, _apply_uv)						\
	static struct regulator_init_data reg_idata_##_name = {		\
		.constraints = {					\
			.name = lp8755_rail(_name),			\
			.min_uV = (_minmv)*1000,			\
			.max_uV = (_maxmv)*1000,			\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |	\
					REGULATOR_MODE_STANDBY),	\
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |	\
					REGULATOR_CHANGE_STATUS |	\
					REGULATOR_CHANGE_VOLTAGE),	\
			.always_on = _always_on,			\
			.boot_on = _boot_on,				\
			.apply_uV = _apply_uv,				\
		},							\
		.num_consumer_supplies =				\
			ARRAY_SIZE(lp8755_##_name##_supply),		\
		.consumer_supplies = lp8755_##_name##_supply,		\
		.supply_regulator = _supply_reg,			\
	}

LP8755_PDATA_INIT(buck0, 500, 1670, NULL, 0, 0, 0);

#define LP8755_REG_PDATA(_sname) &reg_idata_##_sname

static struct regulator_init_data *lp8755_reg_data[LP8755_BUCK_MAX] = {
	LP8755_REG_PDATA(buck0),
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static struct lp8755_platform_data lp8755_pdata;

static struct i2c_board_info lp8755_regulators[] = {
	{
		I2C_BOARD_INFO(LP8755_NAME, 0x60),
		.irq		= 0,
		.platform_data	= &lp8755_pdata,
	},
};

static void lp8755_regulator_init(void)
{
	lp8755_pdata.mphase = MPHASE_CONF6;
	lp8755_pdata.buck_data[LP8755_BUCK0] = lp8755_reg_data[LP8755_BUCK0];
	lp8755_pdata.ramp_us[LP8755_BUCK0] = 230;

	i2c_register_board_info(4, lp8755_regulators,
			ARRAY_SIZE(lp8755_regulators));
}

#ifdef CONFIG_ARCH_TEGRA_HAS_CL_DVFS
/* LP8755LME: fixed 10mV steps from 500mV to 1670mV, with offset 0x80 */
#define PMU_CPU_VDD_MAP_SIZE ((1670000 - 500000) / 10000 + 1)
static struct voltage_reg_map pmu_cpu_vdd_map[PMU_CPU_VDD_MAP_SIZE];
static inline void fill_reg_map(void)
{
	int i;
	for (i = 0; i < PMU_CPU_VDD_MAP_SIZE; i++) {
		pmu_cpu_vdd_map[i].reg_value = i + 0x80;
		pmu_cpu_vdd_map[i].reg_uV = 500000 + 10000 * i;
	}
}

/* board parameters for cpu dfll */
static struct tegra_cl_dvfs_cfg_param ceres_cl_dvfs_param = {
	.sample_rate = 12500,
	.force_mode = TEGRA_CL_DVFS_FORCE_FIXED,
	.cf = 10,
	.ci = 0,
	.cg = 2,

	.droop_cut_value = 0xF,
	.droop_restore_ramp = 0x0,
	.scale_out_ramp = 0x0,
};

static struct tegra_cl_dvfs_platform_data ceres_cl_dvfs_data = {
	.dfll_clk_name = "dfll_cpu",
	.pmu_if = TEGRA_CL_DVFS_PMU_I2C,
	.u.pmu_i2c = {
		.fs_rate = 400000,
		.slave_addr = 0xc0,
		.reg = 0x00,
	},
	.vdd_map = pmu_cpu_vdd_map,
	.vdd_map_size = PMU_CPU_VDD_MAP_SIZE,

	.cfg_param = &ceres_cl_dvfs_param,
};

static int __init ceres_cl_dvfs_init(void)
{
	fill_reg_map();
	tegra_cl_dvfs_device.dev.platform_data = &ceres_cl_dvfs_data;
	platform_device_register(&tegra_cl_dvfs_device);

	return 0;
}
#endif


int __init ceres_regulator_init(void)
{
	void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	u32 pmc_ctrl;
	struct board_info board_info;
	int id;

	/* configure the power management controller to trigger PMU
	 * interrupts when low */
	pmc_ctrl = readl(pmc + PMC_CTRL);
	writel(pmc_ctrl | PMC_CTRL_INTR_LOW, pmc + PMC_CTRL);
	tegra_get_board_info(&board_info);
	max77660_pdata.en_buck2_ext_ctrl = true;
	for (id = 0; id < MAX77660_REGULATOR_ID_NR; ++id)
		max77660_pdata.regulator_pdata[id] = max77660_reg_pdata[id];

	if (board_info.fab > BOARD_FAB_A00) {
		max77660_pinctrl_pdata[MAX77660_PINS_GPIO1].pullup_dn_normal =
				MAX77660_PIN_PULL_UP;
		max77660_pinctrl_pdata[MAX77660_PINS_GPIO1].open_drain = 1;
		max77660_pinctrl_pdata[MAX77660_PINS_GPIO2].pullup_dn_normal =
				MAX77660_PIN_PULL_NORMAL;
		max77660_pinctrl_pdata[MAX77660_PINS_GPIO2].open_drain = 0;

		max77660_regulator_idata_buck1.consumer_supplies = max77660_unused_supply;
		max77660_regulator_idata_buck1.num_consumer_supplies =
			ARRAY_SIZE(max77660_unused_supply);
		max77660_regulator_idata_buck2.consumer_supplies = max77660_buck1_supply;
		max77660_regulator_idata_buck2.num_consumer_supplies =
						ARRAY_SIZE(max77660_buck1_supply);
		lp8755_regulator_init();
	}

	i2c_register_board_info(4, max77660_regulators,
			ARRAY_SIZE(max77660_regulators));

#ifdef CONFIG_ARCH_TEGRA_HAS_CL_DVFS
	ceres_cl_dvfs_init();
#endif

	return 0;
}

/* Always ON /Battery regulator */
static struct regulator_consumer_supply fixed_reg_battery_supply[] = {
	REGULATOR_SUPPLY("vdd_sys_bl", NULL),
	REGULATOR_SUPPLY("vdd_sys_cam", NULL),
};

/* LCD_AVDD_EN From PMU GP6 */
static struct regulator_consumer_supply fixed_reg_avdd_lcd_supply[] = {
	REGULATOR_SUPPLY("unused", NULL),
};

static struct regulator_consumer_supply fixed_reg_vdd_hdmi_5v0_supply[] = {
	REGULATOR_SUPPLY("vdd_hdmi_5v0", "tegradc.1"),
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
FIXED_REG(2,	vdd_hdmi_5v0,	vdd_hdmi_5v0,
	NULL,	0,	0,
	MAX77660_GPIO_BASE + MAX77660_GPIO3,	false,	true,	0,	5000);
/*
 * Creating the fixed regulator device tables
 */

#define ADD_FIXED_REG(_name)    (&fixed_reg_##_name##_dev)

#define CERES_COMMON_FIXED_REG		\
	ADD_FIXED_REG(battery),		\
	ADD_FIXED_REG(avdd_lcd),		\
	ADD_FIXED_REG(vdd_hdmi_5v0)		\

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

/* enable this after verifying ceres uses max77660 regulator
static struct tegra_tsensor_pmu_data tpdata_max77663 = {
	.reset_tegra = 1,
	.pmu_16bit_ops = 0,
	.controller_type = 0,
	.pmu_i2c_addr = 0x3c,
	.i2c_controller_id = 4,
	.poweroff_reg_addr = 0x41,
	.poweroff_reg_data = 0x80,
};
*/

int __init ceres_edp_init(void)
{
	unsigned int regulator_mA;

	regulator_mA = get_maximum_cpu_current_supported();
	if (!regulator_mA)
		regulator_mA = 9000;

	pr_info("%s: CPU regulator %d mA\n", __func__, regulator_mA);
	tegra_init_cpu_edp_limits(regulator_mA);

	regulator_mA = get_maximum_core_current_supported();
	if (!regulator_mA)
		regulator_mA = 4000;

	pr_info("%s: core regulator %d mA\n", __func__, regulator_mA);
	tegra_init_core_edp_limits(regulator_mA);

	return 0;
}

static struct soctherm_platform_data ceres_soctherm_data = {
	.therm = {
		[THERM_CPU] = {
			.zone_enable = true,
			.passive_delay = 1000,
			.num_trips = 3,
			.trips = {
				{
					.cdev_type = "tegra-balanced",
					.trip_temp = 84000,
					.trip_type = THERMAL_TRIP_PASSIVE,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
				},
				{
					.cdev_type = "tegra-heavy",
					.trip_temp = 94000,
					.trip_type = THERMAL_TRIP_HOT,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
				},
				{
					.cdev_type = "tegra-shutdown",
					.trip_temp = 104000,
					.trip_type = THERMAL_TRIP_CRITICAL,
					.upper = THERMAL_NO_LIMIT,
					.lower = THERMAL_NO_LIMIT,
				},
			},
		},
		[THERM_GPU] = {
			.zone_enable = true,
		},
		[THERM_PLL] = {
			.zone_enable = true,
		},
	},
	.throttle = {
		[THROTTLE_HEAVY] = {
			.devs = {
				[THROTTLE_DEV_CPU] = {
					.enable = 1,
				},
			},
		},
	},
	/* enable this after verifying ceres uses max77660 regulator
	.tshut_pmu_trip_data = &tpdata_max77663, */
};

int __init ceres_soctherm_init(void)
{
	tegra_platform_edp_init(ceres_soctherm_data.therm[THERM_CPU].trips,
			&ceres_soctherm_data.therm[THERM_CPU].num_trips);
	tegra_add_tj_trips(ceres_soctherm_data.therm[THERM_CPU].trips,
			&ceres_soctherm_data.therm[THERM_CPU].num_trips);

	return tegra11_soctherm_init(&ceres_soctherm_data);
}
