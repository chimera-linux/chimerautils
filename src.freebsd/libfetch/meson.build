extra_cargs = ['-DFTP_COMBINE_CWDS', '-DINET6', '-DWITH_SSL']

# uses some stuff deprecated in newer openssl
if cc.has_argument('-Wdeprecated-declarations')
    extra_cargs += ['-Wno-deprecated-declarations']
endif

ftperr_h = custom_target(
    'ftperr_h',
    output: 'ftperr.h',
    input: ['ftp.errors'],
    command: ['./gen_errors.sh', 'ftp', '@INPUT@'],
    capture: true
)

httperr_h = custom_target(
    'httperr_h',
    output: 'httperr.h',
    input: ['http.errors'],
    command: ['./gen_errors.sh', 'http', '@INPUT@'],
    capture: true
)

libfetch_inc = include_directories('.')

libfetch = static_library(
    'libfetch',
    ['fetch.c', 'common.c', 'ftp.c', 'http.c', 'file.c', ftperr_h, httperr_h],
    include_directories: [inc, cinc, libfetch_inc],
    dependencies: [libssl, libcrypto],
    link_with: [libcompat],
    c_args: extra_cargs,
)
