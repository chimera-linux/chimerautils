extra_cargs = [
    '-DDEFBR=9600', '-DDEFFS=BUFSIZ', '-DCONNECT', '-DACULOG', '-DPRISTINE',
    '-DV831', '-DVENTEL', '-DHAYES', '-DCOURIER', '-DT3000',
]

if cc.has_argument('-Wunused-parameter')
    extra_cargs += ['-Wno-unused-parameter']
endif

if cc.has_argument('-Wclobbered')
    extra_cargs += ['-Wno-clobbered']
endif

tip_prog = executable(
    'tip',
    [
        'tip/acu.c', 'tip/acutab.c', 'tip/cmds.c', 'tip/cmdtab.c',
        'tip/cu.c', 'tip/hunt.c', 'tip/log.c', 'tip/partab.c',
        'tip/remote.c', 'tip/tip.c', 'tip/tipout.c', 'tip/uucplock.c',
        'tip/value.c', 'tip/vars.c',
        # libacu
        'libacu/biz22.c', 'libacu/courier.c', 'libacu/df.c', 'libacu/dn11.c',
        'libacu/hayes.c', 'libacu/t3000.c', 'libacu/v3451.c', 'libacu/v831.c',
        'libacu/ventel.c',
        # from freebsd libc
        'getcap.c',
    ],
    include_directories: [inc, cinc, include_directories('tip')],
    link_with: [libcompat],
    c_args: extra_cargs,
    install: true,
)

install_man('tip/tip.1')
install_man('tip/cu.1')
install_man('remote.5')

install_symlink('cu', pointing_to: 'tip', install_dir: bindir)

install_data(
    'tip/remote',
    install_dir: get_option('sysconfdir'),
    install_mode: 'rw-r--r--'
)

install_data(
    'tip/phones',
    install_dir: get_option('sysconfdir'),
    install_mode: 'rw-r--r--'
)
