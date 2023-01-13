md5sum = executable('md5sum', ['md5sum.c'],
    dependencies: [libcrypto],
    include_directories: inc_main,
    install: true
)

# variants detected via progname, emulates most of freebsd and gnu ones
meson.add_install_script(install_link, 'md5sum', 'b2sum', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha1sum', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha224sum', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha256sum', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha384sum', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha512sum', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'md5', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha1', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha224', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha256', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha384', get_option('bindir'))
meson.add_install_script(install_link, 'md5sum', 'sha512', get_option('bindir'))
