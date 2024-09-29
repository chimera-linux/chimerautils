find_prog = executable(
    'find',
    [
        'find.c', 'function.c', yacc.process('getdate.y'),
        'ls.c', 'main.c', 'misc.c', 'operator.c', 'option.c',
    ],
    c_args: ['-DQUAD_MAX=LONG_MAX', '-DMAXLOGNAME=_POSIX_LOGIN_NAME_MAX', '-DACL_TYPE_NFS4'],
    include_directories: [inc, cinc],
    dependencies: [libacl],
    link_with: [libcompat, libutil_static],
    install: true,
)

install_man('find.1')
