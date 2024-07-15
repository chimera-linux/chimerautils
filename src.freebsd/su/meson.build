su_prog = executable(
    'su',
    ['su.c'],
    include_directories: [inc],
    dependencies: [pam, pam_misc],
    link_with: [libcompat],
    install: true,
    install_dir: bindir,
)

install_man('su.1')
