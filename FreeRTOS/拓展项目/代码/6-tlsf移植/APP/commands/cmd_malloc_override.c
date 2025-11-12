#include "shell.h"
#include "stdlib.h"

int cmd_malloc_override(int argc, char *argv[])
{
    int  size = sizeof(int) * 10;
    int *arr  = (int *)malloc(size);
    if (!arr)
    {
        shellPrint(shellGetCurrent(), "Memory allocated failed.");
    } else
    {
        shellPrint(shellGetCurrent(), "%d bytes memory allocated.", size);

        free(arr);
    }
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, mallocoveride, cmd_malloc_override, mallocoveride test);
