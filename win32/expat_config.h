/* expat_config.h — NabiCloud vendored Expat (R_2_6_4) build config for Windows/MSVC.
 *
 * Expat upstream generates this from expat_config.h.cmake (CMake/autotools). We are a
 * hand-wired MSVC static-lib build of libhangul's ENABLE_EXTERNAL_KEYBOARDS XML loader,
 * so we provide the minimal, Windows-accurate values directly. x86/x64 = little-endian.
 * winconfig.h (lib/) supplies the Windows runtime bits; xmlparse.c defines _CRT_RAND_S
 * itself for rand_s() entropy on Windows (no getrandom/arc4random needed here).
 */
#ifndef EXPAT_CONFIG_H
#define EXPAT_CONFIG_H 1

#define BYTEORDER 1234            /* LIL_ENDIAN (x86/x64) */

#define XML_CONTEXT_BYTES 1024
#define XML_DTD 1
#define XML_GE 1
#define XML_NS 1

#define HAVE_MEMMOVE 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_TYPES_H 1

#define PACKAGE "expat"
#define PACKAGE_NAME "expat"
#define PACKAGE_TARNAME "expat"
#define PACKAGE_VERSION "2.6.4"
#define PACKAGE_STRING "expat 2.6.4"
#define PACKAGE_BUGREPORT "expat-bugs@libexpat.org"
#define PACKAGE_URL ""
#define VERSION "2.6.4"

#endif /* EXPAT_CONFIG_H */
