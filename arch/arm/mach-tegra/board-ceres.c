/*
 * arch/arm/mach-tegra/board-ceres.c
 *
 * Copyright (c) 2013, NVIDIA CORPORATION.  All rights reserved.
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/memblock.h>
#include <linux/of_platform.h>
#include <linux/serial_8250.h>
#include <linux/tegra_uart.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/spi/spi.h>
#include <linux/nfc/bcm2079x.h>
#include <linux/spi/rm31080a_ts.h>
#include <linux/spi-tegra.h>
#include <sound/max98090.h>
#include <asm/hardware/gic.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/isomgr.h>
#include <mach/tegra_bb.h>
#include <mach/tegra_bbc_proxy.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/tegra_fiq_debugger.h>

#include <mach/tegra_asoc_pdata.h>

#include "board.h"
#include "board-ceres.h"
#include "board-common.h"
#include "board-touch-raydium.h"
#include "clock.h"
#include "devices.h"
#include "common.h"

#ifdef CONFIG_ARCH_TEGRA_11x_SOC
#define CERES_BT_EN		TEGRA_GPIO_PQ6
#define CERES_BT_HOST_WAKE	TEGRA_GPIO_PU6
#define CERES_BT_EXT_WAKE	TEGRA_GPIO_PEE1
#define CERES_NFC_IRQ		TEGRA_GPIO_PW2
#define CERES_NFC_EN		TEGRA_GPIO_PU4
#define CERES_NFC_WAKE		TEGRA_GPIO_PX7
#else
#define CERES_BT_EN		TEGRA_GPIO_PM3
#define CERES_BT_HOST_WAKE	TEGRA_GPIO_PM2
#define CERES_BT_EXT_WAKE	TEGRA_GPIO_PM1
#define CERES_NFC_IRQ		TEGRA_GPIO_PM4
#define CERES_NFC_EN		TEGRA_GPIO_PI0
#define CERES_NFC_WAKE		TEGRA_GPIO_PM0
#endif

#ifdef CONFIG_BT_BLUESLEEP
static struct rfkill_gpio_platform_data ceres_bt_rfkill_pdata = {
	.name           = "bt_rfkill",
	.reset_gpio	= CERES_BT_EN,
	.type           = RFKILL_TYPE_BLUETOOTH,
};

static struct platform_device ceres_bt_rfkill_device = {
	.name = "rfkill_gpio",
	.id             = -1,
	.dev = {
		.platform_data = &ceres_bt_rfkill_pdata,
	},
};

static noinline void __init ceres_setup_bt_rfkill(void)
{
	platform_device_register(&ceres_bt_rfkill_device);
}

static struct resource ceres_bluesleep_resources[] = {
	[0] = {
		.name = "gpio_host_wake",
			.start  = CERES_BT_HOST_WAKE,
			.end    = CERES_BT_HOST_WAKE,
			.flags  = IORESOURCE_IO,
	},
	[1] = {
		.name = "gpio_ext_wake",
			.start  = CERES_BT_EXT_WAKE,
			.end    = CERES_BT_EXT_WAKE,
			.flags  = IORESOURCE_IO,
	},
	[2] = {
		.name = "host_wake",
			.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
};

static struct platform_device ceres_bluesleep_device = {
	.name           = "bluesleep",
	.id             = -1,
	.num_resources  = ARRAY_SIZE(ceres_bluesleep_resources),
	.resource       = ceres_bluesleep_resources,
};

static noinline void __init ceres_setup_bluesleep(void)
{
	ceres_bluesleep_resources[2].start =
		ceres_bluesleep_resources[2].end =
			gpio_to_irq(CERES_BT_HOST_WAKE);
	platform_device_register(&ceres_bluesleep_device);
	return;
}
#elif defined CONFIG_BLUEDROID_PM
static struct resource ceres_bluedroid_pm_resources[] = {
	[0] = {
		.name   = "shutdown_gpio",
		.start  = CERES_BT_EN,
		.end    = CERES_BT_EN,
		.flags  = IORESOURCE_IO,
	},

	[1] = {
		.name = "host_wake",
		.flags  = IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHEDGE,
	},
	[2] = {
		.name = "gpio_ext_wake",
		.start  = CERES_BT_EXT_WAKE,
		.end    = CERES_BT_EXT_WAKE,
		.flags  = IORESOURCE_IO,
	},
	[3] = {
		.name = "gpio_host_wake",
		.start  = CERES_BT_HOST_WAKE,
		.end    = CERES_BT_HOST_WAKE,
		.flags  = IORESOURCE_IO,
	},
};

static struct platform_device ceres_bluedroid_pm_device = {
	.name = "bluedroid_pm",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ceres_bluedroid_pm_resources),
	.resource       = ceres_bluedroid_pm_resources,
};

static noinline void __init ceres_setup_bluedroid_pm(void)
{
	ceres_bluedroid_pm_resources[1].start =
		ceres_bluedroid_pm_resources[1].end =
					gpio_to_irq(CERES_BT_HOST_WAKE);
	platform_device_register(&ceres_bluedroid_pm_device);
}
#endif

static struct bcm2079x_platform_data nfc_pdata = {
	.irq_gpio = CERES_NFC_IRQ,
	.en_gpio = CERES_NFC_EN,
	.wake_gpio = CERES_NFC_WAKE,
	};

static struct i2c_board_info __initdata ceres_i2c_bus3_board_info[] = {
	{
		I2C_BOARD_INFO("bcm2079x-i2c", 0x77),
		.platform_data = &nfc_pdata,
	},
};

static struct resource tegra_rtc_resources[] = {
	[0] = {
		.start = TEGRA_RTC_BASE,
		.end = TEGRA_RTC_BASE + TEGRA_RTC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = INT_RTC,
		.end = INT_RTC,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device tegra_rtc_device = {
	.name = "tegra_rtc",
	.id   = -1,
	.resource = tegra_rtc_resources,
	.num_resources = ARRAY_SIZE(tegra_rtc_resources),
};

static struct max98090_eq_cfg max98090_eq_cfg[] = {
};

static struct max98090_pdata ceres_max98090_pdata = {
	/* Headphone Detection */
	.irq = TEGRA_GPIO_HP_DET,

	/* Equalizer Configuration */
	.eq_cfg = max98090_eq_cfg,
	.eq_cfgcnt = ARRAY_SIZE(max98090_eq_cfg),

	/* Microphone Configuration */
	.digmic_left_mode = 1,
	.digmic_right_mode = 1,
};

static struct i2c_board_info __initdata max98090_board_info = {
	I2C_BOARD_INFO("max98090", 0x10),
	.platform_data = &ceres_max98090_pdata,
	.irq		= TEGRA_GPIO_CDC_IRQ,
};

static struct tegra_asoc_platform_data ceres_audio_max98090_pdata = {
	.gpio_spkr_en		= TEGRA_GPIO_SPKR_EN,
	.gpio_hp_det		= TEGRA_GPIO_HP_DET,
	.gpio_hp_mute		= -1,
	.gpio_int_mic_en	= TEGRA_GPIO_INT_MIC_EN,
	.gpio_ext_mic_en	= TEGRA_GPIO_EXT_MIC_EN,
	.gpio_ldo1_en		= TEGRA_GPIO_LDO1_EN,
	.i2s_param[HIFI_CODEC]	= {
		.audio_port_id	= 0,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
		.sample_size	= 16,
		.channels       = 2,
	},
	.i2s_param[BASEBAND]	= {
		.audio_port_id	= 1,
		.is_i2s_master	= 1,
		.i2s_mode	= TEGRA_DAIFMT_I2S,
		.sample_size	= 16,
		.rate		= 16000,
		.channels	= 1,
		.bit_clk        = 768000,
	},
};

static struct platform_device ceres_audio_max98090_device = {
	.name	= "tegra-snd-max98090",
	.id	= 0,
	.dev	= {
		.platform_data = &ceres_audio_max98090_pdata,
	},
};

#if defined(CONFIG_TEGRA_BASEBAND)
static struct tegra_bb_platform_data ceres_tegra_bb_data;

static struct platform_device ceres_tegra_bb_device = {
	.name = "tegra_bb",
	.id = 0,
	.dev = {
		.platform_data = &ceres_tegra_bb_data,
	},
};
#endif

static struct platform_device *ceres_spi_devices[] __initdata = {
#ifdef CONFIG_ARCH_TEGRA_11x_SOC
	&tegra11_spi_device4,
#else
	&tegra11_spi_device2,
	&tegra11_spi_device3,
#endif
};

struct spi_clk_parent spi_parent_clk_ceres[] = {
	[0] = {.name = "pll_p"},
#ifndef CONFIG_TEGRA_PLLM_RESTRICTED
	[1] = {.name = "pll_m"},
	[2] = {.name = "clk_m"},
#else
	[1] = {.name = "clk_m"},
#endif
};

static struct tegra_spi_platform_data ceres_spi_pdata = {
	.is_dma_based           = true,
	.max_dma_buffer         = 16 * 1024,
	.is_clkon_always        = false,
	.max_rate               = 25000000,
};

static void __init ceres_spi_init(void)
{
	int i;
	struct clk *c;
	struct board_info board_info, display_board_info;

	tegra_get_board_info(&board_info);
	tegra_get_display_board_info(&display_board_info);

	for (i = 0; i < ARRAY_SIZE(spi_parent_clk_ceres); ++i) {
		c = tegra_get_clock_by_name(spi_parent_clk_ceres[i].name);
		if (IS_ERR_OR_NULL(c)) {
			pr_err("Not able to get the clock for %s\n",
			       spi_parent_clk_ceres[i].name);
			continue;
		}
		spi_parent_clk_ceres[i].parent_clk = c;
		spi_parent_clk_ceres[i].fixed_clk_rate = clk_get_rate(c);
	}
	ceres_spi_pdata.parent_clk_list = spi_parent_clk_ceres;
	ceres_spi_pdata.parent_clk_count = ARRAY_SIZE(spi_parent_clk_ceres);

#ifdef CONFIG_ARCH_TEGRA_11x_SOC
	tegra11_spi_device4.dev.platform_data = &ceres_spi_pdata;
#else
	tegra11_spi_device2.dev.platform_data = &ceres_spi_pdata;
	tegra11_spi_device3.dev.platform_data = &ceres_spi_pdata;
#endif
	platform_add_devices(ceres_spi_devices, ARRAY_SIZE(ceres_spi_devices));
}

#define BBC_BOOT_EDP_MAX 0
static unsigned int bbc_boot_edp_states[] = {500};
static struct edp_client bbc_boot_edp_client = {
	.name = "bbc_boot",
	.states = bbc_boot_edp_states,
	.num_states = ARRAY_SIZE(bbc_boot_edp_states),
	.e0_index = BBC_BOOT_EDP_MAX,
	.priority = EDP_MAX_PRIO,
};

static struct tegra_bbc_proxy_platform_data bbc_proxy_pdata = {
	.modem_boot_edp_client = &bbc_boot_edp_client,
	.edp_manager_name = NULL, /* FIXME when edp manager present */
	.i_breach_ppm = 500000,
	.i_thresh_3g_adjperiod = 10000,
	.i_thresh_lte_adjperiod = 10000,
};

static struct platform_device tegra_bbc_proxy_device = {
	.name = "tegra_bbc_proxy",
	.id = -1,
	.dev = {
		.platform_data = &bbc_proxy_pdata,
	},
};


static struct platform_device *ceres_audio_devices[] __initdata = {
	&tegra_ahub_device,
	&tegra_pcm_device,
	&tegra_dam_device0,
	&tegra_dam_device1,
	&tegra_dam_device2,
	&tegra_i2s_device0,
	&tegra_i2s_device1,
	&tegra_i2s_device2,
	&tegra_i2s_device3,
	&tegra_i2s_device4,
	&spdif_dit_device,
	&tegra_spdif_device,
	&bluetooth_dit_device,
	&baseband_dit_device,
#if defined(CONFIG_SND_HDA_PLATFORM_NVIDIA_TEGRA)
	&tegra_hda_device,
#endif
	&ceres_audio_max98090_device,
};

static struct platform_device *ceres_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_rtc_device,
	&tegra_udc_device,
#if defined(CONFIG_TEGRA_AVP)
	&tegra_avp_device,
#endif
#if defined(CONFIG_CRYPTO_DEV_TEGRA_SE)
	&tegra11_se_device,
#endif
	&tegra_bbc_proxy_device,
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	&tegra_mipi_bif_device,
#endif
};

static struct i2c_board_info __initdata max97236_board_info = {
	I2C_BOARD_INFO("max97236", 0x40),
};

static void ceres_audio_init(void)
{
	i2c_register_board_info(5, &max97236_board_info, 1);
	i2c_register_board_info(5, &max98090_board_info, 1);

	platform_add_devices(ceres_audio_devices,
			ARRAY_SIZE(ceres_audio_devices));
}

#ifdef CONFIG_USB_SUPPORT
static struct tegra_usb_platform_data tegra_udc_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.builtin_host_disabled = true,
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	.support_pmu_vbus = true,
#endif
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_DEVICE,
	.u_data.dev = {
		.vbus_pmu_irq = 0,
		.vbus_gpio = -1,
		.charging_supported = false,
		.remote_wakeup_supported = false,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 8,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_platform_data tegra_ehci1_utmi_pdata = {
	.port_otg = true,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.builtin_host_disabled = true,
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	.support_pmu_vbus = true,
#endif
	.phy_intf = TEGRA_USB_PHY_INTF_UTMI,
	.op_mode = TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
	.u_cfg.utmi = {
		.hssync_start_delay = 0,
		.elastic_limit = 16,
		.idle_wait_delay = 17,
		.term_range_adj = 6,
		.xcvr_setup = 15,
		.xcvr_lsfslew = 2,
		.xcvr_lsrslew = 2,
		.xcvr_setup_offset = 0,
		.xcvr_use_fuses = 1,
	},
};

static struct tegra_usb_otg_data tegra_otg_pdata = {
	.ehci_device = &tegra_ehci1_device,
	.ehci_pdata = &tegra_ehci1_utmi_pdata,
#ifdef CONFIG_ARCH_TEGRA_14x_SOC
	.extcon_dev_name = "max77660-charger-extcon",
#endif
};

static struct tegra_usb_platform_data tegra_ehci2_hsic_smsc_hub_pdata = {
	.port_otg = false,
	.has_hostpc = true,
	.unaligned_dma_buf_supported = false,
	.phy_intf = TEGRA_USB_PHY_INTF_HSIC,
	.op_mode	= TEGRA_USB_OPMODE_HOST,
	.u_data.host = {
		.vbus_gpio = -1,
		.hot_plug = false,
		.remote_wakeup_supported = true,
		.power_off_on_suspend = true,
	},
};

static void ceres_usb_init(void)
{
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	/* Setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;
}

static void ceres_modem_init(void)
{
	int modem_id = tegra_get_modem_id();

	if (TEGRA_BB_HSIC_HUB == modem_id) {
		tegra_ehci2_device.dev.platform_data =
			&tegra_ehci2_hsic_smsc_hub_pdata;
		platform_device_register(&tegra_ehci2_device);
	}
}

#else
static void ceres_usb_init(void) { }
static void ceres_modem_init(void) { }
#endif

static __initdata struct tegra_clk_init_table ceres_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
	{ "vi_sensor",	"pll_p",	150000000,	false},
	{ "vi_sensor2",	"pll_p",	150000000,	false},
	{ "cilab",	"pll_p",	102000000,	false},
	{ "cile",	"pll_p",	102000000,	false},
	{ "i2c1",	"pll_p",	3200000,	false},
	{ "i2c2",	"pll_p",	3200000,	false},
	{ "i2c6",	"pll_p",	3200000,	false},
	{ "hda",	"pll_p",	108000000,	false},
	{ "hda2codec_2x", "pll_p",	48000000,	false},
	{ "i2s0",	"pll_a_out0",	0,		false},
	{ "i2s1",	"pll_a_out0",	0,		false},
	{ "i2s2",	"pll_a_out0",	0,		false},
	{ "i2s3",	"pll_a_out0",	0,		false},
	{ "i2s4",	"pll_a_out0",	0,		false},
	{ "d_audio",	"clk_m",	12000000,	false},
	{ "dam0",	"clk_m",	12000000,	false},
	{ "dam1",	"clk_m",	12000000,	false},
	{ "dam2",	"clk_m",	12000000,	false},
	{ "audio0",	"i2s0_sync",	0,		false},
	{ "audio1",	"i2s1_sync",	0,		false},
	{ "audio2",	"i2s2_sync",	0,		false},
	{ "audio3",	"i2s3_sync",	0,		false},
	{ NULL,		NULL,		0,		0},
};

static struct platform_device *ceres_uart_devices[] __initdata = {
	&tegra_uarta_device,
	&tegra_uartb_device,
	&tegra_uartc_device,
	&tegra_uartd_device,
};
static struct uart_clk_parent uart_parent_clk[] = {
	[0] = {.name = "clk_m"},
	[1] = {.name = "pll_p"},
#ifndef CONFIG_TEGRA_PLLM_RESTRICTED
	[2] = {.name = "pll_m"},
#endif
};

static struct tegra_uart_platform_data ceres_uart_pdata;
static struct tegra_uart_platform_data ceres_loopback_uart_pdata;

static void __init uart_debug_init(void)
{
	int debug_port_id;

	debug_port_id = uart_console_debug_init(3);
	if (debug_port_id < 0)
		return;
	ceres_uart_devices[debug_port_id] = uart_console_debug_device;
}

static void __init ceres_uart_init(void)
{
	struct clk *c;
	int i;

	for (i = 0; i < ARRAY_SIZE(uart_parent_clk); ++i) {
		c = tegra_get_clock_by_name(uart_parent_clk[i].name);
		if (IS_ERR_OR_NULL(c)) {
			pr_err("Not able to get the clock for %s\n",
						uart_parent_clk[i].name);
			continue;
		}
		uart_parent_clk[i].parent_clk = c;
		uart_parent_clk[i].fixed_clk_rate = clk_get_rate(c);
	}
	ceres_uart_pdata.parent_clk_list = uart_parent_clk;
	ceres_uart_pdata.parent_clk_count = ARRAY_SIZE(uart_parent_clk);
	ceres_loopback_uart_pdata.parent_clk_list = uart_parent_clk;
	ceres_loopback_uart_pdata.parent_clk_count =
						ARRAY_SIZE(uart_parent_clk);
	ceres_loopback_uart_pdata.is_loopback = true;
	tegra_uarta_device.dev.platform_data = &ceres_uart_pdata;
	tegra_uartb_device.dev.platform_data = &ceres_uart_pdata;
	tegra_uartc_device.dev.platform_data = &ceres_uart_pdata;
	tegra_uartd_device.dev.platform_data = &ceres_uart_pdata;

	/* Register low speed only if it is selected */
	if (!is_tegra_debug_uartport_hs())
		uart_debug_init();

	platform_add_devices(ceres_uart_devices,
				ARRAY_SIZE(ceres_uart_devices));
}

static struct tegra_i2c_platform_data ceres_i2c1_platform_data = {
	.bus_clk_rate	= 100000,
	.scl_gpio	= TEGRA_GPIO_I2C1_SCL,
	.sda_gpio	= TEGRA_GPIO_I2C1_SDA,
	.is_clkon_always = true,
};

static struct tegra_i2c_platform_data ceres_i2c2_platform_data = {
	.bus_clk_rate	= 100000,
	.scl_gpio	= -1,
	.sda_gpio	= -1,
};

static struct tegra_i2c_platform_data ceres_i2c3_platform_data = {
	.bus_clk_rate	= 100000,
	.scl_gpio	= -1,
	.sda_gpio	= -1,
};

static struct tegra_i2c_platform_data ceres_i2c4_platform_data = {
	.bus_clk_rate	= 100000,
	.scl_gpio	= -1,
	.sda_gpio	= -1,
};

static struct tegra_i2c_platform_data ceres_i2c5_platform_data = {
	.bus_clk_rate	= 400000,
	.scl_gpio	= -1,
	.sda_gpio	= -1,
};

static __maybe_unused struct tegra_i2c_platform_data ceres_i2c6_platform_data = {
	.bus_clk_rate	= 400000,
	.scl_gpio	= TEGRA_GPIO_I2C5_SCL,
	.sda_gpio	= TEGRA_GPIO_I2C5_SDA,
};

static void ceres_i2c_init(void)
{
#ifdef CONFIG_ARCH_TEGRA_11x_SOC

	tegra11_i2c_device1.dev.platform_data = &ceres_i2c1_platform_data;
	tegra11_i2c_device2.dev.platform_data = &ceres_i2c2_platform_data;
	tegra11_i2c_device3.dev.platform_data = &ceres_i2c3_platform_data;
	tegra11_i2c_device4.dev.platform_data = &ceres_i2c4_platform_data;
	tegra11_i2c_device5.dev.platform_data = &ceres_i2c5_platform_data;

	platform_device_register(&tegra11_i2c_device5);
	platform_device_register(&tegra11_i2c_device4);
	platform_device_register(&tegra11_i2c_device3);
	platform_device_register(&tegra11_i2c_device2);
	platform_device_register(&tegra11_i2c_device1);

#else

	tegra14_i2c_device1.dev.platform_data = &ceres_i2c1_platform_data;
	tegra14_i2c_device2.dev.platform_data = &ceres_i2c2_platform_data;
	tegra14_i2c_device3.dev.platform_data = &ceres_i2c3_platform_data;
	tegra14_i2c_device4.dev.platform_data = &ceres_i2c4_platform_data;
	tegra14_i2c_device5.dev.platform_data = &ceres_i2c5_platform_data;
	tegra14_i2c_device6.dev.platform_data = &ceres_i2c6_platform_data;

	platform_device_register(&tegra14_i2c_device6);
	platform_device_register(&tegra14_i2c_device5);
	platform_device_register(&tegra14_i2c_device4);
	platform_device_register(&tegra14_i2c_device3);
	platform_device_register(&tegra14_i2c_device2);
	platform_device_register(&tegra14_i2c_device1);

#endif

	ceres_i2c_bus3_board_info[0].irq = gpio_to_irq(CERES_NFC_IRQ);
	i2c_register_board_info(1, ceres_i2c_bus3_board_info, 1);
}

#ifdef CONFIG_ARCH_TEGRA_11x_SOC
static __initdata struct tegra_clk_init_table touch_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "extern2",	"pll_p",	41000000,	false},
	{ "clk_out_2",	"extern2",	40800000,	false},
	{ NULL,		NULL,		0,		0},
};

struct rm_spi_ts_platform_data rm31080ts_ceres_data = {
	.gpio_reset = 0,
	.config = 0,
	.platform_id = RM_PLATFORM_P005,
	.name_of_clock = "clk_out_2",
	.name_of_clock_con = "extern2",
};

static struct tegra_spi_device_controller_data dev_cdata = {
	.rx_clk_tap_delay = 0,
	.tx_clk_tap_delay = 0,
};

struct spi_board_info rm31080a_ceres_spi_board[1] = {
	{
		.modalias = "rm_ts_spidev",
		.bus_num = 3,
		.chip_select = 2,
		.max_speed_hz = 12 * 1000 * 1000,
		.mode = SPI_MODE_0,
		.controller_data = &dev_cdata,
		.platform_data = &rm31080ts_ceres_data,
	},
};

static int __init ceres_touch_init(void)
{
	tegra_clk_init_from_table(touch_clk_init_table);
	rm31080a_ceres_spi_board[0].irq =
		gpio_to_irq(TOUCH_GPIO_IRQ_RAYDIUM_SPI);
	touch_init_raydium(TOUCH_GPIO_IRQ_RAYDIUM_SPI,
				TOUCH_GPIO_RST_RAYDIUM_SPI,
				&rm31080ts_ceres_data,
				&rm31080a_ceres_spi_board[0],
				ARRAY_SIZE(rm31080a_ceres_spi_board));
	return 0;
}

#else
static __initdata struct tegra_clk_init_table touch_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "vi_sensor",	"pll_p",	41000000,	true},
	{ "csus",	NULL,		00000000,	true},
	{ NULL,		NULL,		0,		0},
};

struct rm_spi_ts_platform_data rm31080ts_ceres_data = {
	.gpio_reset = 0,
	.config = 0,
	.platform_id = RM_PLATFORM_P005,
	/* Clocks are defined in tegraXX_clocks.c:  CLK_DUPLICATE(...)» */
	.name_of_clock = "touch_clk",
	.name_of_clock_con = "e1680_ts_clk_con",
};

static struct tegra_spi_device_controller_data dev_cdata = {
	.rx_clk_tap_delay = 0,
	.tx_clk_tap_delay = 0,
};

struct spi_board_info rm31080a_ceres_spi_board[1] = {
	{
		.modalias = "rm_ts_spidev",
		.bus_num = 2,
		.chip_select = 0,
		.max_speed_hz = 9 * 1000 * 1000,
		.mode = SPI_MODE_0,
		.controller_data = &dev_cdata,
		.platform_data = &rm31080ts_ceres_data,
	},
};

static int __init ceres_touch_init(void)
{
	tegra_clk_init_from_table(touch_clk_init_table);
	rm31080a_ceres_spi_board[0].irq =
		gpio_to_irq(TOUCH_GPIO_IRQ_RAYDIUM_SPI);
	touch_init_raydium(TOUCH_GPIO_IRQ_RAYDIUM_SPI,
				TOUCH_GPIO_RST_RAYDIUM_SPI,
				&rm31080ts_ceres_data,
				&rm31080a_ceres_spi_board[0],
				ARRAY_SIZE(rm31080a_ceres_spi_board));
	return 0;
}
#endif

#if defined(CONFIG_TEGRA_BASEBAND)
static void ceres_tegra_bb_init(void)
{
	pr_info("%s: registering tegra bb\n", __func__);
	ceres_tegra_bb_data.bb_irq = INT_BB2AP_INT0;
	platform_device_register(&ceres_tegra_bb_device);
}
#endif

static void __init tegra_ceres_init(void)
{
	tegra_clk_init_from_table(ceres_clk_init_table);
	tegra_smmu_init();
	tegra_enable_pinmux();
	ceres_pinmux_init();
	ceres_i2c_init();
	ceres_spi_init();
	ceres_uart_init();
	ceres_usb_init();
	tegra_soc_device_init("ceres");
	ceres_keys_init();
	ceres_regulator_init();
	ceres_suspend_init();
	ceres_touch_init();
	ceres_sdhci_init();
	isomgr_init();
	platform_add_devices(ceres_devices, ARRAY_SIZE(ceres_devices));
#ifdef CONFIG_ARCH_TEGRA_11x_SOC
	tegra_serial_debug_init(TEGRA_UARTD_BASE, INT_WDT_CPU, NULL, -1, -1);
#else
	tegra_serial_debug_init(TEGRA_UARTA_BASE, INT_WDT_CPU, NULL, -1, -1);
#endif
	ceres_panel_init();
	ceres_edp_init();
	ceres_sensors_init();
	ceres_modem_init();
#if defined(CONFIG_TEGRA_BASEBAND)
	ceres_tegra_bb_init();
#endif
	tegra_register_fuse();
/* Disabled for T148 bringup 
	ceres_soctherm_init();
*/
	ceres_emc_init();
#ifdef CONFIG_BT_BLUESLEEP
	ceres_setup_bluesleep();
	ceres_setup_bt_rfkill();
#elif defined CONFIG_BLUEDROID_PM
	ceres_setup_bluedroid_pm();
#endif
	ceres_audio_init();
	ceres_pmon_init();
}

static void __init tegra_ceres_dt_init(void)
{
	of_platform_populate(NULL,
		of_default_bus_match_table, NULL, &platform_bus);

	tegra_ceres_init();
}

static void __init tegra_ceres_reserve(void)
{
#if defined(CONFIG_NVMAP_CONVERT_CARVEOUT_TO_IOVMM)
	/* for PANEL_5_SHARP_1080p: 1920*1080*4*2 = 16588800 bytes */
	tegra_reserve(0, SZ_16M, SZ_4M);
#else
	tegra_reserve(SZ_128M, SZ_16M, SZ_4M);
#endif
}

static const char * const ceres_dt_board_compat[] = {
	"nvidia,ceres",
	NULL
};

DT_MACHINE_START(CERES, "Ceres")
	.atag_offset	= 0x100,
	.soc			= &tegra_soc_desc,
	.map_io			= tegra_map_common_io,
	.reserve		= tegra_ceres_reserve,
#ifdef CONFIG_ARCH_TEGRA_11x_SOC
	.init_early		= tegra11x_init_early,
#else
	.init_early		= tegra14x_init_early,
#endif
	.init_irq		= tegra_dt_init_irq,
	.handle_irq		= gic_handle_irq,
	.timer			= &tegra_timer,
	.init_machine	= tegra_ceres_dt_init,
	.restart		= tegra_assert_system_reset,
	.dt_compat		= ceres_dt_board_compat,
MACHINE_END
