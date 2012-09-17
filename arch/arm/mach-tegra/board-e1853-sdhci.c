/*
 * arch/arm/mach-tegra/board-e1853-sdhci.c
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
 *
 */

#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>
#include <linux/i2c.h>

#include "gpio-names.h"
#include "board.h"
#include "board-e1853.h"
#include "devices.h"
#include <mach/ioexpander.h>

#define IO_EXPANDER_ADDR		(0x75)
#define WIFI_POWER_ENABLE_BIT_POS	(IO_EXP_PIN_3)
#define WIFI_RESET_BIT_POS		(IO_EXP_PIN_4)

static void (*wifi_status_cb) (int card_present, void *dev_id);
static void *wifi_status_cb_devid;
static int e1853_wifi_status_register(void (*callback) (int, void *), void *);
static int e1853_wifi_reset(int on);
static int e1853_wifi_power(int on);
static int e1853_wifi_set_carddetect(int val);

static struct wifi_platform_data e1853_wifi_control = {
	.set_power = e1853_wifi_power,
	.set_reset = e1853_wifi_reset,
	.set_carddetect = e1853_wifi_set_carddetect,
};

static struct resource wifi_resource[] = {
	[0] = {
		.name = "bcm4329_wlan_irq",
		.flags = IORESOURCE_IRQ
			| IORESOURCE_IRQ_HIGHLEVEL
			| IORESOURCE_IRQ_SHAREABLE,
	},
};

static struct platform_device broadcom_wifi_device = {
	.name = "bcm4329_wlan",
	.id = 1,
	.num_resources = 1,
	.resource = wifi_resource,
	.dev = {
		.platform_data = &e1853_wifi_control,
		},
};

#ifdef CONFIG_MMC_EMBEDDED_SDIO
static struct embedded_sdio_data embedded_sdio_data2 = {
	.cccr = {
		 .sdio_vsn = 2,
		 .multi_block = 1,
		 .low_speed = 0,
		 .wide_bus = 0,
		 .high_power = 1,
		 .high_speed = 1,
		 },
	.cis = {
		.vendor = 0x02d0,
		.device = 0x4329,
		},
};
#endif

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data1 = {
	.mmc_data = {
		     .register_status_notify = e1853_wifi_status_register,
#ifdef CONFIG_MMC_EMBEDDED_SDIO
		     .embedded_sdio = &embedded_sdio_data2,
#endif
		     .built_in = 0,
		     .ocr_mask = MMC_OCR_1V8_MASK,
		     },
#ifndef CONFIG_MMC_EMBEDDED_SDIO
	.pm_flags = MMC_PM_KEEP_POWER,
#endif
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.tap_delay = 0x0F,
	.ddr_clk_limit = 41000000,
	.is_8bit = false,
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data2 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.is_8bit = 1,
	.mmc_data = {
		     .built_in = 1,
		     }
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data3 = {
	.cd_gpio = TEGRA_GPIO_PN6,
	.wp_gpio = TEGRA_GPIO_PD4,
	.power_gpio = TEGRA_GPIO_PN7,
	.is_8bit = false,
	.mmc_data = {
		.ocr_mask = MMC_OCR_2V8_MASK,
	}
};

static struct tegra_sdhci_platform_data tegra_sdhci_platform_data4 = {
	.cd_gpio = -1,
	.wp_gpio = -1,
	.power_gpio = -1,
	.is_8bit = true,
};

static int
e1853_wifi_status_register(void (*callback) (int card_present, void *dev_id),
			   void *dev_id)
{
	if (wifi_status_cb)
		return -EBUSY;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int e1853_wifi_set_carddetect(int val)
{

	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		pr_warning("%s: Nobody to notify\n", __func__);

	return 0;
}

static void e1853_wifi_power_enable(void)
{
	struct i2c_adapter *adapter = NULL;
	struct i2c_board_info info = { {0} };
	struct i2c_client *client = NULL;
	struct i2c_msg msg;
	u8 cmd_buf[2];

	/* Program the serializer */
	adapter = i2c_get_adapter(1);
	if (!adapter)
		printk(KERN_WARNING "%s: adapter is null\n", __func__);
	else {
		info.addr = IO_EXPANDER_ADDR;
		client = i2c_new_device(adapter, &info);
		i2c_put_adapter(adapter);
		if (!client)
			printk(KERN_WARNING "%s: client is null\n", __func__);
		else {

			/* Set data for pin PO3 in output port register */
			/* RMW register contents */
			cmd_buf[0] = IO_EXP_OUTPUT_PORT_REG_0;
			msg.addr = IO_EXPANDER_ADDR;
			msg.flags = 0;
			msg.len = 1;
			msg.buf = &cmd_buf[0];
			if (i2c_transfer(client->adapter, &msg, 1) < 0)
				goto i2c_fail;

			msg.addr = IO_EXPANDER_ADDR;
			msg.flags = I2C_M_RD;
			msg.len = 1;
			msg.buf = &cmd_buf[1];
			if (i2c_transfer(client->adapter, &msg, 1) < 0)
				goto i2c_fail;

			/* Set value for pin PO3 and PO4 */
			cmd_buf[0] = IO_EXP_OUTPUT_PORT_REG_0;
			cmd_buf[1] |= (1 << WIFI_POWER_ENABLE_BIT_POS);
			cmd_buf[1] |= (1 << WIFI_RESET_BIT_POS);
			msg.addr = IO_EXPANDER_ADDR;
			msg.flags = 0;
			msg.len = 2;
			msg.buf = &cmd_buf[0];
			if (i2c_transfer(client->adapter, &msg, 1) < 0)
				goto i2c_fail;

			/* RMW register contents */
			cmd_buf[0] = IO_EXP_CONFIG_REG_0;
			msg.addr = IO_EXPANDER_ADDR;
			msg.flags = 0;
			msg.len = 1;
			msg.buf = &cmd_buf[0];
			if (i2c_transfer(client->adapter, &msg, 1) < 0)
				goto i2c_fail;

			msg.addr = IO_EXPANDER_ADDR;
			msg.flags = I2C_M_RD;
			msg.len = 1;
			msg.buf = &cmd_buf[1];
			if (i2c_transfer(client->adapter, &msg, 1) < 0)
				goto i2c_fail;

			/* Set bit for pin PO3 and PO4 */
			cmd_buf[0] = IO_EXP_CONFIG_REG_0;
			cmd_buf[1] &=  (~(1 << WIFI_POWER_ENABLE_BIT_POS) &
					~(1 << WIFI_RESET_BIT_POS));
			msg.addr = IO_EXPANDER_ADDR;
			msg.flags = 0;
			msg.len = 2;
			msg.buf = &cmd_buf[0];
			if (i2c_transfer(client->adapter, &msg, 1) < 0)
				goto i2c_fail;

			goto done;
		}
	}

i2c_fail:
	printk(KERN_ERR "%s: I2C transaction failed\n", __func__);
done:
	if (client)
		i2c_unregister_device(client);
}

static int e1853_wifi_power(int on)
{
	if (on)
		e1853_wifi_power_enable();

	return 0;
}

static int e1853_wifi_reset(int on)
{
	/*
	 * FIXME: Implement wifi reset
	 */
	return 0;
}

static int __init e1853_wifi_init(void)
{
	platform_device_register(&broadcom_wifi_device);
	return 0;
}

int __init e1853_sdhci_init(void)
{
	tegra_sdhci_device1.dev.platform_data = &tegra_sdhci_platform_data1;
	tegra_sdhci_device2.dev.platform_data = &tegra_sdhci_platform_data2;
	tegra_sdhci_device3.dev.platform_data = &tegra_sdhci_platform_data3;
	tegra_sdhci_device4.dev.platform_data = &tegra_sdhci_platform_data4;

	platform_device_register(&tegra_sdhci_device1);
	platform_device_register(&tegra_sdhci_device2);
	platform_device_register(&tegra_sdhci_device3);
	platform_device_register(&tegra_sdhci_device4);

	e1853_wifi_init();
	return 0;
}
