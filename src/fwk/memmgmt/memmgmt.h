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
 *| 01		| Luo Renan	| 2017-06-29	|								|
 *---------------------------------------------------------------------------
 */


#ifndef FWK_MEMMGMT_MEMMGMT_H
#define FWK_MEMMGMT_MEMMGMT_H
#include <fwk/basic/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FWK_MEMMGMT_LINK
{
	struct FWK_MEMMGMT_LINK *pNextFreeMem;		/*<< The next free memory in the list */
	uint32_t ui4MemSize;						/*<< The size of the free memory */
} fwk_memmgmtLink_t;

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_MEMMGMT_MEMMGMT_H */

