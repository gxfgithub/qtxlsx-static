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
#include "xlsxworkbook.h"
#include "xlsxworkbook_p.h"
#include "xlsxsharedstrings_p.h"
#include "xlsxworksheet.h"
#include "xlsxstyles_p.h"
#include "xlsxformat.h"
#include "xlsxworksheet_p.h"
#include "xlsxformat_p.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QFile>
#include <QBuffer>

namespace QXlsx {

WorkbookPrivate::WorkbookPrivate(Workbook *q) :
    q_ptr(q)
{
    sharedStrings = QSharedPointer<SharedStrings> (new SharedStrings);
    styles = QSharedPointer<Styles>(new Styles);
    theme = QSharedPointer<Theme>(new Theme);

    x_window = 240;
    y_window = 15;
    window_width = 16095;
    window_height = 9660;

    strings_to_numbers_enabled = false;
    date1904 = false;
    defaultDateFormat = QStringLiteral("yyyy-mm-dd");
    activesheetIndex = 0;
    firstsheet = 0;
    table_count = 0;

    last_sheet_index = 0;
    last_sheet_id = 0;
}

Workbook::Workbook() :
    d_ptr(new WorkbookPrivate(this))
{

}

Workbook::~Workbook()
{
    delete d_ptr;
}

bool Workbook::isDate1904() const
{
    Q_D(const Workbook);
    return d->date1904;
}

/*!
  Excel for Windows uses a default epoch of 1900 and Excel
  for Mac uses an epoch of 1904. However, Excel on either
  platform will convert automatically between one system
  and the other. Qt Xlsx stores dates in the 1900 format
  by default.

  \note This function should be called before any date/time
  has been written.
*/
void Workbook::setDate1904(bool date1904)
{
    Q_D(Workbook);
    d->date1904 = date1904;
}

/*
  Enable the worksheet.write() method to convert strings
  to numbers, where possible, using float() in order to avoid
  an Excel warning about "Numbers Stored as Text".

  The default is false
 */
void Workbook::setStringsToNumbersEnabled(bool enable)
{
    Q_D(Workbook);
    d->strings_to_numbers_enabled = enable;
}

bool Workbook::isStringsToNumbersEnabled() const
{
    Q_D(const Workbook);
    return d->strings_to_numbers_enabled;
}

QString Workbook::defaultDateFormat() const
{
    Q_D(const Workbook);
    return d->defaultDateFormat;
}

void Workbook::setDefaultDateFormat(const QString &format)
{
    Q_D(Workbook);
    d->defaultDateFormat = format;
}

/*!
 * \brief Create a defined name in the workbook.
 * \param name The defined name
 * \param formula The cell or range that the defined name refers to.
 * \param comment
 * \param scope The name of one worksheet, or empty which means golbal scope.
 * \return Return false if the name invalid.
 */
bool Workbook::defineName(const QString &name, const QString &formula, const QString &comment, const QString &scope)
{
    Q_D(Workbook);

    //Remove the = sign from the formula if it exists.
    QString formulaString = formula;
    if (formulaString.startsWith(QLatin1Char('=')))
        formulaString = formula.mid(1);

    int id=-1;
    if (!scope.isEmpty()) {
        for (int i=0; i<d->worksheets.size(); ++i) {
            if (d->worksheets[i]->sheetName() == scope) {
                id = d->worksheets[i]->sheetId();
                break;
            }
        }
    }

    d->definedNamesList.append(XlsxDefineNameData(name, formulaString, comment, id));
    return true;
}

Worksheet *Workbook::addWorksheet(const QString &name)
{
    Q_D(Workbook);
    return insertWorkSheet(d->worksheets.size(), name);
}

/*!
 * \internal
 */
QStringList Workbook::worksheetNames() const
{
    Q_D(const Workbook);
    return d->worksheetNames;
}

/*!
 * \internal
 * Used only when load the xlsx file!!
 */
Worksheet *Workbook::addWorksheet(const QString &name, int sheetId)
{
    Q_D(Workbook);
    if (sheetId > d->last_sheet_id)
        d->last_sheet_id = sheetId;

    Worksheet *sheet = new Worksheet(name, sheetId, this);
    d->worksheets.append(QSharedPointer<Worksheet>(sheet));
    d->worksheetNames.append(name);
    return sheet;
}

Worksheet *Workbook::insertWorkSheet(int index, const QString &name)
{
    Q_D(Workbook);
    QString worksheetName = name;
    if (!name.isEmpty()) {
        //If user given an already in-used name, we should not continue any more!
        if (d->worksheetNames.contains(name))
            return 0;
    } else {
        do {
            ++d->last_sheet_index;
            worksheetName = QStringLiteral("Sheet%1").arg(d->last_sheet_index);
        } while (d->worksheetNames.contains(worksheetName));
    }

    ++d->last_sheet_id;
    Worksheet *sheet = new Worksheet(worksheetName, d->last_sheet_id, this);
    d->worksheets.insert(index, QSharedPointer<Worksheet>(sheet));
    d->worksheetNames.insert(index, worksheetName);
    d->activesheetIndex = index;
    return sheet;
}

/*!
 * Returns current active worksheet.
 */
Worksheet *Workbook::activeWorksheet() const
{
    Q_D(const Workbook);
    return d->worksheets[d->activesheetIndex].data();
}

bool Workbook::setActiveWorksheet(int index)
{
    Q_D(Workbook);
    if (index < 0 || index >= d->worksheets.size()) {
        //warning
        return false;
    }
    d->activesheetIndex = index;
    return true;
}

/*!
 * Rename the worksheet at the \a index to \a name.
 */
bool Workbook::renameWorksheet(int index, const QString &name)
{
    Q_D(Workbook);
    //If user given an already in-used name, return false
    for (int i=0; i<d->worksheets.size(); ++i) {
        if (d->worksheets[i]->sheetName() == name)
            return false;
    }

    d->worksheets[index]->setSheetName(name);
    d->worksheetNames[index] = name;
    return true;
}

/*!
 * Remove the worksheet at pos \a index.
 */
bool Workbook::deleteWorksheet(int index)
{
    Q_D(Workbook);
    if (d->worksheets.size() <= 1)
        return false;
    if (index < 0 || index >= d->worksheets.size())
        return false;
    d->worksheets.removeAt(index);
    d->worksheetNames.removeAt(index);
    return true;
}

/*!
 * Moves the worksheet form \a srcIndex to \a distIndex.
 */
bool Workbook::moveWorksheet(int srcIndex, int distIndex)
{
    Q_D(Workbook);
    if (srcIndex == distIndex)
        return false;

    if (srcIndex < 0 || srcIndex >= d->worksheets.size())
        return false;

    QSharedPointer<Worksheet> sheet = d->worksheets.takeAt(srcIndex);
    d->worksheetNames.takeAt(srcIndex);
    if (distIndex >= 0 || distIndex <= d->worksheets.size()) {
        d->worksheets.insert(distIndex, sheet);
        d->worksheetNames.insert(distIndex, sheet->sheetName());
    } else {
        d->worksheets.append(sheet);
        d->worksheetNames.append(sheet->sheetName());
    }
    return true;
}

bool Workbook::copyWorksheet(int index, const QString &newName)
{
    Q_D(Workbook);
    if (index < 0 || index >= d->worksheets.size())
        return false;

    QString worksheetName = newName;
    if (!newName.isEmpty()) {
        //If user given an already in-used name, we should not continue any more!
        if (d->worksheetNames.contains(newName))
            return false;
    } else {
        int copy_index = 1;
        do {
            ++copy_index;
            worksheetName = QStringLiteral("%1(%2)").arg(d->worksheets[index]->sheetName()).arg(copy_index);
        } while (d->worksheetNames.contains(worksheetName));
    }

    ++d->last_sheet_id;
    QSharedPointer<Worksheet> sheet = d->worksheets[index]->copy(worksheetName, d->last_sheet_id);
    d->worksheets.append(sheet);
    d->worksheetNames.append(sheet->sheetName());

    return false;
}

QList<QSharedPointer<Worksheet> > Workbook::worksheets() const
{
    Q_D(const Workbook);
    return d->worksheets;
}

/*!
 * Returns count of worksheets.
 */
int Workbook::worksheetCount() const
{
    Q_D(const Workbook);
    return d->worksheets.count();
}

/*!
 * Returns the sheet object at index \a sheetIndex.
 */
Worksheet *Workbook::worksheet(int index) const
{
    Q_D(const Workbook);
    if (index < 0 || index >= d->worksheets.size())
        return 0;
    return d->worksheets.at(index).data();
}

SharedStrings *Workbook::sharedStrings() const
{
    Q_D(const Workbook);
    return d->sharedStrings.data();
}

Styles *Workbook::styles()
{
    Q_D(Workbook);
    return d->styles.data();
}

Theme *Workbook::theme()
{
    Q_D(Workbook);
    return d->theme.data();
}

QList<QImage> Workbook::images()
{
    Q_D(Workbook);
    return d->images;
}

QList<Drawing *> Workbook::drawings()
{
    Q_D(Workbook);
    return d->drawings;
}

void Workbook::prepareDrawings()
{
    Q_D(Workbook);
    int image_ref_id = 0;
    d->images.clear();
    d->drawings.clear();

    for (int i=0; i<d->worksheets.size(); ++i) {
        QSharedPointer<Worksheet> sheet = d->worksheets[i];
        if (sheet->images().isEmpty()) //No drawing (such as Image, ...)
            continue;

        sheet->clearExtraDrawingInfo();

        //At present, only picture type supported
        for (int idx = 0; idx < sheet->images().size(); ++idx) {
            image_ref_id += 1;
            sheet->prepareImage(idx, image_ref_id);
            d->images.append(sheet->images()[idx]->image);
        }

        d->drawings.append(sheet->drawing());
    }
}

void Workbook::saveToXmlFile(QIODevice *device) const
{
    Q_D(const Workbook);
    d->relationships.clear();

    for (int i=0; i<worksheetCount(); ++i)
        d->relationships.addDocumentRelationship(QStringLiteral("/worksheet"), QStringLiteral("worksheets/sheet%1.xml").arg(i+1));
    d->relationships.addDocumentRelationship(QStringLiteral("/theme"), QStringLiteral("theme/theme1.xml"));
    d->relationships.addDocumentRelationship(QStringLiteral("/styles"), QStringLiteral("styles.xml"));
    if (!sharedStrings()->isEmpty())
        d->relationships.addDocumentRelationship(QStringLiteral("/sharedStrings"), QStringLiteral("sharedStrings.xml"));

    QXmlStreamWriter writer(device);

    writer.writeStartDocument(QStringLiteral("1.0"), true);
    writer.writeStartElement(QStringLiteral("workbook"));
    writer.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
    writer.writeAttribute(QStringLiteral("xmlns:r"), QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"));

    writer.writeEmptyElement(QStringLiteral("fileVersion"));
    writer.writeAttribute(QStringLiteral("appName"), QStringLiteral("xl"));
    writer.writeAttribute(QStringLiteral("lastEdited"), QStringLiteral("4"));
    writer.writeAttribute(QStringLiteral("lowestEdited"), QStringLiteral("4"));
    writer.writeAttribute(QStringLiteral("rupBuild"), QStringLiteral("4505"));
//    writer.writeAttribute(QStringLiteral("codeName"), QStringLiteral("{37E998C4-C9E5-D4B9-71C8-EB1FF731991C}"));

    writer.writeEmptyElement(QStringLiteral("workbookPr"));
    if (d->date1904)
        writer.writeAttribute(QStringLiteral("date1904"), QStringLiteral("1"));
    writer.writeAttribute(QStringLiteral("defaultThemeVersion"), QStringLiteral("124226"));

    writer.writeStartElement(QStringLiteral("bookViews"));
    writer.writeEmptyElement(QStringLiteral("workbookView"));
    writer.writeAttribute(QStringLiteral("xWindow"), QString::number(d->x_window));
    writer.writeAttribute(QStringLiteral("yWindow"), QString::number(d->y_window));
    writer.writeAttribute(QStringLiteral("windowWidth"), QString::number(d->window_width));
    writer.writeAttribute(QStringLiteral("windowHeight"), QString::number(d->window_height));
    //Store the firstSheet when it isn't the default
    //For example, when "the first sheet 0 is hidden", the first sheet will be 1
    if (d->firstsheet > 0)
        writer.writeAttribute(QStringLiteral("firstSheet"), QString::number(d->firstsheet + 1));
    //Store the activeTab when it isn't the first sheet
    if (d->activesheetIndex > 0)
        writer.writeAttribute(QStringLiteral("activeTab"), QString::number(d->activesheetIndex));
    writer.writeEndElement();//bookViews

    writer.writeStartElement(QStringLiteral("sheets"));
    for (int i=0; i<d->worksheets.size(); ++i) {
        QSharedPointer<Worksheet> sheet = d->worksheets[i];
        writer.writeEmptyElement(QStringLiteral("sheet"));
        writer.writeAttribute(QStringLiteral("name"), sheet->sheetName());
        writer.writeAttribute(QStringLiteral("sheetId"), QString::number(sheet->sheetId()));
        if (sheet->isHidden())
            writer.writeAttribute(QStringLiteral("state"), QStringLiteral("hidden"));
        writer.writeAttribute(QStringLiteral("r:id"), QStringLiteral("rId%1").arg(i+1));
    }
    writer.writeEndElement();//sheets

    if (!d->definedNamesList.isEmpty()) {
        writer.writeStartElement(QStringLiteral("definedNames"));
        foreach (XlsxDefineNameData data, d->definedNamesList) {
            writer.writeStartElement(QStringLiteral("definedName"));
            writer.writeAttribute(QStringLiteral("name"), data.name);
            if (!data.comment.isEmpty())
                writer.writeAttribute(QStringLiteral("comment"), data.comment);
            if (data.sheetId != -1) {
                //find the local index of the sheet.
                for (int i=0; i<d->worksheets.size(); ++i) {
                    if (d->worksheets[i]->sheetId() == data.sheetId) {
                        writer.writeAttribute(QStringLiteral("localSheetId"), QString::number(i));
                        break;
                    }
                }
            }
            writer.writeCharacters(data.formula);
            writer.writeEndElement();//definedName
        }
        writer.writeEndElement();//definedNames
    }

    writer.writeStartElement(QStringLiteral("calcPr"));
    writer.writeAttribute(QStringLiteral("calcId"), QStringLiteral("124519"));
    writer.writeEndElement(); //calcPr

    writer.writeEndElement();//workbook
    writer.writeEndDocument();
}

QByteArray Workbook::saveToXmlData() const
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    saveToXmlFile(&buffer);

    return data;
}

bool Workbook::loadFromXmlFile(QIODevice *device)
{
    Q_D(Workbook);

    QXmlStreamReader reader(device);
    while (!reader.atEnd()) {
         QXmlStreamReader::TokenType token = reader.readNext();
         if (token == QXmlStreamReader::StartElement) {
             if (reader.name() == QLatin1String("sheet")) {
                 XlsxSheetItemInfo info;
                 QXmlStreamAttributes attributes = reader.attributes();
                 info.name = attributes.value(QLatin1String("name")).toString();
                 info.sheetId = attributes.value(QLatin1String("sheetId")).toString().toInt();
                 info.rId = attributes.value(QLatin1String("r:id")).toString();
                 if (attributes.hasAttribute(QLatin1String("state")))
                     info.state = attributes.value(QLatin1String("state")).toString();
                 d->sheetItemInfoList.append(info);
             } else if (reader.name() == QLatin1String("workbookPr")) {
                QXmlStreamAttributes attrs = reader.attributes();
                if (attrs.hasAttribute(QLatin1String("date1904")))
                    d->date1904 = true;
             } else if (reader.name() == QLatin1String("bookviews")) {
                while (!(reader.name() == QLatin1String("bookviews") && reader.tokenType() == QXmlStreamReader::EndElement)) {
                    reader.readNextStartElement();
                    if (reader.tokenType() == QXmlStreamReader::StartElement) {
                        if (reader.name() == QLatin1String("workbookView")) {
                            QXmlStreamAttributes attrs = reader.attributes();
                            if (attrs.hasAttribute(QLatin1String("xWindow")))
                                d->x_window = attrs.value(QLatin1String("xWindow")).toString().toInt();
                            if (attrs.hasAttribute(QLatin1String("yWindow")))
                                d->y_window = attrs.value(QLatin1String("yWindow")).toString().toInt();
                            if (attrs.hasAttribute(QLatin1String("windowWidth")))
                                d->window_width = attrs.value(QLatin1String("windowWidth")).toString().toInt();
                            if (attrs.hasAttribute(QLatin1String("windowHeight")))
                                d->window_height = attrs.value(QLatin1String("windowHeight")).toString().toInt();
                            if (attrs.hasAttribute(QLatin1String("firstSheet")))
                                d->firstsheet = attrs.value(QLatin1String("firstSheet")).toString().toInt();
                            if (attrs.hasAttribute(QLatin1String("activeTab")))
                                d->activesheetIndex = attrs.value(QLatin1String("activeTab")).toString().toInt();
                        }
                    }
                }
             } else if (reader.name() == QLatin1String("definedName")) {
                 QXmlStreamAttributes attrs = reader.attributes();
                 XlsxDefineNameData data;

                 data.name = attrs.value(QLatin1String("name")).toString();
                 if (attrs.hasAttribute(QLatin1String("comment")))
                     data.comment = attrs.value(QLatin1String("comment")).toString();
                 if (attrs.hasAttribute(QLatin1String("localSheetId"))) {
                     int localId = attrs.value(QLatin1String("localSheetId")).toString().toInt();
                     int sheetId = d->sheetItemInfoList[localId].sheetId;
                     data.sheetId = sheetId;
                 }
                 data.formula = reader.readElementText();
                 d->definedNamesList.append(data);
             }
         }
    }
    return true;
}

bool Workbook::loadFromXmlData(const QByteArray &data)
{
    QBuffer buffer;
    buffer.setData(data);
    buffer.open(QIODevice::ReadOnly);

    return loadFromXmlFile(&buffer);
}

/*!
 * \internal
 */
Relationships &Workbook::relationships()
{
    Q_D(Workbook);
    return d->relationships;
}

} // namespace QXlsx
