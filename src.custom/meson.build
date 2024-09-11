conf_data = configuration_data()

conf_data.set('PROJECT_NAME', '"' + meson.project_name() + '"')
conf_data.set('PROJECT_VERSION', '"' + meson.project_version() + '"')

configure_file(output: 'config.h', configuration: conf_data)

inc_main = include_directories('.')

subdir('addpart')
subdir('ctrlaltdel')
subdir('delpart')
subdir('dir')
subdir('fallocate')
subdir('flock')
subdir('fsfreeze')
subdir('hostid')
subdir('ionice')
subdir('isosize')
subdir('pivot_root')
subdir('resizepart')
subdir('setarch')
subdir('setsid')
subdir('switch_root')
subdir('taskset')

if libcrypto.found()
    subdir('mcookie')
endif

if chimera_realpath.enabled()
    subdir('realpath')
endif
