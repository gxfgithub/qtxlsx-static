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
#ifndef XLSXUTILITY_H
#define XLSXUTILITY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Xlsx API.  It exists for the convenience
// of the Qt Xlsx.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#include "xlsxglobal.h"
class QPoint;
class QString;
class QStringList;
class QColor;
class QDateTime;
class QTime;

namespace QXlsx {

 int intPow(int x, int p);
 QStringList splitPath(const QString &path);
 QString getRelFilePath(const QString &filePath);

 double datetimeToNumber(const QDateTime &dt, bool is1904=false);
 QDateTime datetimeFromNumber(double num, bool is1904=false);
 double timeToNumber(const QTime &t);

 QPoint xl_cell_to_rowcol(const QString &cell_str);
 QString xl_col_to_name(int col_num);
 int xl_col_name_to_value(const QString &col_str);
 QString xl_rowcol_to_cell(int row, int col, bool row_abs=false, bool col_abs=false);
 QString xl_rowcol_to_cell_fast(int row, int col);

} //QXlsx
#endif // XLSXUTILITY_H
