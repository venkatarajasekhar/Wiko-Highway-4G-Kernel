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
#include <linux/mpu.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio.h>

#include <media/imx091.h>
#include <media/imx132.h>
#include <media/ad5816.h>
#include <media/max77387.h>
#ifdef CONFIG_ARCH_TEGRA_11x_SOC
#include <mach/pinmux-t11.h>
#else
#include <mach/pinmux-t14.h>
#endif
#include <mach/pinmux.h>
#include <linux/nct1008.h>
#include <mach/edp.h>
#include <generated/mach-types.h>

#include "cpu-tegra.h"
#include "devices.h"
#include "board-common.h"
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

#ifdef CONFIG_ARCH_TEGRA_11x_SOC
/* Rear sensor clock pinmux settings */
static struct tegra_pingroup_config mclk_disable =
	VI_PINMUX(CAM_MCLK, VI, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);

static struct tegra_pingroup_config mclk_enable =
	VI_PINMUX(CAM_MCLK, VI_ALT3, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);

/* Front sensor clock pinmux settings */
static struct tegra_pingroup_config pbb0_disable =
	VI_PINMUX(CAM_MCLK, VI, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);

static struct tegra_pingroup_config pbb0_enable =
	VI_PINMUX(CAM_MCLK, VI_ALT3, NORMAL, NORMAL, OUTPUT, DEFAULT, DEFAULT);
#else
/* Rear sensor clock pinmux settings */
static struct tegra_pingroup_config mclk_disable =
	VI_PINMUX(CAM2_MCLK, VIMCLK2, NORMAL, NORMAL, OUTPUT, DEFAULT, DISABLE);

static struct tegra_pingroup_config mclk_enable =
	VI_PINMUX(CAM2_MCLK, VIMCLK2_ALT_ALT, PULL_UP,
			NORMAL, OUTPUT, DEFAULT, DISABLE);

/* Front sensor clock pinmux settings */
static struct tegra_pingroup_config pbb0_disable =
	VI_PINMUX(CAM2_MCLK, VIMCLK2, NORMAL, NORMAL, OUTPUT, DEFAULT, DISABLE);

static struct tegra_pingroup_config pbb0_enable =
	VI_PINMUX(CAM2_MCLK, VIMCLK2_ALT_ALT, PULL_UP,
			NORMAL, OUTPUT, DEFAULT, DISABLE);
#endif

static struct throttle_table tj_throttle_table[] = {
	/* CPU_THROT_LOW cannot be used by other than CPU */
	/* NO_CAP cannot be used by CPU */
	/*    CPU,   C2BUS,   C3BUS,    SCLK,     EMC */
	{ { 1938000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1836000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1734000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1632000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1530000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1428000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1326000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1224000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1122000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ { 1020000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ {  918000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ {  816000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ {  714000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ {  612000,  NO_CAP,  NO_CAP,  NO_CAP,  NO_CAP } },
	{ {  612000,  564000,  564000,  NO_CAP,  NO_CAP } },
	{ {  612000,  528000,  528000,  NO_CAP,  NO_CAP } },
	{ {  612000,  492000,  492000,  NO_CAP,  NO_CAP } },
	{ {  612000,  420000,  420000,  NO_CAP,  NO_CAP } },
	{ {  612000,  408000,  408000,  NO_CAP,  NO_CAP } },
	{ {  612000,  360000,  360000,  NO_CAP,  NO_CAP } },
	{ {  612000,  360000,  360000,  312000,  NO_CAP } },
	{ {  510000,  360000,  360000,  312000,  480000 } },
	{ {  408000,  360000,  360000,  312000,  480000 } },
	{ {  408000,  276000,  276000,  208000,  480000 } },
	{ {  355200,  276000,  276000,  208000,  204000 } },
	{ {  306000,  276000,  276000,  208000,  204000 } },
	{ {  297600,  276000,  228000,  208000,  102000 } },
	{ {  240000,  276000,  228000,  208000,  102000 } },
	{ {  102000,  276000,  228000,  208000,  102000 } },
	{ { CPU_THROT_LOW,  276000,  228000,  208000,  102000 } },
};

static struct balanced_throttle tj_throttle = {
	.throt_tab_size = ARRAY_SIZE(tj_throttle_table),
	.throt_tab = tj_throttle_table,
};

static int __init ceres_throttle_init(void)
{
	if (machine_is_ceres())
		balanced_throttle_register(&tj_throttle, "tegra-balanced");
	return 0;
}
module_init(ceres_throttle_init);

static struct nct1008_platform_data ceres_nct1008_pdata = {
	.supported_hwrev = true,
	.ext_range = true,
	.conv_rate = 0x09, /* 0x09 corresponds to 32Hz conversion rate */
	.shutdown_ext_limit = 95, /* C */
	.shutdown_local_limit = 100, /* C */

	.passive_delay = 2000,

	.num_trips = 1,
	.trips = {
		/* Thermal Throttling */
		{
			.cdev_type = "tegra-balanced",
			.trip_temp = 85000,
			.trip_type = THERMAL_TRIP_PASSIVE,
			.upper = THERMAL_NO_LIMIT,
			.lower = THERMAL_NO_LIMIT,
			.hysteresis = 0,
		},
	},
};



static struct i2c_board_info ceres_i2c0_nct1008_board_info[] = {
	{
		I2C_BOARD_INFO("nct72", 0x4C),
		.platform_data = &ceres_nct1008_pdata,
		.irq = -1,
	}
};

#define CERES_TEMP_ALERT_GPIO	TEGRA_GPIO_PO1
static int ceres_nct1008_init(void)
{
	int ret = 0;

	tegra_add_cdev_trips(ceres_nct1008_pdata.trips,
			     &ceres_nct1008_pdata.num_trips);

	/* FIXME: enable irq when throttling is supported */
	ceres_i2c0_nct1008_board_info[0].irq =
		gpio_to_irq(CERES_TEMP_ALERT_GPIO);

	ret = gpio_request(CERES_TEMP_ALERT_GPIO, "temp_alert");
	if (ret < 0) {
		pr_err("%s: gpio_request failed\n", __func__);
		return ret;
	}

	ret = gpio_direction_input(CERES_TEMP_ALERT_GPIO);
	if (ret < 0) {
		pr_err("%s: set gpio to input failed\n", __func__);
		gpio_free(CERES_TEMP_ALERT_GPIO);
	}

	return ret;
}
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

	err = regulator_enable(vreg[IMX091_VREG_DVDD].vreg);
	if (unlikely(err))
		goto imx091_dvdd_fail;

	err = regulator_enable(vreg[IMX091_VREG_IOVDD].vreg);
	if (err)
		goto imx091_iovdd_fail;

	usleep_range(1, 2);
	gpio_set_value(CAM1_POWER_DWN_GPIO, 1);

	tegra_pinmux_config_table(&mclk_enable, 1);
	usleep_range(300, 310);

	return 1;

imx091_iovdd_fail:
	regulator_disable(vreg[IMX091_VREG_DVDD].vreg);

imx091_dvdd_fail:
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
	regulator_disable(vreg[IMX091_VREG_DVDD].vreg);
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
		.clock_multiplier	= 850000, /* value / 1,000,000 */
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
	.focuser_guid		= NVC_FOCUS_GUID(0),
	.torch_guid		= NVC_TORCH_GUID(0),
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

static unsigned max77387_estates[] = {1000, 800, 600, 400, 200, 100, 0};

static struct max77387_platform_data ceres_max77387_pdata = {
	.config		= {
		.led_mask		= 3,
		.flash_trigger_mode	= 1,
		/* use ONE-SHOOT flash mode - flash triggered at the
		 * raising edge of strobe or strobe signal.
		*/
		.flash_mode		= 1,
		.def_ftimer		= 0x24,
		.max_total_current_mA	= 1000,
		.max_peak_current_mA	= 600,
		.led_config[0]	= {
			.flash_torch_ratio	= 18100,
			.granularity		= 1000,
			.flash_levels		= 0,
			.lumi_levels	= NULL,
			},
		.led_config[1]	= {
			.flash_torch_ratio	= 18100,
			.granularity		= 1000,
			.flash_levels		= 0,
			.lumi_levels		= NULL,
			},
		},
#ifdef CONFIG_ARCH_TEGRA_11x_SOC
	.pinstate	= {
		.mask	= 1 << (CAM_FLASH_STROBE - TEGRA_GPIO_PBB0),
		.values	= 1 << (CAM_FLASH_STROBE - TEGRA_GPIO_PBB0),
		},
#else
	/* Needs to be validated and changed for t14x */
#endif
	.cfg		= NVC_CFG_NODEV,
	.dev_name	= "torch",
	.gpio_strobe	= CAM_FLASH_STROBE,
	.edpc_config	= {
		.states		= max77387_estates,
		.num_states	= ARRAY_SIZE(max77387_estates),
		.e0_index	= 3,
		.priority	= EDP_MAX_PRIO - 2,
		},
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
	{
		I2C_BOARD_INFO("max77387", 0x4A),
		.platform_data = &ceres_max77387_pdata,
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

/* MPU board file definition	*/
static struct mpu_platform_data mpu9150_gyro_data = {
	.int_config	= 0x10,
	.level_shifter	= 0,
	/* Located in board_[platformname].h */
	.orientation	= MPU_GYRO_ORIENTATION,
	.sec_slave_type	= SECONDARY_SLAVE_TYPE_COMPASS,
	.sec_slave_id	= COMPASS_ID_AK8975,
	.secondary_i2c_addr	= MPU_COMPASS_ADDR,
	.secondary_read_reg	= 0x06,
	.secondary_orientation	= MPU_COMPASS_ORIENTATION,
	.key		= {0x4E, 0xCC, 0x7E, 0xEB, 0xF6, 0x1E, 0x35, 0x22,
			   0x00, 0x34, 0x0D, 0x65, 0x32, 0xE9, 0x94, 0x89},
};

static struct i2c_board_info __initdata inv_mpu9150_i2c1_board_info[] = {
	{
		I2C_BOARD_INFO(MPU_GYRO_NAME, MPU_GYRO_ADDR),
		.platform_data = &mpu9150_gyro_data,
	},
};

static void mpuirq_init(void)
{
	int ret = 0;
	unsigned gyro_irq_gpio = MPU_GYRO_IRQ_GPIO;
	unsigned gyro_bus_num = MPU_GYRO_BUS_NUM;
	char *gyro_name = MPU_GYRO_NAME;

	pr_info("*** MPU START *** mpuirq_init...\n");

	ret = gpio_request(gyro_irq_gpio, gyro_name);

	if (ret < 0) {
		pr_err("%s: gpio_request failed %d\n", __func__, ret);
		return;
	}

	ret = gpio_direction_input(gyro_irq_gpio);
	if (ret < 0) {
		pr_err("%s: gpio_direction_input failed %d\n", __func__, ret);
		gpio_free(gyro_irq_gpio);
		return;
	}
	pr_info("*** MPU END *** mpuirq_init...\n");

	inv_mpu9150_i2c1_board_info[0].irq = gpio_to_irq(MPU_GYRO_IRQ_GPIO);
	i2c_register_board_info(gyro_bus_num, inv_mpu9150_i2c1_board_info,
		ARRAY_SIZE(inv_mpu9150_i2c1_board_info));
}

int __init ceres_sensors_init(void)
{
	int err;
	ceres_camera_init();
	mpuirq_init();

	err = ceres_nct1008_init();
	if (err)
		pr_err("%s: nct1008 init failed\n", __func__);
	else
		i2c_register_board_info(0, ceres_i2c0_nct1008_board_info,
				ARRAY_SIZE(ceres_i2c0_nct1008_board_info));

	i2c_register_board_info(0, ceres_i2c_board_info_max44005,
			ARRAY_SIZE(ceres_i2c_board_info_max44005));

	return 0;
}
