#pragma once
#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>
#include "assert.hpp"
namespace ezr
{
    template <typename T, typename... Ts>
    struct largest_of;

    template <typename T>
    struct largest_of<T> {
        using type = T;
    };

    template <typename T1, typename T2, typename... Ts>
    struct largest_of<T1, T2, Ts...> {
        using type = typename largest_of<
            typename std::conditional<(sizeof(T1) >= sizeof(T2)), T1, T2>::type, 
            Ts...
        >::type;
    };

    template<typename T, std::size_t off, typename... Types>
    struct type_index;

    // Base case: If Types is empty, return -1 (type not found)
    template<typename T, std::size_t off>
    struct type_index<T, off> : std::integral_constant<int, -1> {};

    // Recursive case: If the first type matches T, return 0. Otherwise, increment the index and recurse.
    template<typename T, std::size_t off, typename U, typename... Types>
    struct type_index<T, off, U, Types...> : std::integral_constant<int,
        off == 0 ? (std::is_same_v<T, U> ? 0 : (type_index<T, off, Types...>::value)) :
        type_index<T, off - 1, Types...>::value == -1 ? -1 : type_index<T, off - 1, Types...>::value + 1> {};

    template<std::size_t N>
    struct num { static const constexpr auto value = N; };

   template <class F, std::size_t... Is>
    void for_(F&& func, std::index_sequence<Is...>)
    {
      using expander = int[];
      (void)expander{0, ((void)func(num<Is>{}), 0)...};
    }

    template <std::size_t N, typename F>
    void for_(F&& func)
    {
      for_(func, std::make_index_sequence<N>());
    }

    template <typename T, typename... Ts>
    constexpr bool is_assignable_to = std::disjunction_v<std::is_convertible<Ts, T>...>;

    template <typename T, typename... Ts>
    constexpr bool is_constructible_with = std::disjunction_v<std::is_constructible<Ts, T>...>;

    template<typename iden, typename... params>
    class one_of
    {
        static_assert(std::is_integral_v<iden> || std::is_enum_v<iden>);
        largest_of<params...> storage;
        iden idx;
    public:
        template<typename T>
        bool validate(iden id)
        {
            size_t idx = static_cast<size_t>(id);
            int indices[sizeof...(params)];
            for_<sizeof...(params)>([&](auto i){
                indices[i.value] = type_index<T, i.value, params...>::value;
            });
            bool valid = false;
            for(int i = 0; i < sizeof...(params); i++)
            {
                if(indices[i] == idx) 
                {
                    valid = true;
                    break;
                }
            }
            return valid;
        }
        template<typename T>
        void assign(iden id, T&& value)
        {
            static_assert(is_assignable_to<T, params...>);
            EZR_ASSERT(validate<T>(id), "Invalid Type Specified");
            idx = id;
            (*(T*)(&storage)) = std::forward<T>(value);
        }
        template<typename T>
        void operator=(const std::pair<iden, T&&>& value)
        {
            assign(value.first, value.second);
        }

        template<typename T>
        one_of(iden id, T&& value)
        {
            static_assert(is_constructible_with<T, params...>);
            EZR_ASSERT(validate<T>(id), "Invalid Type Specified"); 
            idx = id;
            void* addr = &storage;
            new (addr) T(std::forward<T>(value));
        }

        template<iden i>
        const std::tuple_element_t<(std::size_t)i, std::tuple<params...>>& get()
        {
            EZR_ASSERT(idx == i, "Invalid Get");
            using return_t = std::tuple_element_t<(std::size_t)i, std::tuple<params...>>;
            return *(return_t*)(&storage);
        }
    };
}