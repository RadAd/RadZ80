#pragma once

template <class F, class A>
struct S1
{
    F f;
    A a;

    ~S1()
    {
        f(a);
    }
};

template <class F, class A, class B>
struct S2
{
    F f;
    A a;
    B b;

    ~S2()
    {
        f(a, b);
    }
};

template <class F, class A>
S1<F, A> Auto(F f, A a)
{
    return { f, f(a) };
}

template <class F, class A, class B>
S2<F, A, B> Auto(F f, A a, B b)
{
    return { f, a, f(a, b) };
}

#if 0
#include <memory>

struct free_delete {
    constexpr free_delete() noexcept = default;

    free_delete(const free_delete&) noexcept {}

    void operator()(void* _Ptr) const noexcept /* strengthened */ {
        free(_Ptr);
    }
};

template <class _Ty>
auto unique_malloc(const size_t count = 1, const size_t extra = 0)
{
    const size_t sz = count * sizeof(_Ty) + extra;
    auto p = std::unique_ptr<_Ty, free_delete>((_Ty*) malloc(sz));
    memset(p.get(), 0, sz);
    return p;
}
#endif

