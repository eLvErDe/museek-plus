__all__ = ['Relay']

import os

def relay(lib, libname):
	p = os.popen('nm -D "' + lib + '"')
	d = [x for x in [x.split() for x in p.read().split('\n')] if len(x) == 3]
	functions = []
	variables = []
	for l in d:
		if l[1] == 'T' and l[2] not in ('_init', '_fini'):
			functions.append(l[2])
		elif l[1] in ('D', 'G'):
			variables.append(l[2])
	stub = """
/* automatically generated, do not edit
 *
 * Built by relaytool.py, based on relaytool
 * relaytool.py is (C) 2004 Hyriand <hyriand@thegraveyard.org>
 *
 * relaytool: a program for building delay-load jumptables
 * relaytool is (C) 2004 Mike Hearn <mike@navi.cx>
 * See http://autopackage.org/ for details.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#ifdef __cplusplus
    extern "C" {
#endif

static void **ptrs;
static char *functions[] = {
"""
	for f in functions:
		stub += '\t"%s",\n' % (f)
	stub += """
	0
};

static char *variables[] = {
"""
	for v in variables:
		stub += '\t"%s",\n' % (v)
	stub += """
	0
};

/* 1 if present, 0 if not */
int ${libname}_is_present = 0;

static void *handle = 0;

/* 1 if present, 0 if not, 0 with warning to stderr if lib not present or symbol not found */
int ${libname}_symbol_is_present(char *s) {
    int i;

    if( !${libname}_is_present ) {
        fprintf(stderr, "%s: relaytool: ${lib} not present so cannot check for symbol %s.\\n", getenv("_"), s);
        fprintf(stderr, "%s: relaytool: This probably indicates a bug in the application, please report.\\n", getenv("_"));
        return 0;
    }

    i = 0;
    while (functions[i++]) if (!strcmp( functions[i - 1], s )) return ptrs[i - 1] > 0 ? 1 : 0;
    i = 0;
    while (variables[i++]) if (!strcmp( variables[i - 1], s )) return dlsym( handle, s ) > 0 ? 1 : 0;

    fprintf( stderr, "%s: relaytool: %s is an unknown symbol in ${lib}.\\n", getenv("_"), s );
    fprintf( stderr, "%s: relaytool: If you are the developer of this program, please correct the symbol name or rerun relaytool.\\n", getenv("_") );
    return 0;
}

static __attribute__((noreturn)) void _relaytool_stubcall_${libname}(int offset) {
    fprintf( stderr, "%s: relaytool: stub call to ${lib}:%s, aborting.\\n", getenv("_"),
             functions[offset / sizeof(void*)] );
    exit( 1 );
}

#if defined( __i386__ )
    #define FIXUP_GOT_RELOC(sym, addr) \\
        asm("\tmovl %0, %%eax\\n" \\
            "\tmovl %%eax, " sym "@GOT(%%ebx)\\n" : : "r" (addr));
#elif defined( __powerpc__ )

    /* The PowerPC ELF ABI is a twisted nightmare. Until I figure it out,
       for now we don't support GOT fixup on this architecture */
        
    #error Variables are not currently supported on PowerPC
        
#else        
    #error Please define FIXUP_GOT_RELOC for your architecture
#endif

void __attribute__((constructor)) _relaytool_init_${libname}() {
    int i = 0;

    handle = dlopen( "${lib}", RTLD_LAZY );
    
    if (handle) ${libname}_is_present = 1;

    (void*) ptrs = mmap( NULL,  sizeof(functions), PROT_READ | PROT_WRITE | PROT_EXEC,
                         MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

    assert( ptrs != MAP_FAILED );
    
    /* build function jumptable */
    while (functions[i++]) if (handle) ptrs[i - 1] = dlsym( handle, functions[i - 1] );
"""
	for v in variables:
		stub += '    FIXUP_GOT_RELOC("%s", dlsym(handle, "%s"));\n' % (v, v)
	stub += """
}

#if defined( __i386__ )

#define JUMP_SLOT(name, index)  \\
    asm(".global   " name "\\n" \\
        "        .type " name ", @function\\n"  \\
        name ":\\n" \\
        "        movl ptrs, %eax\\n" \\
        "        movl " #index "(%eax), %eax\\n" \\
        "        test %eax, %eax\\n" \\
        "        jnz  JS" #index "\\n" \\
        "        push $" #index "\\n" \\
        "        call _relaytool_stubcall_${libname}\\n" \\
        "JS" #index ":    jmp *%eax\\n");

#elif defined( __powerpc__ )

#define JUMP_SLOT(name, index) \\
    asm(".global    " name "\\n" \\
        "        .type " name ", @function\\n" \\
        name ":\\n" \\
        "        lis r11, ptrs@ha\\n" \\
        "        lwz r11, " #index "(r11)\\n" \\
        "        cmpi cr0,r11,0\\n" \\
        "        beq- 1f\\n" \\
        "        mtctr r11\\n" \\
        "        bctr\\n" \\
        "1:      li r3, " #index "\\n" \\
        "        b _relaytool_stubcall_${libname}\\n"
        );
        
#else        
    #error Please define JUMP_SLOT for your architecture
#endif

/* define placeholders for the variable imports: their type doesn't matter,
   however we must restrict ELF symbol scope to prevent the definition in the imported
   shared library being bound to this dummy symbol (not all libs are compiled -Bsymbolic)
 */
"""
	for v in variables:
		stub += 'int %s __attribute__(( visibility("hidden") )) = -1;\n' % (v)
	stub += 'asm(".text\\n");\n'
	c = 0
	for f in functions:
		stub += 'JUMP_SLOT("%s", %i);\n' % (f, c * 4)
		c += 1
	stub += """
asm(".previous\\n");

#ifdef __cplusplus
    }
#endif
"""
	return stub.replace("${libname}", libname).replace("${lib}", os.path.basename(lib))


def Relay(env, *libs):
	def RelayStub(target, source, env):
		prefix = env.subst(env['SHLIBPREFIX'])
		suffix = env.subst(env['SHLIBSUFFIX'])
		lib = os.path.split(str(source[0]))[1]
		libname = 'lib' + lib[len(prefix):].split(suffix)[0]
		f = open(str(target[0]), 'w').write(relay(str(source[0]), libname))
	
	if not env['BUILDERS'].has_key('RelayStub'):
		RelayStubBuilder = env.Builder(action = RelayStub)
		env.AppendUnique(BUILDERS = {'RelayStub' : RelayStubBuilder })
	
	def RelayLib(env, lib):
		if not lib in env['LIBS']:
			return None
		path = env['LIBPATH'] + ['/lib', '/usr/lib', '/usr/local/lib']
		for p in path:
			libpath = env.subst(os.path.join(p, 'lib' + lib + '.so'))
			if not os.path.exists(libpath):
				continue
			abspath = os.path.realpath(libpath)
			stub = os.path.split(abspath)[1] + '.stub.c'
			env.RelayStub(stub, abspath)
			env['LIBS'].remove(lib)
			env.AppendUnique(
				LIBS = ['dl'],
				CCFLAGS = ['-fPIC'],
				CPPDEFINES={ 'RELAYED_LIB' + lib.upper(): 1 }
			)
			return stub
		return None
	stubs = []
	for lib in libs:
		stub = RelayLib(env, lib)
		if stub:
			stubs.append(stub)
	return stubs

