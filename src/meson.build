# standalone toplevel projects

subdir('ed')
subdir('grep')
subdir('m4')
subdir('patch')
subdir('sed')
subdir('sh')
subdir('which')

if not get_option('zlib').disabled() and zlib.found()
    subdir('gzip')
endif

# tool collections

subdir('bc')
subdir('coreutils')
subdir('diffutils')
subdir('findutils')
subdir('miscutils')
subdir('extra')
