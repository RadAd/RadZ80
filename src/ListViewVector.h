#pragma once
#include <Windows.h>

class ListViewVector
{
public:
    typedef int index_type;

    ListViewVector(HWND hWndListView)
        : m_hWndListView(hWndListView)
    {
    }

    size_t size() const
    {
        return ListView_GetItemCount(m_hWndListView);
    }

    void erase(const index_type nItem)
    {
        ListView_DeleteItem(m_hWndListView, nItem);
    }

protected:
    HWND m_hWndListView;
};

class ListViewLParamVector : public ListViewVector
{
public:
    using ListViewVector::ListViewVector;
    typedef LPARAM real_value_type;

    LPARAM get(const index_type nItem) const
    {
        return ListView_GetItemParam(m_hWndListView, nItem);
    }

    void set(const index_type nItem, const LPARAM lParam)
    {
        ListView_SetItemParam(m_hWndListView, nItem, lParam);
    }
};

template<class C>
class RadVectorAdaptor
{
public:
    typedef C Container;
    typedef typename C::real_value_type real_value_type;
    typedef typename C::index_type index_type;

    RadVectorAdaptor(Container& v)
        : m_container(v)
    {
    }

    class ConstValueProxy
    {
    public:
        ConstValueProxy(const Container& v, const index_type nItem)
            : m_container(v), m_nItem(nItem)
        {
        }

        operator real_value_type() const
        {
            return m_container.get(m_nItem);
        }

    private:
        const Container& m_container;
        index_type m_nItem;
    };

    class ValueProxy
    {
    public:
        ValueProxy(Container& v, const index_type nItem)
            : m_container(v), m_nItem(nItem)
        {
        }

#if 0
        operator ConstValueProxy() const
        {
            return { m_v, m_nItem };
        }
#endif

        operator real_value_type() const
        {
            return m_container.get(m_nItem);
        }

        ValueProxy& operator=(real_value_type lParam)
        {
            m_container.set(m_nItem, lParam);
            return *this;
        }

    protected:
        Container& m_container;
        index_type m_nItem;
    };

    ValueProxy operator[](int nItem)
    {
        return ValueProxy(*this, nItem);
    }

    ConstValueProxy operator[](int nItem) const
    {
        return ConstValueProxy(*this, nItem);
    }

    class Iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = real_value_type;
        using difference_type = int;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(Container& v, const index_type nItem) noexcept
            : m_container(v), m_nItem(nItem)
        {
        }

        index_type index() const noexcept
        {
            return m_nItem;
        }

        Iterator& operator=(Iterator i) noexcept
        {
            _ASSERTE(&m_container == &i.m_container);
            m_nItem = i.m_nItem;
            return *this;
        }

        Iterator& operator++() noexcept
        {
            ++m_nItem;
            return *this;
        }

        Iterator operator++(int) noexcept
        {
            ListViewIterator o(*this);
            ++o.m_nItem;
            return o;
        }

        Iterator operator+(int i) noexcept
        {
            return { m_container, m_nItem + i };
        }

        Iterator operator-(int i) noexcept
        {
            return { m_container, m_nItem - i };
        }

        Iterator& operator+=(int i) noexcept
        {
            m_nItem += i;
            return *this;
        }

        Iterator& operator-=(int i) noexcept
        {
            m_nItem -= i;
            return *this;
        }

        ValueProxy operator*() const
        {
            return { m_container, m_nItem };
        }

        difference_type operator-(const Iterator& b) const noexcept
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem - b.m_nItem;
        }

        difference_type operator-(const Iterator& b) // Not sure why it needs to be non-const as well
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem - b.m_nItem;
        }

        bool operator==(const Iterator b) const
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem == b.m_nItem;
        }

        bool operator!=(const Iterator b) const
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem != b.m_nItem;
        }

        bool operator>(const Iterator b) const
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem > b.m_nItem;
        }

        bool operator<(const Iterator b) const
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem < b.m_nItem;
        }

        bool operator>=(const Iterator b) const
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem >= b.m_nItem;
        }

        bool operator<=(const Iterator b) const
        {
            _ASSERTE(&m_container == &b.m_container);
            return m_nItem <= b.m_nItem;
        }

    protected:
        Container& m_container;
        index_type m_nItem;
    };

    Iterator begin()
    {
        return { m_container, 0 };
    }

    Iterator end()
    {
        return { m_container, index_type(m_container.size()) };
    }

    void erase(Iterator i)
    {
        m_container.erase(i.index());
    }

private:
    Container& m_container;
};
