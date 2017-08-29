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

#include <pthread.h> 
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>   
#include "../../../include/fwk/cli/cli.h"



struct cli_def *cli;

int efwk_cli_idle_timeout();
int efwk_cli_thread_create();

struct cli_def * efwk_cli_init()
{
    cli = cli_init();

    cli_set_banner(cli, "Connect to server"); // greeting info when user connects
    cli_set_hostname(cli, "Cli"); // cli hostname
    cli_telnet_protocol(cli, 1);

    // disable some default command
    cli_unregister_command(cli,"logout");
    cli_unregister_command(cli,"quit");
    cli_unregister_command(cli,"history");
    cli_unregister_command(cli,"enable");
    cli_unregister_command(cli,"disable");
    cli_unregister_command(cli,"configure");

    // authentication with add username / password combinations via cli_allow_user() function
    // if not set, means no authentication required.
    cli_allow_user(cli,"cli","cli");

    // set idle timeout 
    cli_set_idle_timeout_callback(cli, 60, efwk_cli_idle_timeout); 

    return cli;
}


int efwk_cli_idle_timeout()
{
    cli_print(cli, "Idle timeout");
    return CLI_QUIT;
}

void *efwk_cli_thread_func(void *arg)
{
	int s, x;
    struct sockaddr_in addr;
    int on = 1;
	
    signal(SIGCHLD, SIG_IGN);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        return NULL;
    }
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(EFWK_CLI_PORT);
    if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0)
    {
        perror("bind");
        return NULL;
    }

    if (listen(s, 50) < 0)
    {
        perror("listen");
        return NULL;
    }

    printf("Listening on port %d\n", EFWK_CLI_PORT);
    while ((x = accept(s, NULL, 0)))
    {
        int pid = fork();
        if (pid < 0)
        {
            perror("fork");
            return NULL;
        }

        // parent
        if (pid > 0)
        {
            socklen_t len = sizeof(addr);
            if (getpeername(x, (struct sockaddr *) &addr, &len) >= 0)
                printf(" * accepted connection from %s\n", inet_ntoa(addr.sin_addr));

            close(x);
            continue;
        }

        // child
        close(s);
        cli_loop(cli, x);
        exit(0);
    }
	
    cli_done(cli);
	
	printf("end .... \r\n");

    return NULL;
}


int efwk_cli_start()
{
    pthread_t thr;
    if(pthread_create(&thr,NULL,efwk_cli_thread_func,NULL)!=0)
    {
        printf("create thread failed!\n");
        return CLI_ERROR;
    }

    return CLI_OK;
}

int efwk_cli_register_command(const char *command, 
    int (*callback)(struct cli_def *cli, const char *, char **, int),
    const char *help)
{
    struct cli_command *c = NULL;

    c = cli_register_command(cli, NULL, command, callback, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, help);
    if(c == NULL)
        return CLI_ERROR;
    else
        return CLI_OK;
}
