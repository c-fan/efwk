//
//    Copyright (C) 2017 LGPL
//
//    This library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
//    USA

/*
 *---------------------------------------------------------------------------
 *| VERSION	| AUTHOR		| DATE			| NOTE							|
 *---------------------------------------------------------------------------
 *| 01		| Luo Renan	| 2017-06-28	|								|
 *---------------------------------------------------------------------------
 */


#ifndef FWK_BASIC_BASICTRACE_H_
#define FWK_BASIC_BASICTRACE_H_
#include <fwk/basic/types.h>
#include <fwk/basic/basictrace.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FWK_BASICTRACE_STR_LEN_MAX			256	/* basic trace string max lenght		*/
#define FWK_BASICTRACE_MODULE_NAME_LEN_MAX	50	/* basic trace module name max lenght	*/

/* basic trace level all and trace module all */
#define FWK_BASICTRACE_LEVEL_ALL	0xFFFFFFFF
#define FWK_BASICTRACE_MODULE_ALL	0xFFFFFFFF

/* basic trace module and level */
static uint32_t gu4FwkBasictraceModule	= FWK_BASICTRACE_MODULE_FWK;
static uint32_t gu4FwkBasictraceLevel 	= (FWK_BASICTRACE_LEVEL_DBG | FWK_BASICTRACE_LEVEL_ERR);
#define FWK_BASICTRACE_MODLUE	gu4FwkBasictraceModule
#define FWK_BAISCTRACE_LEVEL	gu4FwkBasictraceLevel

/* basic trace global enable/disable */
static uint32_t gu4FwkBasictaceEnable = 1;
static char gai1FwkBasictraceBuffer[FWK_BASICTRACE_MODULE_NAME_LEN_MAX] = {0};


#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_BASIC_BASICTRACE_H_ */
