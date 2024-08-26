#include "monitor/expr.h"
#include "monitor/monitor.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char *rl_gets()
{
    static char *line_read = NULL;

    if (line_read)
    {
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(nemu) ");

    if (line_read && *line_read)
    {
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char *args)
{
    cpu_exec(-1);
    return 0;
}

static int cmd_q(char *args)
{
    return -1;
}

static int cmd_help(char *args);

static int cmd_si()
{
    char *steps = strtok(NULL, " ");
    int step = 0;
    int i;
    if (steps == NULL)
    {
        cpu_exec(1);
        return 0;
    }
    sscanf(steps, "%d", &step);
    if (step <= 0)
    {
        printf("Invalid steps\n");
        return 0;
    }
    for (i = 0; i < step; i++)
    {
        cpu_exec(1);
    }
    return 0;
}
static int cmd_info()
{
    char *arg = strtok(NULL, " ");
    if (strcmp(arg, "r") == 0)
    {
        for (int i = 0; i < 8; i++)
        {
            printf("%s ", regsl[i]);
            printf("0x%08x  %d\n", cpu.gpr[i]._32, cpu.gpr[i]._32);
        }
        printf("eip: 0x%x\n", cpu.eip);
        return 0;
    }
    printf("Unknown command '%s'\n", arg);
    return 0;
}

static int cmd_x(){
	char *args[2];
	args[0] = strtok(NULL, " ");
	args[1] = strtok(NULL, " ");
	int step=0;
	swaddr_t address;
	sscanf(args[0], "%d", &step);
	sscanf(args[1], "%x", &address);

	int j=0;
	for(int i=0;i<step;i++){
		if(j%4==0){
			printf("0x%x: ", address);
		}
		printf("0x%08x ", swaddr_read(address, 4));

		address+=4;
		j++;
		if(j%4==0){
			printf("\n");
		}
	}
	printf("\n");
	return 0;
}
static struct
{
    char *name;
    char *description;
    int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "One Step", cmd_si},
    {"info", "Print the information of program", cmd_info},
	{"x", "Scan the memory", cmd_x}
    /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args)
{
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL)
    {
        /* no argument given */
        for (i = 0; i < NR_CMD; i++)
        {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    }
    else
    {
        for (i = 0; i < NR_CMD; i++)
        {
            if (strcmp(arg, cmd_table[i].name) == 0)
            {
                printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void ui_mainloop()
{
    while (1)
    {
        char *str = rl_gets();
        char *str_end = str + strlen(str);

        /* extract the first token as the command */
        char *cmd = strtok(str, " ");
        if (cmd == NULL)
        {
            continue;
        }

        /* treat the remaining string as the arguments,
         * which may need further parsing
         */
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end)
        {
            args = NULL;
        }

#ifdef HAS_DEVICE
        extern void sdl_clear_event_queue(void);
        sdl_clear_event_queue();
#endif

        int i;
        for (i = 0; i < NR_CMD; i++)
        {
            if (strcmp(cmd, cmd_table[i].name) == 0)
            {
                if (cmd_table[i].handler(args) < 0)
                {
                    return;
                }
                break;
            }
        }

        if (i == NR_CMD)
        {
            printf("Unknown command '%s'\n", cmd);
        }
    }
}
