extra_cargs = [
    '-DNO_CATGETS',
    '-DHAS_NCURSES',
    '-DHAS_UNISTD',
    '-DHAS_STDARG',
    '-DHAS_STDLIB',
    '-DHAS_SYS_WAIT',
]

# these are all safe uses
if cc.has_argument('-Wformat-security')
    extra_cargs += ['-Wno-format-security']
endif

if cc.has_argument('-Wpointer-sign')
    extra_cargs += ['-Wno-pointer-sign']
endif


ee_prog = executable(
    'ee', ['ee.c'],
    c_args: extra_cargs,
    include_directories: inc,
    dependencies: [ncurses],
    link_with: [libcompat],
    install: true,
)

install_man('ee.1')
