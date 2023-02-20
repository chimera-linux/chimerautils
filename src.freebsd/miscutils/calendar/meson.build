calendar_prog = executable(
    'calendar',
    [
        'calendar.c', 'locale.c', 'events.c', 'dates.c', 'parsedata.c',
        'io.c', 'day.c', 'ostern.c', 'paskha.c', 'pom.c', 'sunpos.c',
    ],
    include_directories: [inc, cinc],
    link_with: [libcompat],
    dependencies: [libm],
    c_args: ['-DWITH_ICONV'],
    install: true,
)

install_man('calendar.1')
