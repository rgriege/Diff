#ifndef __INDIRECT_ROW_TABLE_H__
#define __INDIRECT_ROW_TABLE_H__

#include <memory>

template <class T, class ColumnAllocator = std::allocator<T> >
class TableRow {

    typedef T value_type;
    typedef T* pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;

public:
    TableRow(size_type _cols, const_reference _val = value_type(), const ColumnAllocator& _a = ColumnAllocator())
        : a(_a), cols(_cols), data(a.allocate(cols))
    {
        for (pointer p = data; p < data + cols; ++p)
            a.construct(p, _val);
    }

    ~TableRow()
    {
        for (pointer p = data; p < data + cols; ++p)
            a.destroy(p);
        a.deallocate(data, cols);
    }

    reference operator[](int j) { return data[j]; }
    const_reference operator[](int j) const { return data[j]; }

    operator pointer() { return data; }

private:
    ColumnAllocator a;
    const size_type cols;
    pointer data;
};

template <class T, class ColumnAllocator = std::allocator<T>, class RowAllocator = std::allocator<TableRow<T, ColumnAllocator> > >
class IndirectRowTable {

    typedef T value_type;
    typedef T* pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;

public:

   IndirectRowTable(size_type _rows, size_type _cols, const_reference _val = value_type(),
         const RowAllocator& _ra = RowAllocator(),
         const ColumnAllocator& _ca = ColumnAllocator())
        : rows(_rows), cols(_cols), ra(_ra)
    {
        data = ra.allocate(rows);
        for (TableRow<T, ColumnAllocator>* p = data; p < data + rows; ++p)
            ra.construct(p, rows, _val, _ca);
    }

    ~IndirectRowTable()
    {
        for (TableRow<T, ColumnAllocator>* p = data; p < data + rows; ++p)
            ra.destroy(p);
        ra.deallocate(data, rows);
    }

    TableRow<T, ColumnAllocator>& operator[](int i) { return data[i]; }
    const TableRow<T, ColumnAllocator>& operator[](int i) const { return data[i]; }

    size_type width() const { return cols; }
    size_type height() const { return rows; }

private:
    size_type num_elements() { return rows*cols; }

    const size_type rows;
    const size_type cols;

    RowAllocator ra;
    TableRow<T, ColumnAllocator>* data;
};

#endif