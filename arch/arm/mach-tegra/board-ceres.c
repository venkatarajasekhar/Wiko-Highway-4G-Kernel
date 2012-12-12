/*
 * arch/arm/mach-tegra/board-ceres.c
 *
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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
#include <linux/gpio.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/memblock.h>
#include <linux/of_platform.h>
#include <linux/serial_8250.h>
#include <linux/tegra_uart.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/spi/spi.h>
#include <linux/spi-tegra.h>
#include <asm/hardware/gic.h>

#include <mach/iomap.h>
#include <mach/irqs.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/tegra_fiq_debugger.h>

#include "board.h"
#include "board-ceres.h"
#include "board-common.h"
#include "clock.h"
#include "devices.h"
#include "common.h"


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

static struct platform_device tegra_camera = {
	.name = "tegra_camera",
	.id = -1,
};

static struct platform_device *ceres_spi_devices[] __initdata = {
	&tegra11_spi_device4,
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
	.is_dma_based           = false,
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
	tegra11_spi_device4.dev.platform_data = &ceres_spi_pdata;
	platform_add_devices(ceres_spi_devices, ARRAY_SIZE(ceres_spi_devices));
}


static struct platform_device *ceres_devices[] __initdata = {
	&tegra_pmu_device,
	&tegra_rtc_device,
	&tegra_udc_device,
#if defined(CONFIG_TEGRA_IOVMM_SMMU) || defined(CONFIG_TEGRA_IOMMU_SMMU)
	&tegra_smmu_device,
#endif
	&tegra_camera,
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
	&tegra_spdif_device,
	&spdif_dit_device,
	&bluetooth_dit_device,
	&baseband_dit_device,
#if defined(CONFIG_CRYPTO_DEV_TEGRA_SE)
	&tegra11_se_device,
#endif
	&tegra_hda_device,
};

#ifdef CONFIG_USB_SUPPORT
static struct tegra_usb_platform_data tegra_udc_pdata = {
	.port_otg = true,
	.has_hostpc = true,
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
};

static void ceres_usb_init(void)
{
	tegra_otg_device.dev.platform_data = &tegra_otg_pdata;
	platform_device_register(&tegra_otg_device);

	/* Setup the udc platform data */
	tegra_udc_device.dev.platform_data = &tegra_udc_pdata;
}

#else
static void ceres_usb_init(void) { }
#endif

static __initdata struct tegra_clk_init_table ceres_clk_init_table[] = {
	/* name		parent		rate		enabled */
	{ "pll_m",	NULL,		0,		false},
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
	.adapter_nr	= 0,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
};

static struct tegra_i2c_platform_data ceres_i2c2_platform_data = {
	.adapter_nr	= 1,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
};

static struct tegra_i2c_platform_data ceres_i2c3_platform_data = {
	.adapter_nr	= 2,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
};

static struct tegra_i2c_platform_data ceres_i2c4_platform_data = {
	.adapter_nr	= 3,
	.bus_count	= 1,
	.bus_clk_rate	= { 100000, 0 },
};

static struct tegra_i2c_platform_data ceres_i2c5_platform_data = {
	.adapter_nr	= 4,
	.bus_count	= 1,
	.bus_clk_rate	= { 400000, 0 },
};

static void ceres_i2c_init(void)
{
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
}

static void __init tegra_ceres_init(void)
{
	tegra_clk_init_from_table(ceres_clk_init_table);
	tegra_enable_pinmux();
	ceres_i2c_init();
	ceres_spi_init();
	ceres_uart_init();
	tegra_smmu_init();
	ceres_usb_init();
	tegra_soc_device_init("ceres");
	ceres_keys_init();
	ceres_regulator_init();
	ceres_suspend_init();
	ceres_sdhci_init();
	platform_add_devices(ceres_devices, ARRAY_SIZE(ceres_devices));
	tegra_serial_debug_init(TEGRA_UARTD_BASE, INT_WDT_CPU, NULL, -1, -1);
}

static void __init tegra_ceres_dt_init(void)
{
	tegra_ceres_init();

	of_platform_populate(NULL,
		of_default_bus_match_table, NULL, NULL);
}

static void __init tegra_ceres_reserve(void)
{
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
