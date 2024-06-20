# ezr
A C++ library providing return value error handling, and hopefully other types (make an issue to requeest a feature). It is inspired by rust and `std::optional`.

## Usage
ezr is single-header, just add `include/` to your build system's include path and `#include "result.hpp"`, there is already a meson.build provided that does this for meson users.

## Example
```cpp
#include "result.hpp"
enum class CreationError
{
    NoMemory = -1, //error
    ObjNerfed = 1, //object created but something else happened
    Success = 0
}
ezr::result<Object*, CreationError> CreateObject()
{
    ...
    //one of
    return ezr::result::ok(obj);
    return ezr::result::err(CreationError::NoMemory);
    return ezr::result::warn(obj, CreationError::ObjNerfed);
}

int main()
{
    Object* obj1 = CreateObject().value();//crash if its invalid
    //Object* obj1 = *CreateObject(); should also be possible
    Object* obj = CreateObject().value_or(obj1);//use obj1 if its invalid
    //if functions object doesnt need to persist
    int a = CreateObject().handle<int>([](Object*&& obj)
    {
        return obj->number;
        //you should delete obj if it matters
    }, [](CreationError&& e)
    {
        return -1;
    });
    //it's also possible to check validity
    auto res = CreateObject();
    if(res) {...}
    if(res.is_err()) {...}
    if(res.has_warning()) {...}
}
```
It is also possible to provide your own assertion macro to handle errors (default is cassert).
## How does it compare
To:

[`std::expected`](https://en.cppreference.com/w/cpp/utility/expected): It doesn't reqire c++ 23.

## Bug Reports or Feature Requests
If there is a feature you would like or bug you want removed please open an issue


