#include "shell.h"
#include "stdlib.h"
#include "myringbuffer.h"
#include "mytlsf.h"
#include "tlsf.h"

int cmd_tlsf_malloc(int argc, char *argv[])
{
    int   size = atoi(argv[1]);
    void *ptr  = tlsf_malloc(tlsf, size);

    if (!ptr)
    {
        shellWriteString(shellGetCurrent(), "tlsf_malloc failed\n");
        return -1;
    }

    shellPrint(shellGetCurrent(), "tlsf_malloc result: %p, %d bytes memory allocated", ptr, size);
    return 0;
}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN) | SHELL_CMD_DISABLE_RETURN, tlsfmalloc, cmd_tlsf_malloc, tlsf malloc test);
