/*
 * arch/arm/mach-tegra/board-atlantis-power.c
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
#include <linux/mfd/palmas.h>
#include <linux/platform_data/lp8755.h>
#include <linux/irq.h>
#include <linux/input/drv2603-vibrator.h>

#include <asm/mach-types.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/gpio-tegra.h>

#include "pm.h"
#include "board.h"
#include "tegra-board-id.h"
#include "board-atlantis.h"
#include "board-pmu-defines.h"
#include "devices.h"

#define PMC_CTRL                0x0
#define PMC_CTRL_INTR_LOW       (1 << 17)
#define BOARD_SKU_100		100
#define BOARD_SKU_110		110
#define BOARD_SKU_120		120

/* TPS80036 consumer rails */
static struct regulator_consumer_supply palmas_smps12_supply[] = {
	REGULATOR_SUPPLY("vdd_cpu", NULL),
	REGULATOR_SUPPLY("vdd_cpu_dbg", NULL),
};

static struct regulator_consumer_supply palmas_smps3_supply[] = {
	REGULATOR_SUPPLY("vdd_bb", NULL),
};

static struct regulator_consumer_supply palmas_smps6_supply[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
	REGULATOR_SUPPLY("vdd_core_dbg", NULL),
};

static struct regulator_consumer_supply palmas_smps7_supply[] = {
	REGULATOR_SUPPLY("vddio_sd_slot", "sdhci-tegra.3"),
};

static struct regulator_consumer_supply palmas_smps8_supply[] = {
	REGULATOR_SUPPLY("vdd2_lpddr3", NULL),
	REGULATOR_SUPPLY("vddca_lpddr3", NULL),
	REGULATOR_SUPPLY("vrefca_lpddr3", NULL),
	REGULATOR_SUPPLY("vddio_hsic", "tegra-ehci.1"),
	REGULATOR_SUPPLY("vddio_hsic", "tegra-ehci.2"),
	REGULATOR_SUPPLY("vdd_hsic_modem2", NULL),
};

static struct regulator_consumer_supply palmas_smps9_supply[] = {
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
	REGULATOR_SUPPLY("vdd_1v8_sdmmc_emmc", "sdhci-tegra.3"),
	REGULATOR_SUPPLY("vdd_ddr1", NULL),
	REGULATOR_SUPPLY("vdd_1v8_hdmi", "tegradc.1"),
	REGULATOR_SUPPLY("vdd_ts", NULL),
	REGULATOR_SUPPLY("vdd_1v8_audio_mb", NULL),
	REGULATOR_SUPPLY("vdd_1v8_uart_mb", NULL),
	REGULATOR_SUPPLY("vdd_1v8_com", NULL),
	REGULATOR_SUPPLY("vdd_gps", NULL),
	REGULATOR_SUPPLY("vdd_nfc", NULL),
	REGULATOR_SUPPLY("vdd_dtv", NULL),
	REGULATOR_SUPPLY("vdd_1v8_bb_mb", NULL),
	REGULATOR_SUPPLY("avdd_1v8_rf", NULL),
	REGULATOR_SUPPLY("vdd_modem2", NULL),
	REGULATOR_SUPPLY("vdd_dbg", NULL),
	REGULATOR_SUPPLY("vdd_1v8_ldo_ddr_hs", NULL),
};

static struct regulator_consumer_supply palmas_ldo1_supply[] = {
	REGULATOR_SUPPLY("unused", NULL),
};

static struct regulator_consumer_supply palmas_ldo2_supply[] = {
	REGULATOR_SUPPLY("vddio_sd_slot", "sdhci-tegra.2"),
};

static struct regulator_consumer_supply palmas_ldo3_supply[] = {
	REGULATOR_SUPPLY("vddio_sim0", NULL),
};

static struct regulator_consumer_supply palmas_ldo4_supply[] = {
	REGULATOR_SUPPLY("avdd_plla_m_p", NULL),
	REGULATOR_SUPPLY("avdd_pllc", NULL),
	REGULATOR_SUPPLY("avdd_hdmi_pll", "tegradc.1"),
	REGULATOR_SUPPLY("avdd_mipi_pllu", NULL),
	REGULATOR_SUPPLY("avdd_ddr_hs", NULL),
};

/* powered by vdd_soc_ldo5 - smps6 or vdd_1v2_ldo5 - smps 8 */
static struct regulator_consumer_supply palmas_ldo5_supply[] = {
	REGULATOR_SUPPLY("vdd_rtc", NULL),
};

static struct regulator_consumer_supply palmas_ldo6_supply[] = {
	REGULATOR_SUPPLY("avdd_cam1", NULL),
	REGULATOR_SUPPLY("vana", "2-0010"),
};

static struct regulator_consumer_supply palmas_ldo7_supply[] = {
	REGULATOR_SUPPLY("vddio_sim1", NULL),
};

static struct regulator_consumer_supply palmas_ldo8_supply[] = {
	REGULATOR_SUPPLY("vpp_fuse", NULL),
	REGULATOR_SUPPLY("vpp_bb_fuse", NULL),
};

static struct regulator_consumer_supply palmas_ldo9_supply[] = {
	REGULATOR_SUPPLY("dvdd_bb_pll", NULL),
	REGULATOR_SUPPLY("avdd_bb_pll", NULL),
};

static struct regulator_consumer_supply palmas_ldo10_supply[] = {
	REGULATOR_SUPPLY("vddio_sdmmc", "sdhci-tegra.2"),
};

static struct regulator_consumer_supply palmas_ldo11_supply[] = {
	REGULATOR_SUPPLY("vdd_mb", NULL),
	REGULATOR_SUPPLY("vdd_temp", NULL),
	REGULATOR_SUPPLY("vdd_nfc_2v8", NULL),
	REGULATOR_SUPPLY("vdd_irled", NULL),
	REGULATOR_SUPPLY("vdd_sensor_2v8", NULL),
	REGULATOR_SUPPLY("vdd_pm_2v8", NULL),
	REGULATOR_SUPPLY("vdd", "0-0029"),
	REGULATOR_SUPPLY("vdd", "0-004c"),
	REGULATOR_SUPPLY("vdd", "0-0069"),
};

static struct regulator_consumer_supply palmas_ldo12_supply[] = {
	REGULATOR_SUPPLY("vdd_dis_lcd", NULL),
	REGULATOR_SUPPLY("vdd_dis_ts", NULL),
	REGULATOR_SUPPLY("dvdd", "spi2.0"),
	REGULATOR_SUPPLY("vdd_lcd_1v8_s", NULL),
};

static struct regulator_consumer_supply palmas_ldo13_supply[] = {
	REGULATOR_SUPPLY("vdd_cam_1v8", NULL),
	REGULATOR_SUPPLY("vif", "2-0010"),
	REGULATOR_SUPPLY("vif", "2-0036"),
	REGULATOR_SUPPLY("vdd_i2c", "2-000e"),
	REGULATOR_SUPPLY("vdd", "2-004a"),
	REGULATOR_SUPPLY("vi2c", "2-0030"),
};

static struct regulator_consumer_supply palmas_ldo14_supply[] = {
	REGULATOR_SUPPLY("avdd_2v8_cam_af", NULL),
	REGULATOR_SUPPLY("avdd_cam2", NULL),
	REGULATOR_SUPPLY("vana", "2-0036"),
	REGULATOR_SUPPLY("vdd", "2-000e"),
};

static struct regulator_consumer_supply palmas_ldoln_supply[] = {
	REGULATOR_SUPPLY("avdd_aud", NULL),
};

static struct regulator_consumer_supply palmas_ldousb_supply[] = {
	REGULATOR_SUPPLY("avdd_usb", "tegra-udc.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.0"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.1"),
	REGULATOR_SUPPLY("avdd_usb", "tegra-ehci.2"),
	REGULATOR_SUPPLY("avdd_hdmi", "tegradc.1"),
	REGULATOR_SUPPLY("vdd_sys_dtv_3v3", NULL),
};

static struct regulator_consumer_supply palmas_regen1_supply[] = {
	REGULATOR_SUPPLY("amic_bias_en", NULL),
};

static struct regulator_consumer_supply palmas_regen2_supply[] = {
	REGULATOR_SUPPLY("vdd_cam_1v2", NULL),
	REGULATOR_SUPPLY("vdig", "2-0010"),
	REGULATOR_SUPPLY("vdig", "2-0036"),
};

static struct regulator_consumer_supply palmas_regen4_supply[] = {
	REGULATOR_SUPPLY("avdd_dsi_csi", "tegradc.0"),
	REGULATOR_SUPPLY("avdd_dsi_csi", "tegradc.1"),
	REGULATOR_SUPPLY("avdd_dsi_csi", "vi"),
	REGULATOR_SUPPLY("vdd_1v2_lcd", NULL),
	REGULATOR_SUPPLY("vdd_1v2_cdc", NULL),
	REGULATOR_SUPPLY("vrefdq_lpddr3", NULL),
	REGULATOR_SUPPLY("vddio_ddr", NULL),
	REGULATOR_SUPPLY("vddq_lpddr3", NULL),
	REGULATOR_SUPPLY("vdd_prox", "0-0044"),
};

static struct regulator_consumer_supply palmas_regen5_supply[] = {
	REGULATOR_SUPPLY("vdd_aud_dgtl", NULL),
	REGULATOR_SUPPLY("vdd_aud_anlg", NULL),
	REGULATOR_SUPPLY("vdd_aud_mic", NULL),
	REGULATOR_SUPPLY("vdd_sw_1v8_snsr", NULL),
	REGULATOR_SUPPLY("vdd", "0-0044"),
	REGULATOR_SUPPLY("vlogic", "0-0069"),
};

static struct regulator_consumer_supply palmas_regen7_supply[] = {
	REGULATOR_SUPPLY("avdd_lcd_ext", NULL),
	REGULATOR_SUPPLY("avdd", "spi2.0"),
	REGULATOR_SUPPLY("vin", "2-004a"),
};

static struct regulator_consumer_supply palmas_chargerpump_supply[] = {
};

#undef PALMAS_PDATA_INIT
#define PALMAS_PDATA_INIT(_name, _minmv, _maxmv, _supply_reg, _always_on, \
	_boot_on, _apply_uv)						\
	static struct regulator_init_data reg_idata_##_name = {		\
		.constraints = {					\
			.name = palmas_rails(_name),			\
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
			ARRAY_SIZE(palmas_##_name##_supply),		\
		.consumer_supplies = palmas_##_name##_supply,		\
		.supply_regulator = _supply_reg,			\
	}

PALMAS_PDATA_INIT(smps12, 900,  1300, NULL, 0, 0, 0);
PALMAS_PDATA_INIT(smps3, 1100,  1100, NULL, 0, 0, 0);
PALMAS_PDATA_INIT(smps6, 900,  1400, NULL, 0, 0, 0);
PALMAS_PDATA_INIT(smps7, 2840,  2860, NULL, 0, 0, 0);
PALMAS_PDATA_INIT(smps8, 1200,  1200, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(smps9, 1800,  1800, NULL, 1, 1, 1);
PALMAS_PDATA_INIT(ldo1, 3200,  3200, NULL, 1, 1, 1);
PALMAS_PDATA_INIT(ldo2, 2850,  2850, palmas_rails(smps7), 0, 0, 1);
PALMAS_PDATA_INIT(ldo3, 1800,  3000, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(ldo4, 1050,  1050, palmas_rails(smps8), 1, 0, 1);
PALMAS_PDATA_INIT(ldo5, 1100,  1100, palmas_rails(smps8), 1, 0, 1);
PALMAS_PDATA_INIT(ldo6, 2700,  2700, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(ldo7, 1800,  3000, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(ldo8, 1800,  1800, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(ldo9, 900,  900, palmas_rails(smps8), 0, 0, 1);
PALMAS_PDATA_INIT(ldo10, 1800,  3300, palmas_rails(smps7), 0, 0, 0);
PALMAS_PDATA_INIT(ldo11, 2800,  2800, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(ldo12, 1800,  1800, palmas_rails(smps9), 0, 0, 1);
PALMAS_PDATA_INIT(ldo13, 1800,  1800, palmas_rails(smps9), 0, 0, 1);
PALMAS_PDATA_INIT(ldo14, 2800,  2800, NULL, 0, 0, 1);
PALMAS_PDATA_INIT(ldoln, 2800,  2800, NULL, 1, 1, 0);
PALMAS_PDATA_INIT(ldousb, 3300,  3300, NULL, 0, 0, 0);
PALMAS_PDATA_INIT(regen1, 4300,  4300, NULL, 1, 1, 0);
PALMAS_PDATA_INIT(regen2, 1200,  1200, palmas_rails(smps8), 0, 0, 0);
PALMAS_PDATA_INIT(regen4, 1200,  1200, palmas_rails(smps9), 1, 1, 0);
PALMAS_PDATA_INIT(regen5, 1800,  1800, palmas_rails(smps8), 1, 1, 0);
PALMAS_PDATA_INIT(regen7, 2800,  2800, NULL, 1, 0, 1);
PALMAS_PDATA_INIT(chargerpump, 5000,  5000, NULL, 0, 0, 0);

#define PALMAS_REG_PDATA(_sname) &reg_idata_##_sname

static struct regulator_init_data *atlantis_reg_data[PALMAS_NUM_REGS] = {
	PALMAS_REG_PDATA(smps12),
	NULL,
	PALMAS_REG_PDATA(smps3),
	NULL,
	NULL,
	PALMAS_REG_PDATA(smps6),
	PALMAS_REG_PDATA(smps7),
	PALMAS_REG_PDATA(smps8),
	PALMAS_REG_PDATA(smps9),
	NULL,
	PALMAS_REG_PDATA(ldo1),
	PALMAS_REG_PDATA(ldo2),
	PALMAS_REG_PDATA(ldo3),
	PALMAS_REG_PDATA(ldo4),
	PALMAS_REG_PDATA(ldo5),
	PALMAS_REG_PDATA(ldo6),
	PALMAS_REG_PDATA(ldo7),
	PALMAS_REG_PDATA(ldo8),
	PALMAS_REG_PDATA(ldo9),
	PALMAS_REG_PDATA(ldo10),
	PALMAS_REG_PDATA(ldo11),
	PALMAS_REG_PDATA(ldo12),
	PALMAS_REG_PDATA(ldo13),
	PALMAS_REG_PDATA(ldo14),
	PALMAS_REG_PDATA(ldoln),
	PALMAS_REG_PDATA(ldousb),
	PALMAS_REG_PDATA(regen1),
	PALMAS_REG_PDATA(regen2),
	NULL,
	PALMAS_REG_PDATA(regen4),
	PALMAS_REG_PDATA(regen5),
	PALMAS_REG_PDATA(regen7),
	NULL,
	NULL,
	PALMAS_REG_PDATA(chargerpump),
};

#define PALMAS_REG_INIT(_name, _warm_reset, _roof_floor, _mode_sleep,	\
		_tstep, _vsel)						\
	static struct palmas_reg_init reg_init_data_##_name = {		\
		.warm_reset = _warm_reset,				\
		.roof_floor =	_roof_floor,				\
		.mode_sleep = _mode_sleep,		\
		.tstep = _tstep,			\
		.vsel = _vsel,		\
	}

PALMAS_REG_INIT(smps12, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(smps123, 0, PALMAS_EXT_CONTROL_ENABLE1, 0, 0, 0);
PALMAS_REG_INIT(smps3, 0, PALMAS_EXT_CONTROL_ENABLE2, 0, 0, 0);
PALMAS_REG_INIT(smps45, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(smps457, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(smps6, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(smps7, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(smps8, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(smps9, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(smps10, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(ldo1, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(ldo2, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(ldo3, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(ldo4, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(ldo5, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(ldo6, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(ldo7, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(ldo8, 0, 0, 0, 0, 0);
PALMAS_REG_INIT(ldo9, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(ldoln, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);
PALMAS_REG_INIT(ldousb, 0, PALMAS_EXT_CONTROL_NSLEEP, 0, 0, 0);

#define PALMAS_REG_INIT_DATA(_sname) &reg_init_data_##_sname

static struct palmas_reg_init *atlantis_reg_init[PALMAS_NUM_REGS] = {
	PALMAS_REG_INIT_DATA(smps12),
	PALMAS_REG_INIT_DATA(smps123),
	PALMAS_REG_INIT_DATA(smps3),
	PALMAS_REG_INIT_DATA(smps45),
	PALMAS_REG_INIT_DATA(smps457),
	PALMAS_REG_INIT_DATA(smps6),
	PALMAS_REG_INIT_DATA(smps7),
	PALMAS_REG_INIT_DATA(smps8),
	PALMAS_REG_INIT_DATA(smps9),
	PALMAS_REG_INIT_DATA(smps10),
	PALMAS_REG_INIT_DATA(ldo1),
	PALMAS_REG_INIT_DATA(ldo2),
	PALMAS_REG_INIT_DATA(ldo3),
	PALMAS_REG_INIT_DATA(ldo4),
	PALMAS_REG_INIT_DATA(ldo5),
	PALMAS_REG_INIT_DATA(ldo6),
	PALMAS_REG_INIT_DATA(ldo7),
	PALMAS_REG_INIT_DATA(ldo8),
	PALMAS_REG_INIT_DATA(ldo9),
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	PALMAS_REG_INIT_DATA(ldoln),
	PALMAS_REG_INIT_DATA(ldousb),
};

static struct regulator_consumer_supply fixed_reg_battery_supply[] = {
	REGULATOR_SUPPLY("vdd_sys_cam", NULL),
	REGULATOR_SUPPLY("vdd_sys_bl", NULL),
	REGULATOR_SUPPLY("vdd_sys_com", NULL),
	REGULATOR_SUPPLY("vdd_sys_gps", NULL),
	REGULATOR_SUPPLY("vdd_sys_nfc", NULL),
	REGULATOR_SUPPLY("vdd_vbrtr", NULL),
	REGULATOR_SUPPLY("vdd_sys_kb", NULL),
	REGULATOR_SUPPLY("vdd_sys_flash", NULL),
	REGULATOR_SUPPLY("vdd_sys_lcd", NULL),
	REGULATOR_SUPPLY("vdd_sys_cdc", NULL),
	REGULATOR_SUPPLY("vdd_sys_spkr", NULL),
	REGULATOR_SUPPLY("vin", "1-0036"),
};

static struct regulator_consumer_supply fixed_reg_avdd_lcd_supply[] = {
	REGULATOR_SUPPLY("avdd_lcd_ext", NULL),
	REGULATOR_SUPPLY("avdd", "spi2.0"),
	REGULATOR_SUPPLY("vin", "2-004a"),
};

static struct regulator_consumer_supply fixed_reg_vdd_hdmi_5v0_supply[] = {
	REGULATOR_SUPPLY("vdd_hdmi_5v0", "tegradc.1"),
};

/* Macro for defining fixed regulator sub device data */
#define FIXED_SUPPLY(_name) "fixed_reg_"#_name
#define FIXED_REG(_id, _var, _name, _in_supply, _always_on, _boot_on,	\
	_gpio_nr, _open_drain, _active_high, _boot_state, _millivolts,	\
	_sdelay)							\
	static struct regulator_init_data ri_data_##_var =		\
	{								\
		.supply_regulator = _in_supply,				\
		.num_consumer_supplies =				\
			ARRAY_SIZE(fixed_reg_##_name##_supply),	\
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
	static struct fixed_voltage_config fixed_reg_##_var##_pdata = \
	{								\
		.supply_name = FIXED_SUPPLY(_name),			\
		.microvolts = _millivolts * 1000,			\
		.gpio = _gpio_nr,					\
		.gpio_is_open_drain = _open_drain,			\
		.enable_high = _active_high,				\
		.enabled_at_boot = _boot_state,				\
		.init_data = &ri_data_##_var,				\
		.startup_delay = _sdelay				\
	};								\
	static struct platform_device fixed_reg_##_var##_dev = {	\
		.name = "reg-fixed-voltage",				\
		.id = _id,						\
		.dev = {						\
			.platform_data = &fixed_reg_##_var##_pdata,	\
		},							\
	}

FIXED_REG(0,    battery,        battery,
	NULL,   0,      0,
	-1,     false, true,    0,      4200,   0);

FIXED_REG(1, avdd_lcd, avdd_lcd,
	palmas_rails(ldo1), 1, 1,
	PALMAS_TEGRA_GPIO_BASE + PALMAS_GPIO3,
	false, true, 1, 3200, 0);

FIXED_REG(2, vdd_hdmi_5v0, vdd_hdmi_5v0,
	palmas_rails(chargerpump), 0, 0, PALMAS_TEGRA_GPIO_BASE + PALMAS_GPIO9,
	false, true, 0, 5000, 250000);

#define ADD_FIXED_REG(_name)    (&fixed_reg_##_name##_dev)

#define ATLANTIS_COMMON_FIXED_REG	\
	ADD_FIXED_REG(battery),		\
	ADD_FIXED_REG(avdd_lcd),	\
	ADD_FIXED_REG(vdd_hdmi_5v0)

static struct platform_device *fixed_reg_devs_e1670[] = {
	ATLANTIS_COMMON_FIXED_REG
};

int __init atlantis_fixed_regulator_init(void)
{
	struct board_info board_info;
	int num_fixed_regulators = ARRAY_SIZE(fixed_reg_devs_e1670);

	tegra_get_board_info(&board_info);
	if (board_info.fab == BOARD_FAB_A00)
		num_fixed_regulators--;
	return platform_add_devices(fixed_reg_devs_e1670,
					num_fixed_regulators);
}

static struct palmas_pmic_platform_data pmic_platform = {
	.enable_ldo8_tracking = true,
	.disabe_ldo8_tracking_suspend = true,
};

static struct palmas_pinctrl_config palmas_pincfg[] = {
	PALMAS_PINMUX(POWERGOOD, POWERGOOD, DEFAULT, DEFAULT),
	PALMAS_PINMUX(VAC, VAC, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO0, ID, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO1, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO2, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO3, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO4, GPIO, PULL_UP, DEFAULT),
	PALMAS_PINMUX(GPIO5, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO6, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO7, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO8, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO9, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO10, GPIO, PULL_UP, DEFAULT),
	PALMAS_PINMUX(GPIO11, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO12, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO13, GPIO, DEFAULT, DEFAULT),
	PALMAS_PINMUX(GPIO14, GPIO, PULL_UP, DEFAULT),
	PALMAS_PINMUX(GPIO15, GPIO, DEFAULT, DEFAULT),
};

static struct palmas_pinctrl_platform_data palmas_pinctrl_pdata = {
	.pincfg = palmas_pincfg,
	.num_pinctrl = ARRAY_SIZE(palmas_pincfg),
	.dvfs1_enable = true,
	.dvfs2_enable = false,
};

static struct palmas_extcon_platform_data palmas_extcon_pdata = {
	.connection_name = "palmas-extcon",
	.enable_vbus_detection = true,
	.enable_id_pin_detection = true,
};

struct palmas_clk32k_init_data atlantis_palmas_clk32k_idata[] = {
	{
		.clk32k_id = PALMAS_CLOCK32KG,
		.enable = true,
	}, {
		.clk32k_id = PALMAS_CLOCK32KG_AUDIO,
		.enable = true,
	},
};

static struct palmas_platform_data palmas_pdata = {
	.gpio_base = PALMAS_TEGRA_GPIO_BASE,
	.irq_base = PALMAS_TEGRA_IRQ_BASE,
	.pmic_pdata = &pmic_platform,
	.clk32k_init_data =  atlantis_palmas_clk32k_idata,
	.clk32k_init_data_size = ARRAY_SIZE(atlantis_palmas_clk32k_idata),
	.irq_type = IRQ_TYPE_LEVEL_HIGH,
	.use_power_off = true,
	.watchdog_timer_initial_period = 128,
	.pinctrl_pdata = &palmas_pinctrl_pdata,
	.extcon_pdata = &palmas_extcon_pdata,
};

static struct i2c_board_info palma_device[] = {
	{
		I2C_BOARD_INFO("tps80036", 0x58),
		.irq		= INT_EXTERNAL_PMU,
		.platform_data	= &palmas_pdata,
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

static struct drv2603_platform_data atlantis_vibrator_pdata = {
	.pwm_id = 0,
	.vibrator_mode = ERM_MODE,
	.gpio = PALMAS_TEGRA_GPIO_BASE + PALMAS_GPIO6,
	.duty_cycle = 80,
	.edp_states = {64, 0},
};

static struct platform_device atlantis_vibrator_device = {
	.name = "drv2603-vibrator",
	.id = -1,
	.dev = {
		.platform_data = &atlantis_vibrator_pdata,
	},
};

int atlantis_pwm_init(void)
{
	return platform_device_register(&tegra_pwfm0_device);
}

int atlantis_vibrator_init(void)
{
	return platform_device_register(&atlantis_vibrator_device);
}

int __init atlantis_regulator_init(void)
{
	struct board_info board_info;
	void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	u32 pmc_ctrl;
	int i;

	/* configure the power management controller to trigger PMU
	 * interrupts when high */
	pmc_ctrl = readl(pmc + PMC_CTRL);
	writel(pmc_ctrl & ~PMC_CTRL_INTR_LOW, pmc + PMC_CTRL);


	tegra_get_board_info(&board_info);
	for (i = 0; i < PALMAS_NUM_REGS ; i++) {
		pmic_platform.reg_data[i] = atlantis_reg_data[i];
		pmic_platform.reg_init[i] = atlantis_reg_init[i];
	}
	if (board_info.fab != BOARD_FAB_A00) {
		pmic_platform.reg_data[PALMAS_REG_REGEN7] = NULL;
		pmic_platform.reg_init[PALMAS_REG_REGEN7] = NULL;
	}
	if (board_info.sku == BOARD_SKU_110) {
		pmic_platform.reg_data[PALMAS_REG_SMPS12] = NULL;
		pmic_platform.reg_init[PALMAS_REG_SMPS12] = NULL;

		lp8755_regulator_init();
	} else if (board_info.sku == BOARD_SKU_100) {
		reg_init_data_smps12.roof_floor = PALMAS_EXT_CONTROL_ENABLE1;
	} else if (board_info.sku == BOARD_SKU_120) {
		pmic_platform.reg_data[PALMAS_REG_SMPS12] =
				atlantis_reg_data[PALMAS_REG_SMPS6];
		pmic_platform.reg_data[PALMAS_REG_SMPS12]->constraints.name =
				palmas_rails(smps12);
		pmic_platform.reg_init[PALMAS_REG_SMPS12] =
				atlantis_reg_init[PALMAS_REG_SMPS6];
		pmic_platform.reg_data[PALMAS_REG_SMPS6] = NULL;
		pmic_platform.reg_init[PALMAS_REG_SMPS6] = NULL;

		lp8755_regulator_init();
	}

	i2c_register_board_info(4, palma_device,
			ARRAY_SIZE(palma_device));
	return 0;
}
