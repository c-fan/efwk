//
//    http://www.gnu.org/licenses/lgpl-2.1.html
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
 * Define some basic types.
 *---------------------------------------------------------------------------
 *| VERSION | AUTHOR            | DATE       | NOTE                         |
 *---------------------------------------------------------------------------
 *| 01      | Fan Chunquan      | 2017-05-30 |                              |
 *---------------------------------------------------------------------------
 */

#ifndef FWK_BASIC_TYPES_H
#define FWK_BASIC_TYPES_H

#include <inttypes.h>

typedef int bool_t;
#define TRUE ((bool_t) 1)
#define FALSE ((bool_t) 0) 

typedef unsigned long ubase_t;

#define FOREVER -1
#if defined(MFH) || defined(APB) || defined(HOST)
typedef uint64_t fwk_addr_t;
#define fwk_addr_f PRIu64
#elif defined(DMC)
typedef uint32_t fwk_addr_t;
#define fwk_addr_f PRIu32
#else
#error "Unsupported running time..."
#endif

#endif /* FWK_BASIC_TYPES_H */
