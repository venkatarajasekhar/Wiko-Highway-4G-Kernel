/*
 * arch/arm/mach-tegra/board-ceres-kbc.c
 * Keys configuration for Nvidia tegra3 ceres platform.
 *
 * Copyright (C) 2012 NVIDIA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <mach/io.h>
#include <mach/iomap.h>
#include <mach/kbc.h>
#include <linux/gpio.h>
#include <linux/gpio_keys.h>
#include <linux/mfd/palmas.h>

#include "tegra-board-id.h"
#include "board.h"
#include "board-ceres.h"
#include "devices.h"

/*
static const u32 kbd_keymap[] = {
	KEY(0, 0, KEY_POWER),
	KEY(0, 1, KEY_HOME),

	KEY(1, 0, KEY_RESERVED),
	KEY(1, 1, KEY_VOLUMEDOWN),

	KEY(2, 0, KEY_CAMERA),
	KEY(2, 1, KEY_VOLUMEUP),
	KEY(2, 2, KEY_2),
};

static const struct matrix_keymap_data keymap_data = {
	.keymap		= kbd_keymap,
	.keymap_size	= ARRAY_SIZE(kbd_keymap),
};

static struct tegra_kbc_wake_key ceres_wake_cfg[] = {
	[0] = {
		.row = 0,
		.col = 0,
	},
};

static struct tegra_kbc_platform_data ceres_kbc_platform_data = {
	.debounce_cnt = 20 * 32,
	.repeat_cnt = 1,
	.scan_count = 30,
	.wakeup = true,
	.keymap_data = &keymap_data,
	.wake_cnt = 2,
	.wake_cfg = &ceres_wake_cfg[0],
	.wakeup_key = KEY_POWER,
#ifdef CONFIG_ANDROID
	.disable_ev_rep = true,
#endif
};
*/
/*
#define GPIO_IKEY(_id, _irq, _iswake, _deb)	\
	{					\
		.code = _id,			\
		.gpio = -1,			\
		.irq = _irq,			\
		.desc = #_id,			\
		.type = EV_KEY,			\
		.wakeup = _iswake,		\
		.debounce_interval = _deb,	\
	}
*/
#define GPIO_KEY(_id, _gpio, _iswake)		\
	{					\
		.code = _id,			\
		.gpio = TEGRA_GPIO_##_gpio,	\
		.active_low = 1,		\
		.desc = #_id,			\
		.type = EV_KEY,			\
		.wakeup = _iswake,		\
		.debounce_interval = 10,	\
	}

static struct gpio_keys_button ceres_int_keys[] = {
#ifdef CONFIG_ARCH_TEGRA_11x_SOC
	[0] = GPIO_KEY(KEY_HOME, PR0, 0),
	[1] = GPIO_KEY(KEY_MENU, PR1, 0),
	[2] = GPIO_KEY(KEY_BACK, PR2, 0),
	[3] = GPIO_KEY(KEY_POWER, PQ0, 1),
	[4] = GPIO_KEY(KEY_VOLUMEUP, PQ1, 0),
	[5] = GPIO_KEY(KEY_VOLUMEDOWN, PQ2, 0),

#else
	[0] = GPIO_KEY(KEY_HOME, PV1, 0),
	[1] = GPIO_KEY(KEY_MENU, PV2, 0),
	[2] = GPIO_KEY(KEY_BACK, PV3, 0),
	[3] = GPIO_KEY(KEY_POWER, PV4, 1),
	[4] = GPIO_KEY(KEY_VOLUMEUP, PV5, 0),
	[5] = GPIO_KEY(KEY_VOLUMEDOWN, PV6, 0),

#endif
};

static struct gpio_keys_platform_data ceres_int_keys_pdata = {
	.buttons	= ceres_int_keys,
	.nbuttons	= ARRAY_SIZE(ceres_int_keys),
};

static struct platform_device ceres_int_keys_device = {
	.name	= "gpio-keys",
	.id	= 0,
	.dev	= {
		.platform_data  = &ceres_int_keys_pdata,
	},
};

int __init ceres_keys_init(void)
{
	platform_device_register(&ceres_int_keys_device);

	return 0;
}

