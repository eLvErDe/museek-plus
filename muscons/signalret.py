def CheckSignalRetType(context, sys_h):
    context.Message('Checking signal callback return type... ')
    if context.TryCompile("""
#include <signal.h>
void test_handler(int s) { };
int main() {
     signal(11, test_handler);
}
""", '.cc'):
        context.Result('void')
        sys_h.write('#define RETSIGTYPE void\n')
    else:
        context.Result('int')
        sys_h.write('#define RETSIGTYPE int\n')
