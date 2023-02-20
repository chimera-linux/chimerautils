logger_prog = executable(
    'logger',
    ['logger.c'],
    include_directories: [inc],
    c_args: ['-DINET6'],
    install: true,
)

install_man('logger.1')

