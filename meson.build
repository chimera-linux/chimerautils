project(
    'chimerautils',
    ['c', 'cpp'],
    version: '14.0.7',
    default_options: ['c_std=c99', 'warning_level=2', 'buildtype=debugoptimized'],
    license: 'BSD-3-Clause',
    meson_version: '>=0.61'
)

# to be reused
bindir = get_option('bindir')
sbindir = get_option('sbindir')
man1dir = join_paths(get_option('mandir'), 'man1')
man5dir = join_paths(get_option('mandir'), 'man5')
man8dir = join_paths(get_option('mandir'), 'man8')
tiny = get_option('tiny')

cc = meson.get_compiler('c')
cpp = meson.get_compiler('cpp')

# FreeBSD code sometimes ignores things
add_global_arguments(['-Wno-unused-result'], language: ['c', 'cpp'])

# Avoid a pointless warning in some programs
if cc.has_argument('-Wstring-plus-int')
    add_global_arguments(['-Wno-string-plus-int'], language: ['c'])
endif

# Find lex
lex_prog = find_program('flex', 'lex', required: true)
lex = generator(
    lex_prog,
    output: ['@BASENAME@.c'],
    arguments: ['-o', '@OUTPUT0@', '@INPUT@'],
)

# Find yacc
# (With help from libxkbcommon at https://github.com/xkbcommon/libxkbcommon)
byacc = find_program('byacc', required: false)
if byacc.found()
    yacc = generator(
        byacc,
        output: ['@BASENAME@.c', '@BASENAME@.h'],
        arguments: ['-H', '@OUTPUT1@', '-o', '@OUTPUT0@', '@INPUT@'],
    )
else
    bison = find_program('bison', required: false)
    if bison.found()
        yacc = generator(
            bison,
            output: ['@BASENAME@.c', '@BASENAME@.h'],
            arguments: ['--defines=@OUTPUT1@', '-o', '@OUTPUT0@', '@INPUT@'],
        )
    else
        error('Could not find a compatible yacc(1) program (bison or byacc)')
    endif
endif

# Build everything with _GNU_SOURCE enabled
add_project_arguments('-D_GNU_SOURCE', language: ['c', 'cpp'])

# Always use 64-bit file offsets on glibc
add_project_arguments('-D_FILE_OFFSET_BITS=64', language: ['c', 'cpp'])

# Get rid of unused metadata variables
add_project_arguments('-Dlint', language: ['c'])

# locate ncurses, and try accounting for separate tinfo if necessary
# as e.g. color ls functionality needs just terminfo and not full curses
tinfo = dependency('tinfo', required: false)

# in that order of preference; 'curses' will fall back to whatever
# meson built-in behavior, which may or may not be correct, in an
# attempt to be compatible with netbsd curses and so on
#
# some stuff needs full curses (nvi, telnet)
foreach opt: ['ncursesw', 'ncurses', 'curses']
    ncurses = dependency(opt, required: false)
    if ncurses.found()
        break
    endif
endforeach

# if tinfo is not found, use ncurses; it may be integrated
if not tinfo.found()
    tinfo = ncurses
endif

curses_incdefs = []

if cc.has_header('ncursesw/ncurses.h', dependencies: ncurses)
    curses_incdefs += ['-DHAVE_NCURSESW_NCURSES_H']
elif cc.has_header('ncurses.h', dependencies: ncurses)
    curses_incdefs += ['-DHAVE_NCURSES_H']
endif

# libacl is needed in several tools
libacl = dependency('libacl', required: true)

# bc(1) needs libedit
libedit = dependency('libedit', required: get_option('libedit'))

# dc(1), install(1), sort(1), fetch(1) need libcrypto
libcrypto = dependency('libcrypto', required: get_option('openssl'))

# fetch(1) needs libssl
libssl = dependency('libssl', required: get_option('openssl'))

# seq(1) needs libm
libm = cc.find_library('m', required: true)

# df(1) needs libxo
libxo = dependency('libxo', required: true)

# gzip needs zlib
zlib = dependency('zlib', required: get_option('zlib'))

# gzip needs bzip2
bzip2 = cc.find_library('bz2', required: get_option('bzip2'))

# gzip needs liblzma
lzma = dependency('liblzma', required: get_option('lzma'))

# gzip needs zstd
zstd = dependency('libzstd', required: get_option('zstd'))

# needed by a few utils; provided by glibc, but not by e.g. musl

fts_dirs = []
rpmatch_dirs = []

if get_option('fts_path') != ''
    fts_dirs += get_option('fts_path')
endif

if get_option('rpmatch_path') != ''
    rpmatch_dirs += get_option('rpmatch_path')
endif

libfts = cc.find_library('fts',
    required: not cc.has_function('fts_open'),
    static: get_option('static_fts'),
    dirs: fts_dirs
)
librpmatch = cc.find_library('rpmatch',
    required: not cc.has_function('rpmatch'),
    static: get_option('static_rpmatch'),
    dirs: rpmatch_dirs
)

# sort needs pthreads
libpthread = dependency('threads')

# meson at the moment provides no way to rename installed executables
install_as = files('install-as.sh')

# Include all of the relevant subdirectories
subdir('include')
subdir('src.freebsd')
subdir('src.custom')
