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


#ifndef FWK_MEMMGMT_H
#define FWK_MEMMGMT_H
#include <fwk/basic/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FWK_MEMMGMT_TOTAL_MEM_SIZE		(( uint32_t )128*1024*1024)	/*total 128 Mbytes memory */

#define FWK_MEMMGMT_MIN_MEM_SIZE		(( uint32_t )128)			/*minimum 128 bytes memory */

#define FWK_MEMMGMT_MIN_MEM_SIZE_MASK	(( uint32_t )0x007FU)		/*minimum 128 bytes memory mask*/

#define FWK_MEMMGMT_BYTE_ALIGNMENT		(( uint32_t )8)				/* 8 bytes alignment */

#define FWK_MEMMGMT_BYTE_ALIGNMENT_MASK (( uint32_t )0x0007U )		/* bytes alignment mask */

/*memory malloc: input parameter memory size you want to allocate */
extern void *fwk_memmgmt_malloc( uint32_t u4Size );

/*memory free: input parameter memory address you want to free */
extern void fwk_memmgmt_free( void *pMemaddr );

/*memory set: input parameter destination address, value and size */
extern void* fwk_memmgmt_set(void *pDest, int32_t i4Value, uint32_t u4Size);

/*memory copy: input parameter destination address, source address and size */
extern void *fwk_memmgmt_cpy(void *pDest, const void *pSrc, uint32_t u4Size);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_MEMMGMT_H */
