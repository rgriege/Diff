
int* LCS_create_table(int x_len, int y_len)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    return new int[cx_len*cy_len];
}

template <class T>
void LCS_compute_table_ij_basic(T* x, size_t x_len, T y, size_t y_len, int* table)
{
    int cols = y_len + 1;
    for (size_t i = 0; i <= x_len; i++)
        table[i*cols] = 0;
    for (size_t j = 0; j <= y_len; j++)
        table[j] = 0;
    for (size_t i = 1; i <= x_len; ++i) {
        for (size_t j = 1; j <= y_len; ++j) {
            if (x[i-1] == y[j-1])
                table[i*cols+j] = table[(i-1)*cols+(j-1)] + 1;
            else
                table[i*cols+j] = std::max(table[i*cols+(j-1)], table[(i-1)*cols+j]);
        }
    }
}

template <class T>
void LCS_compute_table_ji_basic(T* x, int x_len, T* y, int y_len, int* table)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int j = 1; j <= y_len; ++j) {
        for (int i = 1; i <= x_len; ++i) {
            if (x[i-1] == y[j-1])
                table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
            else
                table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
        }
    }
}

template <class T>
void LCS_compute_table_ijij(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int ii = 1; ii <= x_len; ii += b) {
        int imax = std::min(ii+b, x_len);
        for (int jj = 1; jj <= y_len; jj += b) {
            int jmax = std::min(jj+b, y_len);
            for (int i = ii; i <= imax; ++i) {
                for (int j = jj; j <= jmax; ++j) {
                    if (x[i-1] == y[j-1])
                        table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                    else
                        table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
                }
            }
        }
    }
}

template <class T>
void LCS_compute_table_jiji(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int jj = 1; jj <= y_len; jj += b) {
        int jmax = std::min(jj+b, y_len);
        for (int ii = 1; ii <= x_len; ii += b) {
            int imax = std::min(ii+b, x_len);
            for (int j = jj; j <= jmax; ++j) {
                for (int i = ii; i <= imax; ++i) {
                    if (x[i-1] == y[j-1])
                        table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                    else
                        table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
                }
            }
        }
    }
}

template <class T>
void LCS_compute_table_jij(T* x, int x_len, T* y, int y_len, int* table, int b)
{
    int cx_len = x_len + 1;
    int cy_len = y_len + 1;
    for (int i = 0; i <= x_len; i++)
        table[i*cy_len] = 0;
    for (int j = 0; j <= y_len; j++)
        table[j] = 0;
    for (int jj = 1; jj <= y_len; jj += b) {
        int jmax = std::min(jj+b, y_len);
        for (int i = 1; i <= x_len; ++i) {
            for (int j = jj; j <= jmax; ++j) {
                if (x[i-1] == y[j-1])
                    table[i*cy_len+j] = table[(i-1)*cy_len+(j-1)] + 1;
                else
                    table[i*cy_len+j] = std::max(table[i*cy_len+(j-1)], table[(i-1)*cy_len+j]);
            }
        }
    }
}

int LCS_length(int* table, int cx_len, int cy_len)
{
    return table[cx_len*cy_len-1];
}

template <class T>
void LCS_read_helper(int*& table, T*& x, T*& y, const int& cy_len, int i, int j,
                     std::ostream& out, OutputMode mode)
{
    if (i == 0 || j == 0) {
        return;
    } else if (x[i-1] == y[j-1]) {
        if (mode != NORMAL)
            switch_from_mode(mode, out);
        out << x[i-1];
        LCS_read_helper(table, x, y, cy_len, i-1, j-1, out, NORMAL);
    } else {
        if (table[i*cy_len+(j-1)] > table[(i-1)*cy_len+j]) {
            if (mode != INSERTION)
                switch_from_mode(mode, out);
            out << y[j-1];
            LCS_read_helper(table, x, y, cy_len, i, j-1, out, INSERTION);
        } else {
            if (mode != DELETION)
                switch_from_mode(mode, out);
            out << x[i-1];
            LCS_read_helper(table, x, y, cy_len, i-1, j, out, DELETION);
        }
    }
}

void LCS_read(int* table, const char* x, const char* y, std::ostream& out)
{
    LCS_read_helper(table, x, y, strlen(y)+1, strlen(x), strlen(y), out, NORMAL);
}