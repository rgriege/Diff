template <class T>
void LCS_compute_table(T* x, T* y, Table<int>& table)
{
    for (Table<int>::size_type i = 0; i < table.height; i++)
        table[i][0] = 0;
    for (Table<int>::size_type j = 0; j < table.width; j++)
        table[0][j] = 0;
    for (Table<int>::size_type i = 1; i < table.height; ++i) {
        for (Table<int>::size_type j = 1; j < table.width; ++j) {
            if (x[i-1] == y[j-1]) {
                table[i][j] = table[i-1][j-1] + 1;
            } else
                table[i][j] = std::max(table[i][j-1], table[i-1][j]);
        }
    }
}

template <class T>
void LCS_compute_table(BlockedData<T>& x, BlockedData<T>& y, int* table)
{
    int cx_len = x.length() + 1;
    int cy_len = y.length() + 1;
    for (int i = 0; i <= x.length(); i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y.length(); j++)
        table[j] = 0;
    for (int i = 1; i <= x.length(); i++) {
        for (int j = 1; j <= y.length(); j++) {
            if (x[i-1] == y[j-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

int LCS_length(const Table<int>& table)
{
    return table[table.height-1][table.width-1];
}

template <class T>
void LCS_print_table(int* C, BlockedData<T>& x, BlockedData<T>& y, std::ostream& out)
{
    out << "  0 ";
    for (int j = 0; j < y.length(); j++)
        out << y[j] << " ";
    out << std::endl;
    for (int i = 0; i <= x.length(); i++) {
        out << (i == 0 ? '0' : x[i-1]) << " ";
        for (int j = 0; j <= y.length(); j++) {
            out << C[i*(y.length()+1)+j] << " ";
        }
        out << std::endl;
    }
}

enum OutputMode {
    NORMAL,
    INSERTION,
    DELETION,
};

void switch_from_mode(OutputMode mode, std::ostream& out)
{
    if (mode == INSERTION)
        out << "+";
    else if (mode == DELETION)
        out << "-";
    out << std::endl;
}

template <class T>
void LCS_read_helper(int*& C, BlockedData<T>& x, BlockedData<T>& y, int i, int j,
                     std::ostream& out, OutputMode mode)
{
    if (i == 0 || j == 0) {
        return;
    } else if (x[i-1] == y[j-1]) {
        if (mode != NORMAL)
            switch_from_mode(mode, out);
        out << x[i-1];
        LCS_read_helper(C, x, y, i-1, j-1, out, NORMAL);
    } else {
        int cy_len = y.length() + 1;
        if (C[i*cy_len+(j-1)] > C[(i-1)*cy_len+j]) {
            if (mode != INSERTION)
                switch_from_mode(mode, out);
            out << y[j-1];
            LCS_read_helper(C, x, y, i, j-1, out, INSERTION);
        } else {
            if (mode != DELETION)
                switch_from_mode(mode, out);
            out << x[i-1];
            LCS_read_helper(C, x, y, i-1, j, out, DELETION);
        }
    }
}

template <class T>
void LCS_read(int* C, BlockedData<T>& x, BlockedData<T>& y, std::ostream& out)
{
    LCS_read_helper(C, x, y, x.length(), y.length(), out, NORMAL);
}
