/// dllimport and dllexport express non-hidden intention. Don't apply hidden visibility.

// RUN: %clang_cc1 -emit-llvm -triple x86_64-windows-msvc -fdeclspec -fvisibility-inlines-hidden -o - %s | FileCheck %s --check-prefix=MSVC
// RUN: %clang_cc1 -emit-llvm -triple x86_64-windows-gnu -fdeclspec -fvisibility-inlines-hidden -o - %s | FileCheck %s --check-prefix=GNU
// RUN: %clang_cc1 -emit-llvm -triple x86_64-windows-gnu -fdeclspec -fvisibility=hidden -o - %s | FileCheck %s --check-prefix=GNU

// RUN: %clang_cc1 -emit-llvm-only -verify -triple x86_64-windows-gnu -fdeclspec -DERR1 -o - %s
// RUN: %clang_cc1 -emit-llvm-only -verify -triple x86_64-windows-gnu -fdeclspec -fvisibility=hidden -DERR1 -o - %s
// RUN: %clang_cc1 -emit-llvm-only -verify -triple x86_64-windows-gnu -fdeclspec -DERR2 -o - %s

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)
#define USE(func) void CONCAT(use, __LINE__)() { func(); }

#define HIDDEN __attribute__((visibility("hidden")))
#define PROTECTED __attribute__((visibility("protected")))
#define DEFAULT __attribute__((visibility("default")))

// MSVC-DAG: declare dllimport void @"?bar@foo@@QEAAXXZ"(
// GNU-DAG: define linkonce_odr hidden void @_ZN3foo3barEv(

struct __attribute__((dllimport)) foo {
  void bar() {}
};
void zed(foo *p) { p->bar(); }

// MSVC-DAG: define dso_local dllexport void @"?exported@@YAXXZ"(
// GNU-DAG: define dso_local dllexport void @_Z8exportedv(
__attribute__((dllexport)) void exported() {}

// MSVC-DAG: define weak_odr dso_local dllexport void @"?exported_inline@@YAXXZ"(
// GNU-DAG: define weak_odr dso_local dllexport void @_Z15exported_inlinev(
__declspec(dllexport) inline void exported_inline() {}
USE(exported_inline)

#if defined(ERR1)
// expected-error@+1 {{non-default visibility cannot be applied to 'dllimport' declaration}}
__attribute__((dllimport)) HIDDEN void imported_hidden();

__attribute__((dllexport)) DEFAULT void exported_default() {}
// expected-error@+1 {{non-default visibility cannot be applied to 'dllexport' declaration}}
__attribute__((dllexport)) HIDDEN void exported_hidden() { imported_hidden(); }
#elif defined(ERR2)
struct PROTECTED C {
  // expected-error@+1 {{non-default visibility cannot be applied to 'dllexport' declaration}}
  __attribute__((dllexport)) void exported_protected() {}
};
#endif
