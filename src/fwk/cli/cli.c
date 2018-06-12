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
#include "fwk/cli/cli.h"
#include  <fwk/task/task.h>

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
    cli_set_idle_timeout_callback(cli, 180, efwk_cli_idle_timeout); 

    return cli;
}

typedef struct {
  struct cli_def* cli;
  int socket;
}cli_arg_wrap_t;

void *efwk_cli_loop_wrapper(void *arg)
{
    cli_arg_wrap_t* pArg = (cli_arg_wrap_t*)arg;
    struct cli_def *cli = pArg->cli;
    int socket = pArg->socket;
    cli_loop(cli, socket);
    close(socket);
    cli_done(cli);
    fwk_terminateTask();
    return NULL;
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
#if 0
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
#else
        // parent
            socklen_t len = sizeof(addr);
            if (getpeername(x, (struct sockaddr *) &addr, &len) >= 0)
                printf(" * accepted connection from %s\n", inet_ntoa(addr.sin_addr));

        // child
        struct cli_def *newCli;
        if (!(newCli = calloc(sizeof(struct cli_def), 1))) break;
    
        memcpy(newCli, cli,  sizeof(*cli));
        if (!(newCli->buffer = calloc(newCli->buf_size, 1)))
        {
            free(newCli);
            break;
        }

        static cli_arg_wrap_t args;
        args.cli = newCli;
        args.socket = x;
        fwk_taskID_t tid;
        fwk_taskRes_t resource = {1024*1024, 0, 0, 0};
#if 1
        int rc = fwk_createNormalTask("cliClt", &tid, efwk_cli_loop_wrapper, &args, 1, resource, 1);
        printf("CLI Client socket %i tid: %"fwk_addr_f" with rc %i\n", x, (fwk_addr_t)tid, rc);
#else
    pthread_t thr;
    int rc = pthread_create(&thr, NULL, efwk_cli_loop_wrapper, &args);
    if (rc != 0)
    {
        printf("create thread failed (%i)!\n", rc);
    }
#endif
#endif
    }
	
    cli_done(cli);
	
	printf("end .... \r\n");

    return NULL;
}


int efwk_cli_start()
{
#if 0
    pthread_t thr;
    if(pthread_create(&thr,NULL,efwk_cli_thread_func,NULL)!=0)
    {
        printf("create thread failed!\n");
        return CLI_ERROR;
    }

    return CLI_OK;
#elif 0
  fwk_taskID_t tid;
  fwk_taskAttr_t tAttr = {"cliSvr", NULL, efwk_cli_thread_func, NULL, SCHED_FIFO, 50, {1024*1024, 0, 0, 0}, 1, 0, -1};
  int rc = fwk_createTask(&tAttr, &tid);
  printf("CLI Server tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);
  return rc;
#else
  fwk_taskID_t tid;
  fwk_taskRes_t resource = {1024*1024, 0, 0, 0};
  int rc = fwk_createNormalTask("cliSvr", &tid, efwk_cli_thread_func, NULL, 1, resource, 1);
  printf("CLI Server tid: %"fwk_addr_f"\n", (fwk_addr_t)tid);
  return rc;
#endif
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
