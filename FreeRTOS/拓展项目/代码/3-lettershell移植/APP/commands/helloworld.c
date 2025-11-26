#include "shell.h"

int cmd_helloworld(int argc, char *argv[])
{
    shellWriteString(shellGetCurrent(), "This is a test command.\n");

    shellPrint(shellGetCurrent(), "Arguments: %d\n", argc);

    for (int i = 0; i < argc; i++)
    {
        shellPrint(shellGetCurrent(), "Argument %d: %s\n", i, argv[i]);
    }

    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, helloworld, cmd_helloworld, this is helloworld description);

int func(int i, char ch, char *str)
{
    shellPrint(shellGetCurrent(), "input int: %d, char: %c, string: %s\r\n", i, ch, str);
    return 0;
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC) | SHELL_CMD_DISABLE_RETURN, func, func, test);
