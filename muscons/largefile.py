# Large file check, mostly stolen from autoconf
def CheckLargeFile(context, sys_h):
    lf_check_code = """
#include <sys/types.h>
#define LARGE_OFF_T (((off_t) 1 << 62) - 1 + ((off_t) 1 << 62))
int off_t_is_large[(LARGE_OFF_T % 2147483629 == 721 && LARGE_OFF_T % 2147483647 == 1) ? 1 : -1];
int main() { return 0; }
"""
    context.Message('Checking for large file support... ')
    if context.TryCompile(lf_check_code, '.c'):
        context.Result('yes')
    elif context.TryCompile('#define _FILE_OFFSET_BITS 64\n' + lf_check_code, '.c'):
        context.Result('yes (_FILE_OFFSET_BITS=64)')
        sys_h.write('#define _FILE_OFFSET_BITS 64\n\n')
    elif context.TryCompile('#define _LARGE_FILES 1\n' + lf_check_code, '.c'):
        context.Result('yes (_LARGE_FILES=1)')
        sys_h.write('#define _LARGE_FILES 1\n\n')

