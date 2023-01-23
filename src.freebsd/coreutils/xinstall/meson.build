xinstall_prog = executable(
    'xinstall',
    ['xinstall.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    dependencies: [libcrypto],
)

meson.add_install_script(install_as, bindir, xinstall_prog, 'install')
install_man('install.1')
