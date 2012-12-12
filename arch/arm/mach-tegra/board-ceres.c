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
#include <linux/memblock.h>
#include <linux/of_platform.h>
#include <linux/serial_8250.h>
#include <linux/tegra_uart.h>

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
	&tegra_hda_device,
};

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

static void __init tegra_ceres_init(void)
{
	tegra_clk_init_from_table(ceres_clk_init_table);
	tegra_enable_pinmux();
	ceres_uart_init();
	tegra_smmu_init();
	tegra_soc_device_init("ceres");
	ceres_keys_init();
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
