// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

// This file contains code from Chromium's `build/build_config.h`:
// https://source.chromium.org/chromium/chromium/src/+/main:build/build_config.h;drc=444cda9b1f3be33fffb7c6f93e50d33d4bd39635

#pragma once

#include "build/build_flag.h"  // IWYU pragma: export

// A set of macros to use for platform detection.
#if defined(ANDROID)
#define OS_ANDROID 1
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define OS_IOS 1
#if defined(TARGET_OS_MACCATALYST) && TARGET_OS_MACCATALYST
#define OS_IOS_MACCATALYST 1
#endif  // defined(TARGET_OS_MACCATALYST) && TARGET_OS_MACCATALYST
#if defined(TARGET_OS_TV) && TARGET_OS_TV
#define OS_IOS_TVOS 1
#endif  // defined(TARGET_OS_TV) && TARGET_OS_TV
#if defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#define OS_WATCHOS 1
#endif  // defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#else
#define OS_MAC 1
#endif  // defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#elif defined(__linux__)
#define OS_LINUX 1
// Include features.h for glibc/uclibc macros.
#include <features.h>
#if defined(__GLIBC__) && !defined(__UCLIBC__)
// We really are using glibc, not uClibc pretending to be glibc.
#define LIBC_GLIBC 1
#endif
#elif defined(_WIN32)
#define OS_WIN 1
#elif defined(__Fuchsia__)
#define OS_FUCHSIA 1
#elif defined(__FreeBSD__)
#define OS_FREEBSD 1
#elif defined(__NetBSD__)
#define OS_NETBSD 1
#elif defined(__OpenBSD__)
#define OS_OPENBSD 1
#elif defined(__sun)
#define OS_SOLARIS 1
#elif defined(__QNXNTO__)
#define OS_QNX 1
#elif defined(_AIX)
#define OS_AIX 1
#elif defined(__asmjs__) || defined(__wasm__)
#define OS_ASMJS 1
#elif defined(__MVS__)
#define OS_ZOS 1
#else
#error Please add support for your platform in build/build_config.h
#endif

#if defined(OS_MAC) || defined(OS_IOS)
#define OS_APPLE 1
#endif

// For access to standard BSD features, use OS_BSD instead of a
// more specific macro.
#if defined(OS_FREEBSD) || defined(OS_NETBSD) || defined(OS_OPENBSD)
#define OS_BSD 1
#endif

// For access to standard POSIXish features, use OS_POSIX instead of a
// more specific macro.
#if defined(OS_AIX) || defined(OS_ANDROID) || defined(OS_ASMJS) ||   \
    defined(OS_FREEBSD) || defined(OS_IOS) || defined(OS_LINUX) ||   \
    defined(OS_CHROMEOS) || defined(OS_MAC) || defined(OS_NETBSD) || \
    defined(OS_OPENBSD) || defined(OS_QNX) || defined(OS_SOLARIS) || \
    defined(OS_ZOS)
#define OS_POSIX 1
#endif

// Compiler detection
// Note that clang masquerades as GCC on POSIX and as MSVC on Windows.
#if defined(__GNUC__)
#define COMPILER_GCC 1
#elif defined(_MSC_VER)
#define COMPILER_MSVC 1
#else
#error Please add support for your compiler in build/build_config.h
#endif

// Processor architecture detection.  For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86_64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__s390x__)
#define ARCH_CPU_S390_FAMILY 1
#define ARCH_CPU_S390X 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#elif defined(__s390__)
#define ARCH_CPU_S390_FAMILY 1
#define ARCH_CPU_S390 1
#define ARCH_CPU_31_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#elif (defined(__PPC64__) || defined(__PPC__)) && defined(__BIG_ENDIAN__)
#define ARCH_CPU_PPC64_FAMILY 1
#define ARCH_CPU_PPC64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#elif defined(__PPC64__)
#define ARCH_CPU_PPC64_FAMILY 1
#define ARCH_CPU_PPC64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__ARMEL__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARMEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARM64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__asmjs__) || defined(__wasm__)
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__)
#if defined(__LP64__)
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPS64EL 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#else
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPSEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#endif
#elif defined(__MIPSEB__)
#if defined(__LP64__)
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPS64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#else
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPS 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#endif
#elif defined(__loongarch__)
#define ARCH_CPU_LOONGARCH_FAMILY 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#if __loongarch_grlen == 64
#define ARCH_CPU_LOONGARCH64 1
#define ARCH_CPU_64_BITS 1
#else
#define ARCH_CPU_LOONGARCH32 1
#define ARCH_CPU_32_BITS 1
#endif
#elif defined(__riscv) && (__riscv_xlen == 64)
#define ARCH_CPU_RISCV_FAMILY 1
#define ARCH_CPU_RISCV64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#else
#error Please add support for your architecture in build/build_config.h
#endif

// Type detection for wchar_t.
#if defined(OS_WIN)
#define WCHAR_T_IS_16_BIT 1
#elif defined(OS_FUCHSIA)
#define WCHAR_T_IS_32_BIT 1
#elif defined(OS_POSIX) && defined(COMPILER_GCC) && defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define WCHAR_T_IS_32_BIT 1
#elif defined(OS_POSIX) && defined(COMPILER_GCC) && defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
// compile in this mode (in particular, Chrome doesn't). This is intended for
// other projects using base who manage their own dependencies and make sure
// short wchar works for them.
#define WCHAR_T_IS_16_BIT 1
#else
#error Please add support for your compiler in build/build_config.h
#endif

#if defined(OS_ANDROID)
// The compiler thinks std::string::const_iterator and "const char*" are
// equivalent types.
#define STD_STRING_ITERATOR_IS_CHAR_POINTER 1
// The compiler thinks std::u16string::const_iterator and "char16*" are
// equivalent types.
#define BASE_STRING16_ITERATOR_IS_CHAR16_POINTER 1
#endif

// Architecture-specific feature detection.

#if !defined(CPU_ARM_NEON)
#if defined(ARCH_CPU_ARM_FAMILY) && \
    (defined(__ARM_NEON__) || defined(__ARM_NEON))
#define CPU_ARM_NEON 1
#endif
#endif  // !defined(CPU_ARM_NEON)

// Sanity check.
#if defined(ARCH_CPU_ARM64) && !defined(CPU_ARM_NEON)
#error "AArch64 mandates NEON, should be detected"
#endif

#if !defined(HAVE_MIPS_MSA_INTRINSICS)
#if defined(__mips_msa) && defined(__mips_isa_rev) && (__mips_isa_rev >= 5)
#define HAVE_MIPS_MSA_INTRINSICS 1
#endif
#endif

// INTERNAL Build Flags Unified Interface

// Disable cpplint and clang-format to keep line length > 80 characters as is.
// NOLINTBEGIN(whitespace/line_length)
// clang-format off

// Build Configuration
#if !defined(NDEBUG)
#define CHRB_BUILD_FLAG_INTERNAL_IS_DEBUG() 1
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_DEBUG() 0
#endif

#if defined(NDEBUG)
#define CHRB_BUILD_FLAG_INTERNAL_IS_RELEASE() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_RELEASE() (0)
#endif

// OS Internal Flags
#if defined(OS_AIX)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_AIX() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_AIX() (0)
#endif

#if defined(OS_ANDROID)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_ANDROID() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_ANDROID() (0)
#endif

#if defined(OS_APPLE)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_APPLE() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_APPLE() (0)
#endif

#if defined(OS_ASMJS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_ASMJS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_ASMJS() (0)
#endif

#if defined(OS_BSD)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_BSD() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_BSD() (0)
#endif

#if defined(OS_CHROMEOS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_CHROMEOS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_CHROMEOS() (0)
#endif

#if defined(OS_FREEBSD)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_FREEBSD() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_FREEBSD() (0)
#endif

#if defined(OS_FUCHSIA)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_FUCHSIA() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_FUCHSIA() (0)
#endif

#if defined(OS_IOS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_IOS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_IOS() (0)
#endif

#if defined(OS_IOS_MACCATALYST)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_IOS_MACCATALYST() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_IOS_MACCATALYST() (0)
#endif

#if defined(OS_IOS_TVOS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_IOS_TVOS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_IOS_TVOS() (0)
#endif

#if defined(OS_LINUX)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_LINUX() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_LINUX() (0)
#endif

#if defined(OS_MAC)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_MAC() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_MAC() (0)
#endif

#if defined(OS_NETBSD)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_NETBSD() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_NETBSD() (0)
#endif

#if defined(OS_OPENBSD)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_OPENBSD() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_OPENBSD() (0)
#endif

#if defined(OS_POSIX)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_POSIX() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_POSIX() (0)
#endif

#if defined(OS_QNX)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_QNX() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_QNX() (0)
#endif

#if defined(OS_SOLARIS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_SOLARIS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_SOLARIS() (0)
#endif

#if defined(OS_WATCHOS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_WATCHOS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_WATCHOS() (0)
#endif

#if defined(OS_WIN)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_WIN() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_WIN() (0)
#endif

#if defined(OS_ZOS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_ZOS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_ZOS() (0)
#endif

#if defined(USE_OZONE)
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_OZONE() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_OS_OZONE() (0)
#endif

// Compiler Internal Flags
#if defined(COMPILER_GCC)
#define CHRB_BUILD_FLAG_INTERNAL_IS_COMPILER_GCC() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_COMPILER_GCC() (0)
#endif

#if defined(COMPILER_MSVC)
#define CHRB_BUILD_FLAG_INTERNAL_IS_COMPILER_MSVC() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_COMPILER_MSVC() (0)
#endif

// Architecture Internal Flags
#if defined(ARCH_CPU_X86_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_X86_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_X86_FAMILY() (0)
#endif

#if defined(ARCH_CPU_X86)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_X86() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_X86() (0)
#endif

#if defined(ARCH_CPU_X86_64)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_X86_64() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_X86_64() (0)
#endif

#if defined(ARCH_CPU_ARM_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_ARM_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_ARM_FAMILY() (0)
#endif

#if defined(ARCH_CPU_ARMEL)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_ARMEL() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_ARMEL() (0)
#endif

#if defined(ARCH_CPU_ARM64)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_ARM64() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_ARM64() (0)
#endif

#if defined(ARCH_CPU_MIPS_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS_FAMILY() (0)
#endif

#if defined(ARCH_CPU_MIPSEL)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPSEL() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPSEL() (0)
#endif

#if defined(ARCH_CPU_MIPS64EL)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS64EL() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS64EL() (0)
#endif

#if defined(ARCH_CPU_MIPS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS() (0)
#endif

#if defined(ARCH_CPU_MIPS64)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS64() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_MIPS64() (0)
#endif

#if defined(ARCH_CPU_PPC64_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_PPC64_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_PPC64_FAMILY() (0)
#endif

#if defined(ARCH_CPU_PPC64)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_PPC64() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_PPC64() (0)
#endif

#if defined(ARCH_CPU_S390_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_S390_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_S390_FAMILY() (0)
#endif

#if defined(ARCH_CPU_S390)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_S390() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_S390() (0)
#endif

#if defined(ARCH_CPU_S390X)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_S390X() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_S390X() (0)
#endif

#if defined(ARCH_CPU_LOONGARCH_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LOONGARCH_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LOONGARCH_FAMILY() (0)
#endif

#if defined(ARCH_CPU_LOONGARCH32)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LOONGARCH32() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LOONGARCH32() (0)
#endif

#if defined(ARCH_CPU_LOONGARCH64)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LOONGARCH64() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LOONGARCH64() (0)
#endif

#if defined(ARCH_CPU_RISCV_FAMILY)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_RISCV_FAMILY() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_RISCV_FAMILY() (0)
#endif

#if defined(ARCH_CPU_RISCV64)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_RISCV64() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_RISCV64() (0)
#endif

// Word Size and Endian Internal Flags
#if defined(ARCH_CPU_31_BITS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_31_BITS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_31_BITS() (0)
#endif

#if defined(ARCH_CPU_32_BITS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_32_BITS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_32_BITS() (0)
#endif

#if defined(ARCH_CPU_64_BITS)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_64_BITS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_64_BITS() (0)
#endif

#if defined(ARCH_CPU_BIG_ENDIAN)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_BIG_ENDIAN() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_BIG_ENDIAN() (0)
#endif

#if defined(ARCH_CPU_LITTLE_ENDIAN)
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LITTLE_ENDIAN() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_IS_ARCH_LITTLE_ENDIAN() (0)
#endif

// Type and Feature Internal Flags
#if defined(WCHAR_T_IS_16_BIT)
#define CHRB_BUILD_FLAG_INTERNAL_WCHAR_T_IS_16_BIT() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_WCHAR_T_IS_16_BIT() (0)
#endif

#if defined(WCHAR_T_IS_32_BIT)
#define CHRB_BUILD_FLAG_INTERNAL_WCHAR_T_IS_32_BIT() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_WCHAR_T_IS_32_BIT() (0)
#endif

#if defined(CPU_ARM_NEON)
#define CHRB_BUILD_FLAG_INTERNAL_CPU_ARM_NEON() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_CPU_ARM_NEON() (0)
#endif

#if defined(HAVE_MIPS_MSA_INTRINSICS)
#define CHRB_BUILD_FLAG_INTERNAL_HAVE_MIPS_MSA_INTRINSICS() (1)
#else
#define CHRB_BUILD_FLAG_INTERNAL_HAVE_MIPS_MSA_INTRINSICS() (0)
#endif
// clang-format on
// NOLINTEND)
