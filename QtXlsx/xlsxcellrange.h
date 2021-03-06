/****************************************************************************
** Copyright (c) 2013-2014 Debao Zhang <hello@debao.me>
** All right reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
** LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
** OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
** WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
****************************************************************************/
#ifndef QXLSX_XLSXCELLRANGE_H
#define QXLSX_XLSXCELLRANGE_H
#include "xlsxglobal.h"

namespace QXlsx {

class CellRange
{
public:
    CellRange();
    CellRange(int firstRow, int firstColumn, int lastRow, int lastColumn);
    CellRange(const QString &range);
    CellRange(const CellRange &other);
    ~CellRange();

    QString toString() const;
    bool isValid() const;
    inline void setFirstRow(int row) { top = row; }
    inline void setLastRow(int row) { bottom = row; }
    inline void setFirstColumn(int col) { left = col; }
    inline void setLastColumn(int col) { right = col; }
    inline int firstRow() const { return top; }
    inline int lastRow() const { return bottom; }
    inline int firstColumn() const { return left; }
    inline int lastColumn() const { return right; }
    inline int rowCount() const { return bottom - top + 1; }
    inline int columnCount() const { return right - left + 1; }

    inline bool operator ==(const CellRange &other) const
    {
        return top==other.top && bottom==other.bottom
                && left == other.left && right == other.right;
    }
    inline bool operator !=(const CellRange &other) const
    {
        return top!=other.top || bottom!=other.bottom
                || left != other.left || right != other.right;
    }
private:
    int top, left, bottom, right;
};

} // namespace QXlsx

Q_DECLARE_TYPEINFO(QXlsx::CellRange, Q_MOVABLE_TYPE);

#endif // QXLSX_XLSXCELLRANGE_H
