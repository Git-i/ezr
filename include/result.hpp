#pragma once
#include "assert.hpp"
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>



namespace ezr
{
    template<typename ErrorTy>
    class error_logger{};

    template<>
    class error_logger<std::string>
    {
        static std::string get_message(const std::string& error) { return error; }
    };

    template <typename T>
    using logged_error_t = decltype(error_logger<T>::get_message(std::declval<const T&>()));

    template <typename T, typename = std::void_t<>>
    struct has_error_logger : std::false_type{};

    template <typename T>
    struct has_error_logger<T, std::void_t<logged_error_t<T>>>:
                            std::true_type{};

    template <typename T>
    inline constexpr bool has_error_logger_v = has_error_logger<T>::value;
    /*
    class for constructing results with long type names
    ezr::result</ *long stuff* />  error = ezr::err<ErrorTy>(val);
    */
    template <typename T>
    class err
    {
        T error;
        template<typename R, typename E> friend class result;
    public:
        err(T&& val) : error(val) {}
        err(const T& val) : error(val) {}
    };
    template <typename T>
    class ok
    {
        T val;
        template<typename R, typename E> friend class result;
    public:
        ok(T&& val) : val(val) {}
        ok(const T& val) : val(val) {}
    };
    //struct for handling return values by providing (almost?) no way to retrieve a value without error checking
    template<typename T, typename E>
    class result
    {
        union
        {
            T data;
            E error;
        };
        uint8_t is_valid; //0 if error, 1 if warning, 2 if total success
    public:
        result() : error(), is_valid(0) {}
        result(const result& r)
        {
            if (r.is_valid) new(&data) T(r.data);
            else new(&error) E(r.error);
            is_valid = r.is_valid;
        }
        result(result&& r) noexcept
        {
            if (r.is_valid) new(&data) T(std::move(r.data));
            else new(&error) E(std::move(r.error));
            is_valid = r.is_valid;
        }
        using ValidTy = T;
        using ErrorTy = E;
        /*
        retrieve the underlying type or fail otherwise
        */
        [[nodiscard]] T&& value() &&
        {
            if constexpr (has_error_logger_v<E>)
            {
                EZR_ASSERT(is_valid, ("Operation failed: " + error_logger<E>::get_message()).c_str());
            }
            EZR_ASSERT(is_valid, "tried to unwrap error value");
            return std::move(data);
        }
        [[nodiscard]] T& value() &
        {
            if constexpr (has_error_logger_v<E>)
            {
                EZR_ASSERT(is_valid, ("Operation failed: " + error_logger<E>::get_message()).c_str());
            }
            EZR_ASSERT(is_valid, "tried to unwrap error value");
            return data;
        }
        [[nodiscard]] const T& value() const&
        {
            if constexpr (has_error_logger_v<E>)
            {
                EZR_ASSERT(is_valid, ("Operation failed: " + error_logger<E>::get_message()).c_str());
            }
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

        [[nodiscard]] E&& err() &&
        {
            EZR_ASSERT(!is_valid, "tried to retrieve non-existent error");
            return std::move(error);
        }
        [[nodiscard]] E& err() &
        {
            EZR_ASSERT(!is_valid, "tried to retrieve non-existent error");
            return error;
        }
        [[nodiscard]] const E& err() const&
        {
            EZR_ASSERT(!is_valid, "tried to retrieve non-existent error");
            return error;
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
        [[nodiscard]] explicit operator bool() const
        {
            return is_valid;
        }

        /*
        handle error or success value with custom function
        */
        template<typename Valid, typename InValid>
        auto handle(Valid&& valid_fn, InValid&& invalid_fn) &&
        {
            static_assert(std::is_same_v<
                std::invoke_result_t<Valid, T&&>, 
                std::invoke_result_t<InValid, E&&>>);
            return is_valid ? valid_fn(std::move(data)) : invalid_fn(std::move(error));
        }
        template<typename Valid, typename InValid>
        auto handle(Valid&& valid_fn, InValid&& invalid_fn) &
        {
            static_assert(std::is_same_v<
                std::invoke_result_t<Valid, T&>, 
                std::invoke_result_t<InValid, E&>>);
            return is_valid ? valid_fn(data) : invalid_fn(error);
        }
        template<typename Valid, typename InValid>
        auto handle(Valid&& valid_fn, InValid&& invalid_fn) const&
        {
            static_assert(std::is_same_v<
                std::invoke_result_t<Valid, const T&>, 
                std::invoke_result_t<InValid, const E&>>);
            return is_valid ? valid_fn(data) : invalid_fn(error);
        }

        template<typename Valid>
        auto if_valid(Valid&& valid_fn) &&
        {
            return std::invoke(valid_fn, std::move(data));
        }
        template<typename Valid>
        auto if_valid(Valid&& valid_fn) &
        {
            return std::invoke(valid_fn, data);
        }
        template<typename Valid>
        auto if_valid(Valid&& valid_fn) const&
        {
            return std::invoke(valid_fn, data);
        }
        /*
        perform an operation on a value if its valid, and return the option again
        */
        template<typename Valid>
        auto transform(Valid&& valid_fn) &&
        {
            static_assert(std::is_invocable_v<Valid, T&&>);
            using R = std::invoke_result_t<Valid, T&&>;
            if(is_valid)
            {
                if constexpr (std::is_void_v<R>)
                {
                    valid_fn(std::move(data));
                    return result<R, E>();
                }
                else
                {
                    return result<R, E>::ok(valid_fn(std::move(data)));
                }
            }
            else {
                return result<R, E>::err(std::move(error));
            }

        }
        template<typename Valid>
        auto transform(Valid&& valid_fn) &
        {
            static_assert(std::is_invocable_v<Valid, T&>);
            using R = std::invoke_result_t<Valid, T&>;
            if(is_valid)
            {
                if constexpr (std::is_void_v<R>)
                {
                    valid_fn(data);
                    return result<R, E>();
                }
                else
                {
                    return result<R, E>::ok(valid_fn(data));
                }
            }
            else {
                return result<R, E>::err(error);
            }

        }
        template<typename Valid>
        auto transform(Valid&& valid_fn) const&
        {
            static_assert(std::is_invocable_v<Valid, const T&>);
            using R = std::invoke_result<Valid, const T&>;
            if(is_valid)
            {
                if constexpr (std::is_void_v<R>)
                {
                    valid_fn(data);
                    return result<R, E>();
                }
                else
                {
                    return result<R, E>::ok(valid_fn(data));
                }
            }
            else {
                return result<R, E>::err(error);
            }

        }
        /*
        make a `result`
        */
        template<typename... Params>
        static result ok(Params&&... value)
        {
            result r;
            new (&r.data) T(std::forward<Params>(value)...);
            r.is_valid = 2;
            return r;
        }
        template<typename... Params>
        static result err(Params&&... error)
        {
            result r;
            new (&r.error) E(std::forward<Params>(error)...);
            r.is_valid = 0;
            return r;
        }

        result(ezr::err<E>&& err_obj)
        {
            is_valid = 0;
            new (&error) E(err_obj.error);
        }
        result(ezr::ok<T>&& ok_obj)
        {
            is_valid = 2;
            new (&data) T(ok_obj.val);
        }
        result(T&& ok_obj)
        {
            is_valid = 2;
            new (&data) T(std::move(ok_obj));
        }
        result(const T& ok_obj)
        {
            is_valid = 2;
            new(&data) T(ok_obj);
        }
        ~result()
        {
            if(is_valid == 0) error.~E();
            else data.~T();
        }
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
        std::optional<T> to_optional() const&
        {
            if(is_valid) return std::make_optional(data);
            return std::nullopt;
        }
    };

}
