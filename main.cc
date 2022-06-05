/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDE_COROUTINE_HACK_HPP_87B5F3B0_F8BE_49D4_A4B3_2783FF063BD4
#define INCLUDE_COROUTINE_HACK_HPP_87B5F3B0_F8BE_49D4_A4B3_2783FF063BD4

#include <utility>

// >>> libstdc++ coroutine for clang
#define __cpp_impl_coroutine 1
# include <coroutine>
#undef  __cpp_impl_coroutine
namespace std::inline experimental
{
    using std::coroutine_traits;
    using std::coroutine_handle;
}
// <<<

#endif/*INCLUDE_COROUTINE_HACK_HPP_87B5F3B0_F8BE_49D4_A4B3_2783FF063BD4*/

/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDE_AUXILIARY_HPP_C0F6A1AF_FB18_4AA4_BAF9_6378F166F394
#define INCLUDE_AUXILIARY_HPP_C0F6A1AF_FB18_4AA4_BAF9_6378F166F394

#include <tuple>
#include <iosfwd>
#include <cstdint>

namespace aux
{
    inline namespace tuple
    {
        template <size_t...> struct seq { };
        template <size_t N, size_t... I> struct gen_seq : gen_seq<N-1, N-1, I...> { };
        template <size_t... I> struct gen_seq<0, I...> : seq<I...> { };
        template <class Ch, class T, size_t... I>
        void print(std::basic_ostream<Ch>& output, T const& t, seq<I...>) noexcept {
            using swallow = int[];
            (void) swallow{0, (void(output << (I==0 ? "" : ", ") << std::get<I>(t)), 0)...};
        }
        template <class Ch, class... Args>
        auto& operator<<(std::basic_ostream<Ch>& output, std::tuple<Args...> const& t) noexcept {
            output << '(';
            print(output, t, gen_seq<sizeof...(Args)>());
            output << ')';
            return output;
        }
    } // ::aux::tuple
    inline namespace coroutine
    {
        template <class T>
        struct filament {
            struct promise_type {
                void unhandled_exception() { throw; }
                auto get_return_object() noexcept { return filament{*this}; }
                auto initial_suspend() noexcept { return std::suspend_always{}; }
                auto final_suspend() noexcept { return std::suspend_always{}; }
                auto yield_value(T value) noexcept {
                    this->result = value;
                    return std::suspend_always{};
                }
                auto return_void() noexcept { }
                T result;
            };
            filament() = delete;
            filament(filament const&) = delete;
            filament(filament&& src)
                : handle{std::exchange(src.handle, nullptr)}
            {
            }
            filament& operator=(filament const&) = delete;
            filament& operator=(filament&& src) {
                if (this != &src) {
                    this->handle = std::exchange(src.handle, nullptr);
                }
                return *this;
            }
            ~filament() noexcept { if (this->handle) this->handle.destroy(); }
            T step() const noexcept {
                this->handle.resume();
                return this->handle.promise().result;
            }
        private:
            explicit filament(promise_type& p) noexcept
                : handle{std::coroutine_handle<promise_type>::from_promise(p)}
            {
            }
            std::coroutine_handle<promise_type> handle;
        };
    } // ::aux::coroutine
} // ::aux

#endif/*INCLUDE_AUXILIARY_HPP_C0F6A1AF_FB18_4AA4_BAF9_6378F166F394*/

/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDE_WAYLAND_CLIENT_HELPER_HPP_5EB58011_CE66_477C_BA2C_00259E712521
#define INCLUDE_WAYLAND_CLIENT_HELPER_HPP_5EB58011_CE66_477C_BA2C_00259E712521

#include <concepts>
#include <iosfwd>
#include <memory>
#include <string_view>
#include <wayland-client.h>

namespace wayland_client_helper
{
    template <class> constexpr std::nullptr_t wl_interface_ptr = nullptr;
#define INTERN_WL_INTERFACE(wl_client)                                  \
    template <> constexpr wl_interface const* wl_interface_ptr<wl_client> = &wl_client##_interface;
    INTERN_WL_INTERFACE(wl_display);
    INTERN_WL_INTERFACE(wl_registry);
    INTERN_WL_INTERFACE(wl_compositor);
    INTERN_WL_INTERFACE(wl_shell);
    INTERN_WL_INTERFACE(wl_seat);
    INTERN_WL_INTERFACE(wl_keyboard);
    INTERN_WL_INTERFACE(wl_pointer);
    INTERN_WL_INTERFACE(wl_touch);
    INTERN_WL_INTERFACE(wl_shm);
    INTERN_WL_INTERFACE(wl_surface);
    INTERN_WL_INTERFACE(wl_shell_surface);
    INTERN_WL_INTERFACE(wl_buffer);
    INTERN_WL_INTERFACE(wl_shm_pool);
    INTERN_WL_INTERFACE(wl_callback);
    INTERN_WL_INTERFACE(wl_output);

    template <class T>
    concept WL_CLIENT_CONCEPT = std::same_as<decltype (wl_interface_ptr<T>),
                                             wl_interface const* const>;

    template <class Ch, WL_CLIENT_CONCEPT T>
    auto& operator<<(std::basic_ostream<Ch>& output, T const* ptr) noexcept {
        return output << static_cast<void const*>(ptr)
                      << '[' << wl_interface_ptr<T>->name << ']';
    }

    template <WL_CLIENT_CONCEPT T>
    [[nodiscard]] auto attach_unique(T* ptr) noexcept {
        static constexpr auto deleter = [](T* ptr) noexcept -> void {
            if constexpr      (wl_interface_ptr<T> == &wl_display_interface) {
                wl_display_disconnect(ptr);
            }
            else if constexpr (wl_interface_ptr<T> == &wl_keyboard_interface) {
                wl_keyboard_release(ptr);
            }
            else if constexpr (wl_interface_ptr<T> == &wl_pointer_interface) {
                wl_pointer_release(ptr);
            }
            else if constexpr (wl_interface_ptr<T> == &wl_touch_interface) {
                wl_touch_release(ptr);
            }
            else {
                wl_proxy_destroy(reinterpret_cast<wl_proxy*>(ptr));
            }
        };
        return std::unique_ptr<T, decltype (deleter)>(ptr, deleter);
    }

    template <WL_CLIENT_CONCEPT T>
    using wl_client_ptr = decltype (attach_unique(std::declval<T>()));

} // ::wayland_client_helper

#endif/*INCLUDE_WAYLAND_CLIENT_HELPER_HPP_5EB58011_CE66_477C_BA2C_00259E712521*/

/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <thread>
#include <map>
#include <string>

int main() {
    using namespace std::literals;
    using namespace aux;
    using namespace wayland_client_helper;

    std::map<uint32_t, std::pair<std::string, uint32_t>> globals;

    auto display = attach_unique(wl_display_connect(nullptr));
    if (!display) {
        std::cerr << "wl_display_connect failed..." << std::endl;
        return -1;
    }
    auto registry = attach_unique(wl_display_get_registry(display.get()));
    if (!registry) {
        std::cerr << "wl_display_get_registry failed..." << std::endl;
        return -1;
    }
    wl_registry_listener registry_listener = {
        .global = [](void* data, auto... args) noexcept {
            std::cerr << std::tuple{args...} << std::endl;
            auto globals_ptr = reinterpret_cast<decltype (globals)*>(data);
            auto [wl_registry, name, interface, version] = std::tuple{args...};
            globals_ptr->insert({name, {std::string(interface), version}});
        },
        .global_remove = [](void* data, auto... args) noexcept {
            std::cerr << std::tuple{args...} << std::endl;
            auto globals_ptr = reinterpret_cast<decltype (globals)*>(data);
            auto [wl_registry, name] = std::tuple{args...};
            globals_ptr->erase(name);
        },
    };
    if (wl_registry_add_listener(registry.get(), &registry_listener, &globals) != 0) {
        std::cerr << "wl_registry_add_listener failed..." << std::endl;
        return -1;
    }
    for (;;) {
        wl_display_roundtrip(display.get());
        std::this_thread::sleep_for(1s);
        std::cout << globals.size() << std::endl;
    }

    return 0;
}
