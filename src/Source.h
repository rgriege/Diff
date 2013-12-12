#ifndef __SOURCE_H__
#define __SOURCE_H__

template <class T>
struct keep_non_default {
    bool operator() (const T& t) const { return t != T(); }
    typedef T first_argument_type;
    typedef T second_argument_type;
    typedef bool result_type;
};

template <class T, class Allocator = std::allocator<T> >
class Source {

    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef size_t size_type;

public:
    Source(size_type _len, const_reference _val = value_type(), Allocator _a = Allocator())
        : len(_len), a(_a), data(a.allocate(len))
    {
        for (pointer p = data; p < data + len; ++p)
            a.construct(p, _val);
    }

    ~Source()
    {
        for (pointer p = data; p < data + len; ++p)
            a.destroy(p);
        a.deallocate(data, len);
    }

    reference operator[](size_type i) { return data[i]; }
    const_reference operator[](size_type i) const { return data[i]; }

    operator pointer() { return data; }
    operator const_pointer() const { return data; }

    size_type length() const { return len; }

    template <class Predicate = keep_non_default<value_type> >
    void shrink_to_fit(const Predicate& predicate = Predicate())
    {
        /* temporarily preserve the old data */
        pointer old_data = data;
        size_type old_len = len;

        /* find the amount to shrink */
        while (len > 0 && !predicate(data[len-1]))
            --len;

        /* allocate and fill the new space */
        data = a.allocate(len);
        for (size_type i = 0; i < len; ++i)
            a.construct(data + i, old_data[i]);

        /* destroy and deallocate the old space */
        for (size_type i = 0; i < old_len; ++i)
            a.destroy(old_data + i);
        a.deallocate(old_data, old_len);
    }

private:
    size_type len;
    Allocator a;
    pointer data;
};

#endif
