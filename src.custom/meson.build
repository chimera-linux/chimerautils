conf_data = configuration_data()

conf_data.set('PROJECT_NAME', '"' + meson.project_name() + '"')
conf_data.set('PROJECT_VERSION', '"' + meson.project_version() + '"')

configure_file(output: 'config.h', configuration: conf_data)

inc_main = include_directories('.')

subdir('basenc')
subdir('ctrlaltdel')
subdir('dir')
subdir('flock')
subdir('hostid')
subdir('nproc')
subdir('setarch')
subdir('setsid')

if libcrypto.found()
    subdir('mcookie')
    subdir('md5sum')
endif
