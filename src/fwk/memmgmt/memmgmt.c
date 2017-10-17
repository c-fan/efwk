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

 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>
#include <fwk/basic/types.h>
#include <fwk/basic/basictrace.h>
#include <fwk/memmgmt/memmgmt.h>
#include "memmgmt.h"

static char gau1fwkmemmgmtPool[ FWK_MEMMGMT_TOTAL_MEM_SIZE ];

static uint32_t gu4fwkmemmgmtFreeBytesRemaining = 0U;

static uint32_t gu4fwkmemmgmtMinFreeBytesRemaining = 0U;

static uint32_t gu4fwkmemmgmtAllocatedBit = 0;

static fwk_memmgmtLink_t gfwkmemmgmtStart, *gpfwkmemmgmtEnd = NULL;

static const uint32_t gu4fwkmemmgmtStructSize	= ( sizeof( fwk_memmgmtLink_t ) + ( ( uint32_t ) ( FWK_MEMMGMT_BYTE_ALIGNMENT - 1 ) ) ) & ~( ( uint32_t ) FWK_MEMMGMT_BYTE_ALIGNMENT_MASK );

static sem_t gfwkmemmgmtSemId;

uint32_t fwk_memmgmt_getFreeSize( void )
{
	return gu4fwkmemmgmtFreeBytesRemaining;
}

uint32_t fwk_memmgmt_getMinUsedSize( void )
{
	return gu4fwkmemmgmtMinFreeBytesRemaining;
}

static void fwk_memmgmt_insert( fwk_memmgmtLink_t *pMemToInsert )
{
	fwk_memmgmtLink_t *pxIterator;
	char *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &gfwkmemmgmtStart; pxIterator->pNextFreeMem < pMemToInsert; pxIterator = pxIterator->pNextFreeMem )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( char * ) pxIterator;
	if( ( puc + pxIterator->ui4MemSize ) == ( char * ) pMemToInsert )
	{
		pxIterator->ui4MemSize += pMemToInsert->ui4MemSize;
		pMemToInsert = pxIterator;
	}


	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( char * ) pMemToInsert;
	if( ( puc + pMemToInsert->ui4MemSize ) == ( char * ) pxIterator->pNextFreeMem )
	{
		if( pxIterator->pNextFreeMem != gpfwkmemmgmtEnd )
		{
			/* Form one big block from the two blocks. */
			pMemToInsert->ui4MemSize += pxIterator->pNextFreeMem->ui4MemSize;
			pMemToInsert->pNextFreeMem = pxIterator->pNextFreeMem->pNextFreeMem;
		}
		else
		{
			pMemToInsert->pNextFreeMem = gpfwkmemmgmtEnd;
		}
	}
	else
	{
		pMemToInsert->pNextFreeMem = pxIterator->pNextFreeMem;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pNextFreeMem pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pMemToInsert )
	{
		pxIterator->pNextFreeMem = pMemToInsert;
	}
}


static void fwk_memmgmt_init( void )
{
	int32_t i4res;
	fwk_addr_t ui4Address;
	char *pi1AlignedMem;
	uint32_t ui4MemTotalSize = FWK_MEMMGMT_TOTAL_MEM_SIZE;
	fwk_memmgmtLink_t *pFirstFreeMem;
	
	fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_INF,"F:%s memory init start.\n",__FUNCTION__);

	i4res = sem_init(&gfwkmemmgmtSemId, 0, 1);
	if (i4res != 0)
	{	  
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"Semaphore gfwkmemmgmtSemId initialization failed.\n");
		return;
	}

	ui4Address = (fwk_addr_t) gau1fwkmemmgmtPool;
	if( ( ui4Address & FWK_MEMMGMT_BYTE_ALIGNMENT_MASK ) != 0 )
	{
		ui4Address += ( FWK_MEMMGMT_BYTE_ALIGNMENT - 1 );
		ui4Address &= ~( (fwk_addr_t) FWK_MEMMGMT_BYTE_ALIGNMENT_MASK );
		ui4MemTotalSize -= ui4Address - (fwk_addr_t) gau1fwkmemmgmtPool;
	}
	pi1AlignedMem = ( char * ) ui4Address;
	
	fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_INF,"ui4Address:0x%x,total size:0x%x.\n",ui4Address,ui4MemTotalSize);

	gfwkmemmgmtStart.pNextFreeMem = ( void * ) pi1AlignedMem;
	gfwkmemmgmtStart.ui4MemSize = ( uint32_t ) 0;

	ui4Address = ( (fwk_addr_t) pi1AlignedMem ) + ui4MemTotalSize;
	ui4Address -= gu4fwkmemmgmtStructSize;
	ui4Address &= ~( (fwk_addr_t) FWK_MEMMGMT_BYTE_ALIGNMENT_MASK );
	gpfwkmemmgmtEnd = ( void * ) ui4Address;
	gpfwkmemmgmtEnd->ui4MemSize = 0;
	gpfwkmemmgmtEnd->pNextFreeMem = NULL;

	fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_INF,"gfwkmemmgmtStart pNextFreeMem:0x%x,gpfwkmemmgmtEnd:0x%x.\n",gfwkmemmgmtStart.pNextFreeMem,gpfwkmemmgmtEnd);

	pFirstFreeMem = ( void * ) pi1AlignedMem;
	pFirstFreeMem->ui4MemSize = ui4Address - (fwk_addr_t) pFirstFreeMem;
	pFirstFreeMem->pNextFreeMem = gpfwkmemmgmtEnd;

	gu4fwkmemmgmtMinFreeBytesRemaining = pFirstFreeMem->ui4MemSize;
	gu4fwkmemmgmtFreeBytesRemaining = pFirstFreeMem->ui4MemSize;

	/* Work out the position of the top bit in a u32_t variable. */
	gu4fwkmemmgmtAllocatedBit = ( ( uint32_t ) 1 ) << ( ( sizeof( uint32_t ) * 8 ) - 1 );

	fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_INF,"gu4fwkmemmgmtAllocatedBit:0x%x,gu4fwkmemmgmtFreeBytesRemaining:0x%x.\n",gu4fwkmemmgmtAllocatedBit,gu4fwkmemmgmtFreeBytesRemaining);
}



void *fwk_memmgmt_malloc( uint32_t u4Size )
{
	fwk_memmgmtLink_t *pMem, *pPreviousMem, *pNewMem;
	void *pvReturn = NULL;

	fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_INF,"F:%s size:0x%x.\n",__FUNCTION__,u4Size);

	if( gpfwkmemmgmtEnd == NULL )
	{
		fwk_memmgmt_init();
	}

	sem_wait(&gfwkmemmgmtSemId);	
	{
		
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_INF,"size:0x%x,gu4fwkmemmgmtAllocatedBit:0x%x.\n",u4Size,gu4fwkmemmgmtAllocatedBit);
		if( ( u4Size & gu4fwkmemmgmtAllocatedBit ) == 0 )
		{
			if( u4Size > 0 )
			{
				u4Size += gu4fwkmemmgmtStructSize;
				/* minimum alloc and need to alignment */
				if( ( u4Size & FWK_MEMMGMT_MIN_MEM_SIZE_MASK ) != 0x00 )
				{
					u4Size += ( FWK_MEMMGMT_MIN_MEM_SIZE - ( u4Size & FWK_MEMMGMT_MIN_MEM_SIZE_MASK ) );
					if( ( u4Size & FWK_MEMMGMT_MIN_MEM_SIZE_MASK ) != 0 )
					{
						fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"F:%s size(0x%x) error.\n",__FUNCTION__,u4Size);
						return NULL;
					}	
				}
			}

			if( ( u4Size > 0 ) && ( u4Size <= gu4fwkmemmgmtFreeBytesRemaining ) )
			{
				pPreviousMem = &gfwkmemmgmtStart;
				pMem = gfwkmemmgmtStart.pNextFreeMem;
				while( ( pMem->ui4MemSize < u4Size ) && ( pMem->pNextFreeMem != NULL ) )
				{
					pPreviousMem = pMem;
					pMem = pMem->pNextFreeMem;
				}

				if( pMem != gpfwkmemmgmtEnd )
				{
					pvReturn = ( void * ) ( ( ( char * ) pPreviousMem->pNextFreeMem ) + gu4fwkmemmgmtStructSize );
					pPreviousMem->pNextFreeMem = pMem->pNextFreeMem;
					if( ( pMem->ui4MemSize - u4Size ) > FWK_MEMMGMT_MIN_MEM_SIZE )
					{
						pNewMem = ( void * ) ( ( ( char * ) pMem ) + u4Size );
						pNewMem->ui4MemSize = pMem->ui4MemSize - u4Size;
						pMem->ui4MemSize = u4Size;

						/* Insert the new memory into the list of free memory. */
						fwk_memmgmt_insert( pNewMem );
					}

					gu4fwkmemmgmtFreeBytesRemaining -= pMem->ui4MemSize;

					if( gu4fwkmemmgmtFreeBytesRemaining < gu4fwkmemmgmtMinFreeBytesRemaining )
					{
						gu4fwkmemmgmtMinFreeBytesRemaining = gu4fwkmemmgmtFreeBytesRemaining;
					}

					pMem->ui4MemSize |= gu4fwkmemmgmtAllocatedBit;
					pMem->pNextFreeMem = NULL;
				}

			}

		}
	}
	sem_post(&gfwkmemmgmtSemId);

	return pvReturn;
}
 
void fwk_memmgmt_free( void *pMemaddr )
{
	char *pi1MemAddr = ( char * ) pMemaddr;
	fwk_memmgmtLink_t *pMem;

	if( pMemaddr != NULL )
	{
		pi1MemAddr -= gu4fwkmemmgmtStructSize;
		pMem = ( void * ) pi1MemAddr;

		if( ( pMem->ui4MemSize & gu4fwkmemmgmtAllocatedBit ) == 0 )
		{
			fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"F:%s memory has free:0x%x.\n",__FUNCTION__,pMemaddr);
			return;
		}
		if( pMem->pNextFreeMem != NULL )
		{
			fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"F:%s pNextFreeMem is NULL:0x%x.\n",__FUNCTION__,pMemaddr);
			return;
		}

		if( ( pMem->ui4MemSize & gu4fwkmemmgmtAllocatedBit ) != 0 )
		{
			if( pMem->pNextFreeMem == NULL )
			{
				pMem->ui4MemSize &= ~gu4fwkmemmgmtAllocatedBit;
				sem_wait(&gfwkmemmgmtSemId);
				{
					gu4fwkmemmgmtFreeBytesRemaining += pMem->ui4MemSize;
					fwk_memmgmt_insert( ( ( fwk_memmgmtLink_t * ) pMem ) );
				}
				sem_post(&gfwkmemmgmtSemId);
			}
			return;
		}
	}
	fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"F:%s pMemaddr is NULL.\n",__FUNCTION__);
	return;
}

void* fwk_memmgmt_set(void *pDest, int32_t i4Value, uint32_t u4Size)
{
	if(pDest == NULL || u4Size <= 0) 
	{
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"F:%s param is error.\n",__FUNCTION__);
		return NULL;
	}
    char* pDestStart = (char*)pDest;
    while(u4Size--)
        *pDestStart++ = i4Value;
    return pDestStart;
}

void *fwk_memmgmt_cpy(void *pDest, const void *pSrc, uint32_t u4Size)
{  
	if(pDest == NULL || pSrc == NULL || u4Size <= 0) 
	{
		fwk_basictrace_print(FWK_BASICTRACE_MODULE_FWK,FWK_BASICTRACE_LEVEL_ERR,"F:%s param is error.\n",__FUNCTION__);
		return NULL;
	}
	char *pSrcTemp = (char *)pSrc;
	char *pDstTemp = (char *)pDest;  
	while(u4Size-->0)
		*pDstTemp++ = *pSrcTemp++;  
	return pDest;
} 

