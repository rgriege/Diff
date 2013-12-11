#ifndef __TABLE_H__
#define __TABLE_H__

template <class T, class ColumnAllocator = std::allocator<T> >
class TableRow {

    typedef T value_type;
    typedef T* pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;

public:
    TableRow(size_type _width, const_reference _val = value_type(), const ColumnAllocator& _a = ColumnAllocator())
        : a(_a), width(_width), data(a.allocate(width))
    {
        for (pointer p = data; p < data + width; ++p)
            a.construct(p, _val);
    }

    ~TableRow()
    {
        for (pointer p = data; p < data + width; ++p)
            a.destroy(p);
        a.deallocate(data, width);
    }

    reference operator[](int j) { return data[j]; }
    const_reference operator[](int j) const { return data[j]; }

    operator pointer() { return data; }

private:
    ColumnAllocator a;
    const size_type width;
    pointer data;
};

template <class T, class ColumnAllocator = std::allocator<T>, class RowAllocator = std::allocator<TableRow<T, ColumnAllocator> > >
class Table {
public:

    typedef T value_type;
    typedef T* pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;

   Table(size_type _width, size_type _height, const_reference _val = value_type(),
         const RowAllocator& _ra = RowAllocator(),
         const ColumnAllocator& _ca = ColumnAllocator())
        : width(_width), height(_height), ra(_ra)
    {
        rows = ra.allocate(height);
        for (TableRow<T, ColumnAllocator>* p = rows; p < rows + height; ++p)
            ra.construct(p, width, _val, _ca);
    }

    ~Table()
    {
        for (TableRow<T, ColumnAllocator>* p = rows; p < rows + height; ++p)
            ra.destroy(p);
        ra.deallocate(rows, height);
    }

    TableRow<T, ColumnAllocator>& operator[](int i) { return rows[i]; }
    const TableRow<T, ColumnAllocator>& operator[](int i) const { return rows[i]; }

    const size_type width;
    const size_type height;

private:
    RowAllocator ra;
    TableRow<T, ColumnAllocator>* rows;
};

#endif