test_prog = executable(
    'xtest',
    ['test.c'],
    include_directories: inc
)

meson.add_install_script(install_as, bindir, test_prog, 'test', '[')
install_man('test.1')
