xinstall_prog = executable(
    'xinstall',
    ['xinstall.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    dependencies: [libcrypto],
)

meson.add_install_script(install_as, bindir, xinstall_prog, 'install', 'binstall')

install_man('install.1')
install_symlink('binstall.1', pointing_to: 'install.1', install_dir: man1dir)
