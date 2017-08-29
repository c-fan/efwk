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


int cmd_test(struct cli_def *cli, const char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "called %s with \"%s\"", __func__, command);
    cli_print(cli, "%d arguments:", argc);
    for (i = 0; i < argc; i++)
        cli_print(cli, "        %s", argv[i]);

    printf("cmd_test called\n");

    return CLI_OK;
}

int cmd_test_2(struct cli_def *cli, const char *command, char *argv[], int argc)
{
    int i;
    cli_print(cli, "execute command: %s ",command);

    for (i = 0; i < argc; i++)
        cli_print(cli, "        %s", argv[i]);

    printf("cmd_test_2 was called with command: %s ",command);
    for (i = 0; i < argc; i++)
        printf("%s ", argv[i]);
    printf("\n");

    return CLI_OK;
}


int main()
{
	struct cli_def *cli;

    cli = efwk_cli_init();
    efwk_cli_register_command("test",cmd_test,"My registe test function");
    efwk_cli_register_command("test_2",cmd_test_2,"My registe test function_2");
    efwk_cli_start();

    while(1)
    {
    }
}
