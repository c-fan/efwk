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
 *---------------------------------------------------------------------------
 *| VERSION | AUTHOR            | DATE       | NOTE                         |
 *---------------------------------------------------------------------------
 *| 01      | Fan Chunquan      | 2017-05-30 |                              |
 *---------------------------------------------------------------------------
 */

#ifndef FWK_TASKEXT_EVENTEXT_H_
#define FWK_TASKEXT_EVENTEXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * we choose to manage remote event explicitly
 * as developers shall be aware that it's more costing.
 */
int fwk_task_sendRemoteEvent(fwk_sysID_t sid, fwk_taskID_t dest,
		fwk_eventType_t eventType, void * data, int datalen, int timeout);

int fwk_task_sendRemoteOverullingEvent(fwk_sysID_t sid, fwk_taskID_t dest,
		fwk_eventType_t eventType, void * data, int datalen, int timeout);

#ifdef __cplusplus
} /* extern C */
#endif

#endif /* FWK_TASKEXT_EVENTEXT_H_ */
