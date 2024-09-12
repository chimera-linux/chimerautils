xinstall_prog = executable(
    'xinstall',
    ['xinstall.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    dependencies: [libcrypto],
)

install_symlink('binstall', pointing_to: 'install', install_dir: bindir)
install_symlink('binstall.1', pointing_to: 'install.1', install_dir: man1dir)

meson.add_install_script(install_as, bindir, xinstall_prog, 'install')
install_man('install.1')
