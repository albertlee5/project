/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <asm/arch/tegra2.h>
#include <asm/arch/sys_proto.h>

#include <asm/arch/clk_rst.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/uart.h>
#include "board.h"

#ifdef CONFIG_TEGRA2_MMC
#include <mmc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

const struct tegra2_sysinfo sysinfo = {
	CONFIG_TEGRA2_BOARD_STRING
};

/*
 * Routine: timer_init
 * Description: init the timestamp and lastinc value
 */
int timer_init(void)
{
	return 0;
}

static void enable_uart(enum periph_id pid)
{
	/* Assert UART reset and enable clock */
	reset_set_enable(pid, 1);
	clock_enable(pid);
	clock_ll_set_source(pid, 0);	/* UARTx_CLK_SRC = 00, PLLP_OUT0 */

	/* wait for 2us */
	udelay(2);

	/* De-assert reset to UART */
	reset_set_enable(pid, 0);
}

/*
 * Routine: clock_init_uart
 * Description: init the PLL and clock for the UART(s)
 */
static void clock_init_uart(void)
{
#if defined(CONFIG_TEGRA2_ENABLE_UARTA)
	enable_uart(PERIPH_ID_UART1);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTA */
#if defined(CONFIG_TEGRA2_ENABLE_UARTD)
	enable_uart(PERIPH_ID_UART4);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTD */
}

/*
 * Routine: pin_mux_uart
 * Description: setup the pin muxes/tristate values for the UART(s)
 */
static void pin_mux_uart(void)
{
#if defined(CONFIG_TEGRA2_ENABLE_UARTA)
	pinmux_set_func(PINGRP_IRRX, PMUX_FUNC_UARTA);
	pinmux_set_func(PINGRP_IRTX, PMUX_FUNC_UARTA);

	pinmux_tristate_disable(PINGRP_IRRX);
	pinmux_tristate_disable(PINGRP_IRTX);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTA */
#if defined(CONFIG_TEGRA2_ENABLE_UARTD)
	pinmux_set_func(PINGRP_GMC, PMUX_FUNC_UARTD);

	pinmux_tristate_disable(PINGRP_GMC);
#endif	/* CONFIG_TEGRA2_ENABLE_UARTD */
}

#ifdef CONFIG_TEGRA2_MMC
/*
 * Routine: pin_mux_mmc
 * Description: setup the pin muxes/tristate values for the SDMMC(s)
 */
static void pin_mux_mmc(void)
{
	/* SDMMC4: config 3, x8 on 2nd set of pins */
	pinmux_set_func(PINGRP_ATB, PMUX_FUNC_SDIO4);
	pinmux_set_func(PINGRP_GMA, PMUX_FUNC_SDIO4);
	pinmux_set_func(PINGRP_GME, PMUX_FUNC_SDIO4);

	pinmux_tristate_disable(PINGRP_ATB);
	pinmux_tristate_disable(PINGRP_GMA);
	pinmux_tristate_disable(PINGRP_GME);

	/* SDMMC3: SDIO3_CLK, SDIO3_CMD, SDIO3_DAT[3:0] */
	pinmux_set_func(PINGRP_SDB, PMUX_FUNC_SDIO3);
	pinmux_set_func(PINGRP_SDC, PMUX_FUNC_SDIO3);
	pinmux_set_func(PINGRP_SDD, PMUX_FUNC_SDIO3);

	pinmux_tristate_disable(PINGRP_SDC);
	pinmux_tristate_disable(PINGRP_SDD);
	pinmux_tristate_disable(PINGRP_SDB);
}
#endif

/*
 * Routine: board_init
 * Description: Early hardware init.
 */
int board_init(void)
{
	clock_init();
	clock_verify();

	/* boot param addr */
	gd->bd->bi_boot_params = (NV_PA_SDRAM_BASE + 0x100);

	return 0;
}

#ifdef CONFIG_TEGRA2_MMC
/* this is a weak define that we are overriding */
int board_mmc_init(bd_t *bd)
{
	debug("board_mmc_init called\n");
	/* Enable muxes, etc. for SDMMC controllers */
	pin_mux_mmc();
	gpio_config_mmc();

	debug("board_mmc_init: init eMMC\n");
	/* init dev 0, eMMC chip, with 4-bit bus */
	tegra2_mmc_init(0, 4);

	debug("board_mmc_init: init SD slot\n");
	/* init dev 1, SD slot, with 4-bit bus */
	tegra2_mmc_init(1, 4);

	return 0;
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	/* Initialize essential common plls */
	clock_early_init();

	/* Initialize UART clocks */
	clock_init_uart();

	/* Initialize periph pinmuxes */
	pin_mux_uart();

	/* Initialize periph GPIOs */
	gpio_config_uart();

	/* Init UART, scratch regs, and start CPU */
	tegra2_start();
	return 0;
}
#endif	/* EARLY_INIT */
