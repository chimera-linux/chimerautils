conf_data = configuration_data()

conf_data.set('PROJECT_VERSION', '"' + meson.project_version() + '"')

configure_file(output: 'config.h', configuration: conf_data)

inc_main = include_directories('.')

subdir('basenc')
subdir('dir')
subdir('hostid')
subdir('nproc')

if libcrypto.found()
    subdir('md5sum')
endif
