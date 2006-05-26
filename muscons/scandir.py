import SCons

def CheckScandirType(context, sys_h, dirent, sysndir, sysdir, ndir):
    context.Message('Checking for scandir entry type... ')
    check_code = ''
    if dirent: check_code = check_code + '#include <dirent.h>\n'
    if sysndir: check_code = check_code + '#include <sys/ndir.h>\n'
    if sysdir: check_code = check_code + '#include <sys/dir.h>\n'
    if ndir: check_code = check_code + '#include <ndir.h>\n'
    check_code = check_code + """
int main() {
    struct dirent **temp;
    scandir(0, &temp, 0, 0);
    return 0;
}
"""
    if context.TryCompile(check_code, '.c'):
        context.Result('dirent')
        sys_h.write('#define SCANDIR_ENTRY dirent\n')
    else:
        context.Result('direct')
        sys_h.write('#define SCANDIR_ENTRY direct\n')


def CheckScandir(sys_h, conf):
    dirent = sysndir = sysdir = ndir = 0
    if conf.CheckCHeader('dirent.h'):
        sys_h.write('#include <dirent.h>\n')
        sys_h.write('#define NAMELEN(dirent) strlen((dirent)->d_name\n')
        dirent = 1
    else:
        sys_h.write('#define dirent direct\n')
        sys_h.write('#define NAMELEN(dirent) (dirent)->d_namlen\n')
        if conf.CheckCHeader('sys/ndir.h'):
            sys_h.write('#include <sys/ndir.h>\n')
            sysndir = 1
        if conf.CheckCHeader('sys/dir.h'):
            sys_h.write('#include <sys/dir.h>\n')
            sysdir = 1
        if conf.CheckCHeader('ndir.h'):
            sys_h.write('#include <ndir.h>\n')
            ndir = 1
    if SCons.__version__[:4] == "0.95" or SCons.__version__ == "0.96.1":
        if conf.CheckFunc('scandir',  'C++'):
            sys_h.write('#define HAVE_SCANDIR 1\n')
            conf.CheckScandirType(sys_h, dirent, sysndir, sysdir, ndir)
	else:
            sys_h.write('#define SCANDIR_TYPE direct\n')
    else:
	if conf.CheckFunc('scandir', "", 'C++'):
            sys_h.write('#define HAVE_SCANDIR 1\n')
            conf.CheckScandirType(sys_h, dirent, sysndir, sysdir, ndir)
	else:
            sys_h.write('#define SCANDIR_TYPE direct\n')	   	
    sys_h.write('\n')
