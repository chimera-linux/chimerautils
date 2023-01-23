test_prog = executable(
    'xtest',
    [ 'test.c' ],
    include_directories : inc
)

meson.add_install_script('install.sh', bindir)
install_man('test.1')
