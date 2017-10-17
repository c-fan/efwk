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

#ifndef FWK_ERROR_CODE_H
#define FWK_ERROR_CODE_H

/*
 * Error code
 */
typedef enum
{
	FWK_E_PARAM = -1, /* invalid parameter */
	FWK_E_INTERNAL = -2, /* internal error */

	FWK_E_QUEUE_FULL = -3, /* queue is full */
	FWK_E_QUEUE_NODATA = -4, /* no data in the queue */
	FWK_E_QUEUE_BUFOVERFLOW = -5, /* buffer size too small */

	FWK_E_EVENT_FULL = -6, /* event queue full */
	FWK_E_EVENT_NODATA = -7, /* no event in the queue */

	FWK_E_NO_MEM = -8, /* No dynamic memory to alloc */
	FWK_E_RESOURCE = -9, /* lack resource */
	FWK_E_EMPTY = -10, /* request resource exhaust */
	FWK_E_FULL = -11, /* no space to accept new request */

} fwk_errCode_t;

#endif /* FWK_ERROR_CODE_H */
