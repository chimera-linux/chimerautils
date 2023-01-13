md5sum = executable('basenc', ['basenc.c'],
    include_directories: inc_main,
    install: true
)

# variants detected via progname
meson.add_install_script(install_link, 'basenc', 'base64', get_option('bindir'))
meson.add_install_script(install_link, 'basenc', 'base32', get_option('bindir'))
