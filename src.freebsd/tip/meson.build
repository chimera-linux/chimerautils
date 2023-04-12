extra_cargs = ['-DDEFBR=9600', '-DDEFFS=BUFSIZ', '-DCONNECT',]

if cc.has_argument('-Wunused-parameter')
    extra_cargs += ['-Wno-unused-parameter']
endif

if cc.has_argument('-Wclobbered')
    extra_cargs += ['-Wno-clobbered']
endif

tip_prog = executable(
    'cu',
    [
        'tip/acu.c', 'tip/acutab.c', 'tip/cmds.c', 'tip/cmdtab.c',
        'tip/cu.c', 'tip/hunt.c', 'tip/log.c', 'tip/partab.c',
        'tip/remote.c', 'tip/tip.c', 'tip/tipout.c', 'tip/uucplock.c',
        'tip/value.c', 'tip/vars.c',
    ],
    include_directories: [inc, cinc, include_directories('tip')],
    link_with: [libcompat],
    c_args: extra_cargs,
    install: true,
)

install_man('tip/cu.1')
