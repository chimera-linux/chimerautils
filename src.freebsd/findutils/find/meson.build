find_prog = executable(
    'find',
    [
        'find.c', 'function.c', yacc.process('getdate.y'),
        'ls.c', 'main.c', 'misc.c', 'operator.c', 'option.c',
    ],
    c_args: ['-DQUAD_MAX=LONG_MAX', '-DMAXLOGNAME=_POSIX_LOGIN_NAME_MAX', '-DACL_TYPE_NFS4'],
    include_directories: [inc],
    dependencies: [libfts, librpmatch, libacl],
    link_with: [libcompat],
    install: true,
)

install_man('find.1')
