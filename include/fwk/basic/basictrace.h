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


#ifndef FWK_BASICTRACE_H_
#define FWK_BASICTRACE_H_
#include <fwk/basic/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* bitmask for basic trace module are 32 bits: bit 1 to bit 32 */
#define FWK_BASICTRACE_MODULE_FWK	(1 << 0)    	/* framework module		*/

/* bitmask for basic trace level are 32 bits: bit 1 to bit 32 */
#define FWK_BASICTRACE_LEVEL_DBG 	(1 << 0)		/* level debug			*/
#define FWK_BASICTRACE_LEVEL_ERR	(1 << 1)		/* level errors			*/
#define FWK_BASICTRACE_LEVEL_WRN	(1 << 2)		/* level warning		*/
#define FWK_BASICTRACE_LEVEL_INF	(1 << 3)		/* level informations	*/

/* basic trace api. */
extern void fwk_basictrace_print(const uint32_t u4Mod, const uint32_t u4Level, const char *pi1Fmt, ...);
extern void fwk_basictrace_setModule(const uint32_t u4Mod, const bool_t Enable);
extern void fwk_basictrace_setLevel(const uint32_t u4Level, const bool_t Enable);
extern void fwk_basictrace_show(void);


#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_BASICTRACE_H_ */
