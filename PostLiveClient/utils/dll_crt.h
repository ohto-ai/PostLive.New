#pragma once

#ifndef _OHTOAI_DLLLOADER_H_
#define _OHTOAI_DLLLOADER_H_

#if __cplusplus >= 202002L
// for std::bit_cast
#include <bit>
#else
#include <cstring>
#endif // __cplusplus >= 202002L

#if defined(_WIN32) || defined(_WIN64)
#define _OHTOAI_WINDOWS_PLATFORM_
#elif defined(__linux__)
#define _OHTOAI_LINUX_PLATFORM_
#else
#error "unsupported platform"
#endif

#if defined(_OHTOAI_WINDOWS_PLATFORM_)
#include <Windows.h>
#include <libloaderapi.h>
#elif defined(_OHTOAI_LINUX_PLATFORM_)
#include <dlfcn.h>
#endif

namespace ohtoai {

    namespace detail {
        namespace {
#if __cplusplus >= 202002L
            using std::bit_cast;
#else
            template<typename To, typename From>
            To bit_cast(const From& src) {
                static_assert(sizeof(To) == sizeof(From),
                    "bit_cast requires both types to have the same size");
                To dst;
                std::memcpy(&dst, &src, sizeof(To));
                return dst;
            }
#endif
            struct reg_t {
                size_t val;
                constexpr reg_t() : val(0) {}
                constexpr reg_t(size_t v) : val(v) {}
                template <typename T>
                constexpr reg_t(T v) : val(bit_cast<size_t>(v)) { static_assert(sizeof(v) <= sizeof(val)); }
                template <typename T>
                constexpr operator T() const {
                    static_assert(sizeof(T) <= sizeof(val));
                    return bit_cast<T>(val);
                }
            };
        }

        namespace crt{
#if defined(_OHTOAI_WINDOWS_PLATFORM_)
            using lib_t = HMODULE;
#elif defined(_OHTOAI_LINUX_PLATFORM_)
            using lib_t = void*;
#endif

            struct lib_ {
                lib_t lib;
#if defined(_OHTOAI_WINDOWS_PLATFORM_)
                lib_(const char* name) : lib(LoadLibraryA(name)) {}
                lib_(const wchar_t* name) : lib(LoadLibraryW(name)) {}
#elif defined(_OHTOAI_LINUX_PLATFORM_)
                lib_(const char* name) : lib(dlopen(name, RTLD_NOW)) {}
#endif
                lib_(lib_t lib) : lib(lib) {}
                ~lib_() {
                    if (lib != nullptr)
#if defined(_OHTOAI_WINDOWS_PLATFORM_)
                        FreeLibrary(lib);
#elif defined(_OHTOAI_LINUX_PLATFORM_)
                        dlclose(lib);
#endif
                }
                auto get(const char* name) const {
#if defined(_OHTOAI_WINDOWS_PLATFORM_)
                    return GetProcAddress(lib, name);
#elif defined(_OHTOAI_LINUX_PLATFORM_)
                    return dlsym(lib, name);
#endif
                }

                template <typename T>
                T get(const char* name) const {
                    return reinterpret_cast<T>(get(name));
                }

                static lib_ self() {
#if defined(_OHTOAI_WINDOWS_PLATFORM_)
                    return lib_(GetModuleHandle(nullptr));
#elif defined(_OHTOAI_LINUX_PLATFORM_)
                    Dl_info info;
                    if (dladdr((void*)self, &info)) {
                        return lib_(dlopen(info.dli_fname, RTLD_NOW));
                    }
                    throw "self not found";
#endif
                }

                template <typename... args_t>
                reg_t call(const char* name, args_t... args) {
                    auto func = get(name);
                    if (func != nullptr) {
                        return ((size_t(*)(args_t...))func)(args...);
                    }
                    throw "lib or function not found";
                }

            };
        }
    }

    namespace rt {
        using lib = detail::crt::lib_;
#if defined(_OHTOAI_WINDOWS_PLATFORM_)
        detail::crt::lib_ cruntime("msvcrt");
#elif defined(_OHTOAI_LINUX_PLATFORM_)
        detail::crt::lib_ cruntime("libc");
#endif
    }
}

#undef _OHTOAI_WINDOWS_PLATFORM_
#undef _OHTOAI_LINUX_PLATFORM_
#endif
