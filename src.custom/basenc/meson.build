md5sum = executable('basenc', ['basenc.c'],
    include_directories: inc_main,
    install: true
)

# variants detected via progname
install_symlink('base64', pointing_to: 'basenc', install_dir: bindir)
install_symlink('base32', pointing_to: 'basenc', install_dir: bindir)
