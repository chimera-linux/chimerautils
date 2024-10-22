subdir('bigram')
subdir('code')
subdir('locate')

configure_file(
    input: 'updatedb.sh.in',
    output: 'locate-updatedb',
    configuration: confd,
    format: 'cmake@',
    install_dir: get_option('bindir'),
    install_mode: 'rwxr-xr-x'
)
