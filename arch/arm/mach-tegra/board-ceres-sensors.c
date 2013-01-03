/*
 * arch/arm/mach-tegra/board-ceres-sensors.c
 *
 * Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.

 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.

 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>

#include <media/imx091.h>
#include <media/imx132.h>
#include <media/ad5816.h>

#include <mach/pinmux-t11.h>
#include <mach/pinmux.h>

#include "cpu-tegra.h"
#include "devices.h"
#include "board-ceres.h"

static struct nvc_gpio_pdata imx091_gpio_pdata[] = {
	{IMX091_GPIO_RESET, CAM_RSTN, true, false},
	{IMX091_GPIO_PWDN, CAM1_POWER_DWN_GPIO, true, false},
};

#define VI_PINMUX(_pingroup, _mux, _pupd, _tri, _io, _lock, _ioreset) \
	{							\
		.pingroup	= TEGRA_PINGROUP_##_pingroup,	\
		.func		= TEGRA_MUX_##_mux,		\
		.pupd		= TEGRA_PUPD_##_pupd,		\
		.tristate	= TEGRA_TRI_##_tri,		\
		.io		= TEGRA_PIN_##_io,		\
		.lock		= TEGRA_PIN_LOCK_##_lock,	\
		.od		= TEGRA_PIN_OD_DEFAULT,		\
		.ioreset	= TEGRA_PIN_IO_RESET_##_ioreset	\
}

/* Rear sensor clock pinmux settings */
static struct tegra_pingroup_config mclk_disable =
	VI_PINMUX(CAM_MCLK, VI, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);

static struct tegra_pingroup_config mclk_enable =
	VI_PINMUX(CAM_MCLK, VI_ALT3, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);

/* Front sensor clock pinmux settings */
static struct tegra_pingroup_config pbb0_disable =
	VI_PINMUX(GPIO_PBB0, VI, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);

static struct tegra_pingroup_config pbb0_enable =
	VI_PINMUX(GPIO_PBB0, VI_ALT3, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);


static int ceres_focuser_power_on(struct ad5816_power_rail *pw)
{
	int err;

	if (unlikely(WARN_ON(!pw || !pw->vdd || !pw->vdd_i2c)))
		return -EFAULT;

	err = regulator_enable(pw->vdd_i2c);
	if (unlikely(err))
		goto ad5816_vdd_i2c_fail;

	err = regulator_enable(pw->vdd);
	if (unlikely(err))
		goto ad5816_vdd_fail;

	return 0;

ad5816_vdd_fail:
	regulator_disable(pw->vdd_i2c);

ad5816_vdd_i2c_fail:
	pr_err("%s FAILED\n", __func__);

	return -ENODEV;
}

static int ceres_focuser_power_off(struct ad5816_power_rail *pw)
{
	if (unlikely(WARN_ON(!pw || !pw->vdd || !pw->vdd_i2c)))
		return -EFAULT;

	regulator_disable(pw->vdd);
	regulator_disable(pw->vdd_i2c);

	return 0;
}

static int ceres_imx091_power_on(struct nvc_regulator *vreg)
{
	int err;

	if (unlikely(WARN_ON(!vreg)))
		return -EFAULT;

	gpio_set_value(CAM1_POWER_DWN_GPIO, 0);
	usleep_range(10, 20);

	err = regulator_enable(vreg[IMX091_VREG_AVDD].vreg);
	if (err)
		goto imx091_avdd_fail;

	err = regulator_enable(vreg[IMX091_VREG_IOVDD].vreg);
	if (err)
		goto imx091_iovdd_fail;

	usleep_range(1, 2);
	gpio_set_value(CAM1_POWER_DWN_GPIO, 1);

	tegra_pinmux_config_table(&mclk_enable, 1);
	usleep_range(300, 310);

	return 1;

imx091_iovdd_fail:
	regulator_disable(vreg[IMX091_VREG_AVDD].vreg);

imx091_avdd_fail:
	gpio_set_value(CAM1_POWER_DWN_GPIO, 0);

	return -ENODEV;
}

static int ceres_imx091_power_off(struct nvc_regulator *vreg)
{
	if (unlikely(WARN_ON(!vreg)))
		return -EFAULT;

	usleep_range(1, 2);
	tegra_pinmux_config_table(&mclk_disable, 1);
	gpio_set_value(CAM1_POWER_DWN_GPIO, 0);
	usleep_range(1, 2);

	regulator_disable(vreg[IMX091_VREG_IOVDD].vreg);
	regulator_disable(vreg[IMX091_VREG_AVDD].vreg);
	return 0;
}

static int ceres_imx132_power_on(struct imx132_power_rail *pw)
{
	int err;

	if (unlikely(WARN_ON(!pw || !pw->avdd || !pw->iovdd || !pw->dvdd)))
		return -EFAULT;

	gpio_set_value(CAM2_POWER_DWN_GPIO, 0);

	tegra_pinmux_config_table(&pbb0_enable, 1);


	err = regulator_enable(pw->avdd);
	if (unlikely(err))
		goto imx132_avdd_fail;

	err = regulator_enable(pw->dvdd);
	if (unlikely(err))
		goto imx132_dvdd_fail;

	err = regulator_enable(pw->iovdd);
	if (unlikely(err))
		goto imx132_iovdd_fail;

	usleep_range(1, 2);

	gpio_set_value(CAM2_POWER_DWN_GPIO, 1);

	return 0;

imx132_iovdd_fail:
	regulator_disable(pw->dvdd);

imx132_dvdd_fail:
	regulator_disable(pw->avdd);

imx132_avdd_fail:
	tegra_pinmux_config_table(&pbb0_disable, 1);

	return -ENODEV;
}

static int ceres_imx132_power_off(struct imx132_power_rail *pw)
{
	if (unlikely(WARN_ON(!pw || !pw->avdd || !pw->iovdd || !pw->dvdd)))
		return -EFAULT;

	gpio_set_value(CAM2_POWER_DWN_GPIO, 0);

	usleep_range(1, 2);

	regulator_disable(pw->iovdd);
	regulator_disable(pw->dvdd);
	regulator_disable(pw->avdd);

	tegra_pinmux_config_table(&pbb0_disable, 1);

	return 0;
}

static struct nvc_imager_cap imx091_cap = {
	.identifier		= "IMX091",
	.sensor_nvc_interface	= 3,
	.pixel_types[0]		= 0x100,
	.orientation		= 0,
	.direction		= 0,
	.initial_clock_rate_khz	= 6000,
	.clock_profiles[0] = {
		.external_clock_khz	= 24000,
		.clock_multiplier	= 10416667, /* value / 1,000,000 */
	},
	.clock_profiles[1] = {
		.external_clock_khz	= 0,
		.clock_multiplier	= 0,
	},
	.h_sync_edge		= 0,
	.v_sync_edge		= 0,
	.mclk_on_vgp0		= 0,
	.csi_port		= 0,
	.data_lanes		= 4,
	.virtual_channel_id	= 0,
	.discontinuous_clk_mode	= 0,
	.cil_threshold_settle	= 0xd,
	.min_blank_time_width	= 16,
	.min_blank_time_height	= 16,
	.preferred_mode_index	= 0,
	.focuser_guid		= 0,
	.torch_guid		= 0,
	.cap_version		= NVC_IMAGER_CAPABILITIES_VERSION2,
};

static struct imx091_platform_data ceres_imx091_data = {
	.num			= 0,
	.sync			= 0,
	.dev_name		= "camera",
	.gpio_count		= ARRAY_SIZE(imx091_gpio_pdata),
	.gpio			= imx091_gpio_pdata,
	.flash_cap		= {
		.sdo_trigger_enabled = 1,
		.adjustable_flash_timing = 1,
	},
	.cap			= &imx091_cap,
	.power_on		= ceres_imx091_power_on,
	.power_off		= ceres_imx091_power_off,
};

struct imx132_platform_data ceres_imx132_data = {
	.power_on = ceres_imx132_power_on,
	.power_off = ceres_imx132_power_off,
};

static struct ad5816_platform_data ceres_ad5816_pdata = {
	.cfg		= 0,
	.num		= 0,
	.sync		= 0,
	.dev_name	= "focuser",
	.power_on	= ceres_focuser_power_on,
	.power_off	= ceres_focuser_power_off,
};

static struct i2c_board_info ceres_i2c_board_info_e1707[] = {
	{
		I2C_BOARD_INFO("imx091", 0x10),
		.platform_data = &ceres_imx091_data,
	},
	{
		I2C_BOARD_INFO("imx132", 0x36),
		.platform_data = &ceres_imx132_data,
	},
	{
		I2C_BOARD_INFO("ad5816", 0x0E),
		.platform_data = &ceres_ad5816_pdata,
	},
};

static struct i2c_board_info __initdata ceres_i2c_board_info_max44005[] = {
	{
		I2C_BOARD_INFO("max44005", 0x44),
	},
};

static int ceres_camera_init(void)
{
	tegra_pinmux_config_table(&mclk_disable, 1);
	tegra_pinmux_config_table(&pbb0_disable, 1);

	i2c_register_board_info(2, ceres_i2c_board_info_e1707,
		ARRAY_SIZE(ceres_i2c_board_info_e1707));
	return 0;
}

int __init ceres_sensors_init(void)
{
	ceres_camera_init();

	i2c_register_board_info(0, ceres_i2c_board_info_max44005,
			ARRAY_SIZE(ceres_i2c_board_info_max44005));

	return 0;
}
