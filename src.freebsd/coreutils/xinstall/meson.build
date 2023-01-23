xinstall_prog = executable(
    'xinstall',
    ['xinstall.c'],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    dependencies: [libcrypto],
)

meson.add_install_script('install.sh', bindir)
install_man('install.1')
