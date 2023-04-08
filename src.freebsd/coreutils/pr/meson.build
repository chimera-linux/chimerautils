pr_prog = executable(
    'pr',
    ['egetopt.c', 'pr.c'],
    include_directories: [inc],
    link_with: [libcompat],
    install: true,
)

install_man('pr.1')
