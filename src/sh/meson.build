mknodes = executable(
    'mknodes',
    'mknodes.c',
    include_directories : [ inc, ],
    native: true,
)

mksyntax = executable(
    'mksyntax',
    'mksyntax.c',
    include_directories : [ inc, ],
    native: true,
)

curdir = meson.current_source_dir()
bldir = meson.current_build_dir()

mkbuiltins_gen = custom_target('builtins.h',
    output : [ 'builtins.h', 'builtins.c' ],
    command : [
        'wrapper.sh', bldir, join_paths(curdir, 'mkbuiltins'), curdir
    ]
)

mknodes_gen = custom_target('nodes.h',
    output : [ 'nodes.h', 'nodes.c' ],
    command : [
        'wrapper.sh', bldir, mknodes,
        join_paths(curdir, 'nodetypes'),
        join_paths(curdir, 'nodes.c.pat')
    ]
)

mksyntax_gen = custom_target('syntax.h',
    output : [ 'syntax.h', 'syntax.c' ],
    command : [ 'wrapper.sh', bldir, mksyntax, ]
)

mktokens_gen = custom_target('gen-tokens',
    output : [ 'token.h' ],
    command : [ 'wrapper.sh', bldir, join_paths(curdir, 'mktokens') ]
)

bltins_src = [
    'bltin/echo.c',
    '../miscutils/kill/kill.c',
    '../coreutils/printf/printf.c',
    '../coreutils/test/test.c',
    mkbuiltins_gen,
]

bltins_flags = [ '-DSHELL' ]

sh_src = [
    'alias.c',
    'arith_yacc.c',
    'arith_yylex.c',
    'cd.c',
    'error.c',
    'eval.c',
    'exec.c',
    'expand.c',
    'histedit.c',
    'input.c',
    'jobs.c',
    'mail.c',
    'main.c',
    'memalloc.c',
    'miscbltin.c',
    'mystring.c',
    'options.c',
    'output.c',
    'parser.c',
    'redir.c',
    'show.c',
    'trap.c',
    'var.c',
    mkbuiltins_gen,
    mknodes_gen,
    mksyntax_gen,
    mktokens_gen,
]

sh_flags = []

if not libedit.found()
    sh_flags += [ '-DNO_HISTORY' ]
endif

if cc.has_argument('-Wclobbered')
    sh_flags += [ '-Wno-clobbered' ]
endif

bltins_lib = static_library(
    'bltins_lib',
    bltins_src,
    c_args : bltins_flags,
    include_directories : [ inc, ],
    pic : true,
    install : false
)

sh_prog = executable(
    'sh', sh_src,
    c_args : sh_flags,
    include_directories : [ inc, ],
    dependencies : [ libedit ],
    link_with : [ bltins_lib, libcompat, ],
    install : true,
)

install_man('sh.1')
