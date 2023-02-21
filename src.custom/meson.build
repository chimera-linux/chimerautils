conf_data = configuration_data()

conf_data.set('PROJECT_NAME', '"' + meson.project_name() + '"')
conf_data.set('PROJECT_VERSION', '"' + meson.project_version() + '"')

configure_file(output: 'config.h', configuration: conf_data)

inc_main = include_directories('.')

subdir('addpart')
subdir('basenc')
subdir('ctrlaltdel')
subdir('delpart')
subdir('dir')
subdir('flock')
subdir('fsfreeze')
subdir('hostid')
subdir('isosize')
subdir('nproc')
subdir('pivot_root')
subdir('resizepart')
subdir('setarch')
subdir('setsid')
subdir('switch_root')

if libcrypto.found()
    subdir('mcookie')
    subdir('md5sum')
endif
