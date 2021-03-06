/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifndef __NSSINFO_IPV4_REASM_H
#define __NSSINFO_IPV4_REASM_H

/**
 * @brief initialize IPV4_REASM module.
 *
 * @return 0 on success or -ve for failure
 */
int nssinfo_ipv4_reasm_init(void *data);
void nssinfo_ipv4_reasm_deinit(void *data);

#endif /* __NSSINFO_IPV4_REASM_H*/
