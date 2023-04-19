fetch_prog = executable(
    'fetch',
    ['fetch.c'],
    include_directories: [inc, libfetch_inc],
    link_with: [libfetch],
    install: true,
)

install_man('fetch.1')

