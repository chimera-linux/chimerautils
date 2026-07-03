diff_prog = executable(
    'diff',
    ['diff.c', 'diffdir.c', 'diffreg.c', 'pr.c', 'xmalloc.c'],
    include_directories: [inc],
    dependencies: [libm],
    link_with: [libcompat],
    c_args: diff_incdefs,
    install: true,
)

install_man('diff.1')
