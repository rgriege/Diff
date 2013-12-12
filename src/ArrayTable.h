#ifndef __ARRAY_TABLE_H__
#define __ARRAY_TABLE_H__

template <class T, class Allocator = std::allocator<T> >
class ArrayTable {

    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef size_t size_type;

public:
    ArrayTable(size_type _rows, size_type _cols, const_reference _val = value_type(), Allocator _a = Allocator())
        : rows(_rows), cols(_cols), a(_a), data(a.allocate(rows*cols))
    {
        for (pointer p = data; p < data + num_elements(); ++p)
            a.construct(p, _val);
    }

    ~ArrayTable()
    {
        for (pointer p = data; p < data + num_elements(); ++p)
            a.destroy(p);
        a.deallocate(data, num_elements());
    }

    pointer operator[](size_type row) { return data + row * cols; }
    const_pointer operator[](size_type row) const { return data + row * cols; }

    reference at(size_type row, size_type col) { return data[row*cols+col]; }
    const_reference at(size_type row, size_type col) const { return data[row*cols+col]; }

    size_type width() const { return cols; }
    size_type height() const { return rows; }

private:

    size_type num_elements() { return rows*cols; }

    const size_type rows;
    const size_type cols;
    Allocator a;
    pointer data;
};

#endif