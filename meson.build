project('gssproxy', 'c',
        default_options: ['c_std=c99'])

### Configuration ###
version = '0.8.0'
prerelease_version = ''

compiler = meson.get_compiler('c')
config = configuration_data()

add_project_arguments(
    '-D_GNU_SOURCE', '-D_FILE_OFFSET_BITS=64', '-D_LARGEFILE_SOURCE',
    '-D_LARGEFILE64_SOURCE', '-Wall', '-Wshadow', '-Wstrict-prototypes',
    '-Wpointer-arith', '-Wcast-qual', '-Wcast-align', '-Wwrite-strings',
    '-fstrict-aliasing', '-Wstrict-aliasing', '-Werror=strict-aliasing',
    '-Werror-implicit-function-declaration', '-Werror=format-security',
    '-Wextra', language: 'c')
if get_option('hardening')
    add_project_arguments(
        '-D_FORTIFY_SOURCE=2', '-Wdate-time', '-fPIE',
        '-fstack-protector-string', '-pie', '-fPIC', '-Wl,-z,relro',
        '-Wl,-z,now', language: 'c')
endif

config.set_quoted('VERSION', version)
config.set_quoted('DISTRO_VERSION', get_option('distro-version'))
config.set_quoted('GP_SOCKET_NAME', get_option('socket-name'))
config.set_quoted('GP_PID_FILE', get_option('pid-file'))
config.set_quoted('PUBCONF_PATH', get_option('pubconf-path'))
default_behavior = get_option('gpp-default-behavior')
config.set('default_behavior', default_behavior)
config.set('GPP_DEFAULT_BEHAVIOR',
           'GPP_' + get_option('gpp-default-behavior'))
config.set10('GSSI_DEBUGGING', get_option('gssidebug'))

config.set('PACKAGE_VERSION', version)
config.set('PRERELEASE_VERSION', prerelease_version)
config.set('PACKAGE_NAME', 'gssproxy')

prefix = get_option('prefix') + '/'
config.set('libdir', prefix + get_option('libdir'))
config.set('sbindir', prefix + get_option('sbindir'))
mandir = get_option('prefix') + get_option('mandir')

localstatedir = get_option('localstatedir')
config.set('localstatedir', localstatedir)
gpstatedir = localstatedir + '/lib/gssproxy' # not used directly
config.set('gpclidir', gpstatedir + '/clients')

threads = dependency('threads') # probably pthreads
popt = dependency('popt')

verto = dependency('libverto', version: '>=0.2.2')
if compiler.has_function('verto_cleanup', dependencies: verto)
    config.set10('HAVE_VERTO_CLEANUP', true)
else
    message('Warning: libverto lacks verto_cleanup function')
endif

ini_config = dependency('ini_config', version: '>=1.2.0')
if not compiler.has_function('ini_config_file_open',
                             dependencies: ini_config)
    error('libini_config is missing ini_config_file_open!')
elif not compiler.has_function('ini_config_augment',
                               dependencies: ini_config)
    error('libini_config is missing ini_config_augment!')
endif

krb5 = dependency('krb5')
gssrpc = dependency('gssrpc')
gssapi = dependency('krb5-gssapi')
if not compiler.has_header_symbol('gssapi/gssapi_krb5.h',
                                  'GSS_KRB5_CRED_NO_CI_FLAGS_X')
    error('gssapi/krb5 too old; missing GSS_KRB5_CRED_NO_CI_FLAGS_X (v1.14)')
endif
config.set10('HAVE_GSS_KRB5_GET_CRED_IMPERSONATOR',
             compiler.has_header_symbol('gssapi/gssapi_krb5.h',
                                        'GSS_KRB5_GET_CRED_IMPERSONATOR'))

sec_getenv_prefix = '#define _GNU_SOURCE\n#include <stdlib.h>'
if compiler.has_function('secure_getenv', prefix: sec_getenv_prefix)
    config.set10('HAVE_SECURE_GETENV', true)
elif compiler.has_function('__secure_getenv', prefix: sec_getenv_prefix)
    config.set10('HAVE___SECURE_GETENV', true)
endif

configure_file(output: 'config.h', configuration: config)

### Build gssproxy ###
rpcgen_sources = ['rpcgen/gp_rpc_xdr.c',
                  'rpcgen/gp_xdr.c',
                  'rpcgen/gss_proxy_xdr.c']
util_sources = ['src/gp_conv.c',
                'src/gp_util.c']
gssproxy_sources = ['src/gp_config.c',
                    'src/gp_creds.c',
                    'src/gp_debug.c',
                    'src/gp_export.c',
                    'src/gp_init.c',
                    'src/gp_log.c',
                    'src/gp_rpc_accept_sec_context.c',
                    'src/gp_rpc_acquire_cred.c',
                    'src/gp_rpc_debug.c',
                    'src/gp_rpc_get_mic.c',
                    'src/gp_rpc_import_and_canon_name.c',
                    'src/gp_rpc_indicate_mechs.c',
                    'src/gp_rpc_init_sec_context.c',
                    'src/gp_rpc_process.c',
                    'src/gp_rpc_release_handle.c',
                    'src/gp_rpc_unwrap.c',
                    'src/gp_rpc_verify_mic.c',
                    'src/gp_rpc_wrap.c',
                    'src/gp_rpc_wrap_size_limit.c',
                    'src/gp_socket.c',
                    'src/gp_workers.c',
                    'src/gssproxy.c']
gssproxy_includes = include_directories('.', 'rpcgen',
                                        'src', 'src/client', 'src/mechglue')
gssproxy_deps = [krb5, gssapi, gssrpc, threads, verto, ini_config, popt]
executable('gssproxy', gssproxy_sources + rpcgen_sources + util_sources,
           dependencies: gssproxy_deps,
           include_directories: gssproxy_includes,
           install: true, install_dir: get_option('sbindir'))

### Build proxymech.so ###
mechglue_sources = ['src/mechglue/gpp_accept_sec_context.c',
                    'src/mechglue/gpp_acquire_cred.c',
                    'src/mechglue/gpp_context.c',
                    'src/mechglue/gpp_creds.c',
                    'src/mechglue/gpp_display_status.c',
                    'src/mechglue/gpp_import_and_canon_name.c',
                    'src/mechglue/gpp_indicate_mechs.c',
                    'src/mechglue/gpp_init_sec_context.c',
                    'src/mechglue/gpp_misc.c',
                    'src/mechglue/gpp_priv_integ.c',
                    'src/mechglue/gss_plugin.c']
client_sources = ['src/client/gpm_accept_sec_context.c',
                  'src/client/gpm_acquire_cred.c',
                  'src/client/gpm_common.c',
                  'src/client/gpm_display_status.c',
                  'src/client/gpm_get_mic.c',
                  'src/client/gpm_import_and_canon_name.c',
                  'src/client/gpm_indicate_mechs.c',
                  'src/client/gpm_init_sec_context.c',
                  'src/client/gpm_inquire_context.c',
                  'src/client/gpm_release_handle.c',
                  'src/client/gpm_unwrap.c',
                  'src/client/gpm_verify_mic.c',
                  'src/client/gpm_wrap.c',
                  'src/client/gpm_wrap_size_limit.c']
proxymech_includes = include_directories('.', 'src/client', 'src/mechglue')

proxymech_deps = [gssapi, gssrpc, krb5, threads]
library('proxymech',
        rpcgen_sources + mechglue_sources + client_sources + util_sources,
        dependencies: proxymech_deps,
        include_directories: proxymech_includes,
        install: true, install_dir: get_option('libdir') + '/gssproxy')

### Build man pages ###
if get_option('manpages')
    xmllint = find_program('xmllint')
    xsltproc = find_program('xsltproc')

    mans = ['man/gssproxy.8.xml.in',
            'man/gssproxy.conf.5.xml.in',
            'man/gssproxy-mech.8.xml.in']
    foreach m: mans
        barename = m.split('/')[-1]
        num = barename.split('.')[-1]

        xml = configure_file(input: m, output: '@BASENAME@',
                             configuration: config)

        xmllintcmd = [xmllint, '--catalogs', '--postvalid', '--nonet',
                      '--xinclude', '--noout', '@INPUT@']
        precheck = custom_target(barename + '-linted',
                                 input: xml,
                                 capture: true,
                                 output: barename + '-linted',
                                 command: xmllintcmd)

        xsltproccmd = [
            xsltproc, '--xinclude', '--nonet', '-o', '@OUTPUT@',
            'http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl',
            '@INPUT@']
        custom_target(barename,
                      input: xml,
                      output: barename,
                      command: xsltproccmd,
                      depends: precheck,
                      install: true,
                      install_dir: mandir + '/man' + num)
    endforeach
endif

### Build free samples ###
confs = ['examples/24-nfs-server.conf.in',
         'examples/80-httpd.conf.in',
         'examples/99-nfs-client.conf.in',
         'examples/gssproxy.conf.in',
        ]
foreach c: confs
    configure_file(input: c, output: '@BASENAME@', configuration: config,
                   install_dir: 'etc/gssproxy')
endforeach

configure_file(input: 'examples/mech.in', output: '@BASENAME@',
               configuration: config, install_dir: 'etc/gss/mech.d')

configure_file(input: 'contrib/gssproxy.spec.in', output: '@BASENAME@',
               configuration: config)

configure_file(input: 'systemd/gssproxy.service.in', output: '@BASENAME@',
               configuration: config)

### Build tests ###
tests = ['tests/cli_srv_comm.c',
         'tests/interposetest.c',
         'tests/t_accept.c',
         'tests/t_acquire.c',
         'tests/t_cred_store.c',
         'tests/t_impersonate.c',
         'tests/t_init.c',
         'tests/t_setcredopt.c',
        ]
test_specific = ['tests/t_utils.c', 'src/gp_debug.c', 'src/gp_log.c']
test_files = client_sources + util_sources + rpcgen_sources + test_specific

test_includes = include_directories('.', 'rpcgen', 'src', 'src/client')
foreach t: tests
    exe_name = t.split('/')[-1].split('.')[0]
    executable(exe_name, [t] + test_files, dependencies: gssproxy_deps,
               include_directories: test_includes)
endforeach

testprog = find_program('tests/runtests.py')

# actually running the tests is... hmm.  The location of the current build
# directory needs to be passed back into runtests.py (actually testlib.py)
# which is kind of gross and we're not set up for that yet.
