# compat library

cinc = include_directories('include')
subdir('compat')

# dbcompat library

dbinc = include_directories('dbinclude')
subdir('dbcompat')

# libmp
mpinc = include_directories('libmp')
subdir('libmp')

# standalone toplevel projects

subdir('awk')
subdir('compress')
subdir('ed')
subdir('grep')
subdir('m4')
subdir('patch')
subdir('sed')
subdir('sh')
subdir('shar')
subdir('tip')
subdir('unvis')
subdir('uudecode')
subdir('uuencode')
subdir('vis')
subdir('which')

if not get_option('zlib').disabled() and zlib.found()
    subdir('gzip')
endif

if ncurses.found()
    subdir('nvi')
    subdir('telnet')
endif

if libcrypto.found() and libssl.found()
    subdir('libfetch')
    subdir('fetch')
endif

# tool collections

subdir('bc')
subdir('coreutils')
subdir('diffutils')
subdir('findutils')
subdir('miscutils')
