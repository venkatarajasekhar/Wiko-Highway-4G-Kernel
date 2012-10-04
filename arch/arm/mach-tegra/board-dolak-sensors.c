/*
 * arch/arm/mach-tegra/board-dolak-sensors.c
 *
 * Copyright (c) 2011-2012 NVIDIA CORPORATION, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of NVIDIA CORPORATION nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <media/imx091.h>
#include "board.h"
#include "board-dolak.h"

static struct board_info board_info;

static int dolak_camera_init(void)
{
	return 0;
}

static int dolak_imx091_power_on(void)
{
	return 0;
}

static int dolak_imx091_power_off(void)
{
	return 0;
}

struct imx091_platform_data dolak_imx091_data = {
	.power_on = dolak_imx091_power_on,
	.power_off = dolak_imx091_power_off,
};

static struct i2c_board_info dolak_i2c_board_info[] = {
	{
		I2C_BOARD_INFO("imx091", 0x36),
		.platform_data = &dolak_imx091_data,
	},
};

int __init dolak_sensors_init(void)
{
	i2c_register_board_info(5, dolak_i2c_board_info,
		ARRAY_SIZE(dolak_i2c_board_info));

	return 0;
}
