#pragma once 
#include "assert.hpp"
#include <cstdint>
#if __cplusplus >= 201703L
#include <optional>
#endif
#include <type_traits>
#include <utility>
namespace ezr
{
    //struct for handling return values by providing (almost?) no way to retrieve a value without error checking
    template<typename T, typename E>
    class result
    {
        T data;
        E error;
        uint8_t is_valid; //0 if error, 1 if warning, 2 if total success
    public:
        /* 
        retrieve the underlying type or fail otherwise
        */
        [[nodiscard]] T&& value() &&
        {
            EZR_ASSERT(is_valid, "tried to unwrap error value");
            return std::move(data);
        }
        [[nodiscard]] T& value() &
        {
            EZR_ASSERT(is_valid, "tried to unwrap error value");
            return data;
        }
        [[nodiscard]] const T& value() const&
        {
            EZR_ASSERT(is_valid, "tried to unwrap error value");
            return data;
        }

        [[nodiscard]] T&& operator*() &&
        {
            return value();
        }
        [[nodiscard]] T& operator*() &
        {
            return value();
        }
        [[nodiscard]] const T& operator*() const&
        {
            return value();
        }

        /*
        retrieve the underlying type or a specified default value
        */
        [[nodiscard]] T&& value_or(T&& def) &&
        {
            return is_valid ? std::move(data) : std::move(def);
        }
        [[nodiscard]] T& value_or(T& def) &
        {
            return is_valid ? data : def;
        }
        [[nodiscard]] const T& value_or(const T& def) const&
        {
            return is_valid ? data : def;
        }


        /*
        checks for validity
        */
        [[nodiscard]] bool is_ok() const
        {
            return is_valid;
        }
        [[nodiscard]] bool has_warning() const
        {
            return is_valid == 1;
        }
        [[nodiscard]] bool is_err() const
        {
            return !is_valid;
        }
        [[nodiscard]] operator bool() const
        {
            return is_valid;
        }

        /*
        handle error or success value with custom function
        */
        template<typename R, typename Valid, typename InValid>
        R handle(Valid&& valid_fn, InValid&& invalid_fn) &&
        {
            static_assert(std::is_invocable_r_v<R, Valid, T&&>);
            static_assert(std::is_invocable_r_v<R, InValid, E&&>);
            return is_valid ? valid_fn(std::move(data)) : invalid_fn(std::move(error));
        }
        template<typename R, typename Valid, typename InValid>
        R handle(Valid&& valid_fn, InValid&& invalid_fn) &
        {
            static_assert(std::is_invocable_r_v<R, Valid, T&>);
            static_assert(std::is_invocable_r_v<R, InValid, E&>);
            return is_valid ? valid_fn(data) : invalid_fn(error);
        }
        template<typename R, typename Valid, typename InValid>
        R handle(Valid&& valid_fn, InValid&& invalid_fn) const&
        {
            static_assert(std::is_invocable_r_v<R, Valid, const T&>);
            static_assert(std::is_invocable_r_v<R, InValid, const E&>);
            return is_valid ? valid_fn(data) : invalid_fn(error);
        }
        /*
        make a `result`
        */
        static result ok(T&& value)
        {
            result r;
            r.data = std::forward<T>(value);
            r.is_valid = 2;
            return r;
        }
        static result warn(T&& value, E&& error)
        {
            result r;
            r.data = std::forward<T>(value);
            r.is_valid = 1;
            r.error = std::forward<E>(error);
            return r;
        }
        static result err(E&& error)
        {
            result r;
            r.error = std::forward<E>(error);
            r.is_valid = 0;
            return r;
        }

        #if __cplusplus >= 201703L
        std::optional<T> to_optional() &&
        {
            if(is_valid) return std::make_optional(std::move(data));
            return std::nullopt;
        }
        std::optional<T> to_optional() &
        {
            if(is_valid) return std::make_optional(data);
            return std::nullopt;
        }
        const std::optional<T> to_optional() const&
        {
            if(is_valid) return std::make_optional(data);
            return std::nullopt;
        }
        #endif
    };
}
