extra_args = [
    '-DLIBSTDBUF="' + join_paths(get_option('prefix'), get_option('libdir'), 'libstdbuf.so') + '"'
]

stdbuf_prog = executable(
    'stdbuf',
    ['stdbuf.c'],
    install: true,
    c_args: extra_args,
)

install_man('stdbuf.1')
