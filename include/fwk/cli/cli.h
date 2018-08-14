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
 *| 01		| JiangBo              | 2017-08-29	       |								|
 *---------------------------------------------------------------------------
 */

#ifndef _FWK_CLI_H__
#define _FWK_CLI_H__

#include "libcli.h"

#define EFWK_CLI_PORT                8000   // cli connection port number

#ifdef __cplusplus
extern "C" {
#endif

/*
  * Initialize cli environment and configure default settings
  *     1. set default greeting info
  *     2. set defalut hostname
  *     3. disable some defalut commands
  *     4. add default authentication user name and password: username: 'cli', password:'cli'
  *     5. set default idle time out as 60 seconds
  * Return:
  *     a poniter of cli_def struct
  */
struct cli_def *efwk_cli_init(void);

/*
  * register command to cli
  * input:
  *     command: command name
  *     callback: call back function name
  *     help: description of the command
  * Return:
  *     CLI_ERROR when failed
  *     CLI_OK when successful
  */
int efwk_cli_register_command(struct cli_def * cli, const char *command,
    int (*callback)(struct cli_def *cli, const char *, char **, int), const char *help);

/*
  * Start cli as an independent task
  */
int efwk_cli_start(void);

struct cli_user_cmd {
    const char* name;
    int (*func)(struct cli_def *cli, const char *, char *[], int);
    const char* tips;
};
extern struct cli_user_cmd gCliUserCmd[];

#ifdef __cplusplus
}
#endif

#endif
