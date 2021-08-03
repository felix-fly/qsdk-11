/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __QTI_UART_CONSOLE_H__
#define __QTI_UART_CONSOLE_H__

#include <drivers/console.h>

#define QTI_CONSOLE_T_UART_BASE_OFF		(CONSOLE_T_DRVDATA)

#ifndef __ASSEMBLER__

typedef struct {
	console_t console;
	uintptr_t base;
} qti_console_uart_t;

int qti_console_uart_register(qti_console_uart_t * console,
			      uintptr_t uart_base_addr);

int qti_diag_register(qti_console_uart_t * console,
			      uintptr_t uart_base_addr);
#endif /* __ASSEMBLER__ */

#endif /* __QTI_UART_CONSOLE_H__ */
