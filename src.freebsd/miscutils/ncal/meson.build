ncal_prog = executable(
    'ncal',
    ['ncal.c', 'calendar.c', 'easter.c'],
    include_directories: [inc],
    dependencies: [libtinfo],
    install: true,
)

install_man('ncal.1')

install_symlink('cal', pointing_to: 'ncal', install_dir: bindir)
install_symlink('cal.1', pointing_to: 'ncal.1', install_dir: man1dir)
