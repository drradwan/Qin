/**
 * Copyright (C) 2014 - Adam Radwan <adam@radwan.us>
 * All rights reserved.
 * 
 * Copyright (C) 2013 - Wei-Ning Huang (AZ) <aitjcize@gmail.com>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "QinIMBases.h"

#include <cstring>

#include <QCoreApplication>
#include <QDebug>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTextStream>
#include <QVariant>

/* QinIMBase methods implementation */

QinIMBase::QinIMBase(QString xmlpath): xmlPath(xmlpath) {
    QFile file(xmlPath);
    QString xmlData, err;
    int line, column;

    // setup default keymap + unicode
    changeLanguage("en");

    if (file.open(QFile::ReadOnly)) {
        QTextStream styleIn(&file);
        xmlData = styleIn.readAll();
        file.close();
    } else {
        qDebug() << "Fatal error: can't open `" << xmlPath << "' ..."
            << "abort.";
        QCoreApplication::exit(1);
    }

    QDomDocument xml;
    if (!xml.setContent(xmlData, &err, &line, &column)) {
        qDebug() << "Fatal error: error while parsing `" << xmlPath << "', "
            << line << ", " << column << ": " << err;
        QCoreApplication::exit(1);
    }

    QDomElement root = xml.documentElement();
    imName = root.firstChildElement("name").text();
#ifdef DEBUG
    qDebug() << "DEBUG: imName: " << imName;
#endif

    preEditable = (root.firstChildElement("preeditable").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: preEditable: " << preEditable;
#endif

    useCustomKeyMap = (root.firstChildElement("customkeymap").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: useCustomKeyMap: " << useCustomKeyMap;
#endif
    setupKeyMap(root.firstChildElement("keymap"));

    candidateArrows = (root.firstChildElement("candidatearrows").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: candidateArrows: " << candidateArrows;
#endif

    capsable = (root.firstChildElement("capsable").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: capsable: " << capsable;
#endif

    shiftable = (root.firstChildElement("shiftable").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: shiftable: " << shiftable;
#endif

    symbols = (root.firstChildElement("symbols").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: symbols: " << symbols;
#endif
}

QinIMBase::~QinIMBase() {}

QString QinIMBase::name(void) const {
    return imName;
}

void QinIMBase::setUseCustomKeyMap(bool s) {
    useCustomKeyMap = s;
}

bool QinIMBase::getUseCustomKeyMap(void) {
    return useCustomKeyMap;
}

void QinIMBase::setPreEditable(bool s) {
    preEditable = s;
}

bool QinIMBase::getPreEditable(void) {
    return preEditable;
}

void QinIMBase::setCandidateArrows(bool s) {
    candidateArrows = s;
}

bool QinIMBase::getCandidateArrows(void) {
    return candidateArrows;
}

void QinIMBase::setupKeyMap(const QDomElement& keymap) {
    if (keymap.isNull()) return;

    /* Mapping normal mode keymap */
    QDomElement normal = keymap.firstChildElement("normal");
    if (normal.isNull()) {
        qDebug() << "Fatal error: normal keymap not set!";
        QCoreApplication::exit(1);
    }

    QDomNode node = normal.firstChild();
    QDomElement nodeElement;
    QString attr;
    while (!node.isNull()) {
        nodeElement = node.toElement();
        attr = nodeElement.attribute("value");
        fromStdKB_hash[attr] = nodeElement.text();
        node = node.nextSibling();
    }

    /* Mapping shift mode keymap */
    QDomElement shift = keymap.firstChildElement("shift");
    if (shift.isNull()) {
        qDebug() << "Fatal error: shift keymap not set!";
        QCoreApplication::exit(1);
    }
    node = shift.firstChild();
    while (!node.isNull()) {
        nodeElement = node.toElement();
        attr = nodeElement.attribute("value");
        fromShiftStdKB_hash[attr] = nodeElement.text();
        node = node.nextSibling();
    }
}

bool QinIMBase::getDoPopUp(void) {
    return false;
}

QStringList QinIMBase::getPopUpStrings(void) {
    return QStringList();
}

bool QinIMBase::isPreEditing(void) {
    return false;
}

char* QinIMBase::getPreEditString(void) {
    return NULL;
}

char* QinIMBase::getCommitString(void) {
    return NULL;
}

int QinIMBase::cursorCurrent(void) {
    return -1;
}

void QinIMBase::setCursor(int) {}

QString QinIMBase::fromStdKB(QString str) {
    QString key = (fromStdKB_hash.find(str) != fromStdKB_hash.end())?
        fromStdKB_hash[str]: str;
    if (key == "&") key = "&&";
    return key;
}

QString QinIMBase::fromShiftStdKB(QString str) {
    return (fromShiftStdKB_hash.find(str) != fromShiftStdKB_hash.end())?
        fromShiftStdKB_hash[str]: str.toUpper();
}

QString QinIMBase::fromStdKBUnicode(QString str) {
    QString key = (fromStdKBUnicode_hash.find(str) != fromStdKBUnicode_hash.end())?
        fromStdKBUnicode_hash[str]: str;

    return key;
}

QString QinIMBase::fromShiftStdKBUnicode(QString str) {
    return (fromShiftStdKBUnicode_hash.find(str) != fromShiftStdKBUnicode_hash.end())?
        fromShiftStdKBUnicode_hash[str]: str.toUpper();
}
void QinIMBase::reset(void) {}
void QinIMBase::handle_Alt(void) {}
void QinIMBase::handle_Backspace(void) {}
void QinIMBase::handle_Capslock(void) {}
void QinIMBase::handle_Ctrl(void) {}
void QinIMBase::handle_Default(int, bool) {}
void QinIMBase::handle_Del(void) {}
void QinIMBase::handle_Down(void) {}
void QinIMBase::handle_End(void) {}
void QinIMBase::handle_Enter(void) {}
void QinIMBase::handle_Esc(void) {}
void QinIMBase::handle_Home(void) {}
void QinIMBase::handle_Left(void) {}
void QinIMBase::handle_PageDown(void) {}
void QinIMBase::handle_PageUp(void) {}
void QinIMBase::handle_Right(void) {}
void QinIMBase::handle_Space(void) {}
void QinIMBase::handle_Tab(void) {}
void QinIMBase::handle_Up(void) {}
void QinIMBase::commit_Default(void) {}
void QinIMBase::handle_Candidate(int) {}
void QinIMBase::setLanguageCode(QString) {}
int QinIMBase::getLanguageCode() { return ENGLISH; }



/* QinTableIMBase methods implementation */

QinTableIMBase::QinTableIMBase(QString xmlpath): QinIMBase(xmlpath) {
    QFile file(xmlPath);
    QString xmlData, err;
    int line, column;

    if (file.open(QFile::ReadOnly)) {
        QTextStream styleIn(&file);
        xmlData = styleIn.readAll();
        file.close();
    } else {
        qDebug() << "Fatal error: can't open `" << xmlPath << "' ..."
            << "abort.";
        QCoreApplication::exit(1);
    }

    QDomDocument xml;
    if (!xml.setContent(xmlData, &err, &line, &column)) {
        qDebug() << "Fatal error: error while parsing `" << xmlPath << "', "
            << line << ", " << column << ": " << err;
        QCoreApplication::exit(1);
    }

    QDomElement root = xml.documentElement();

    maxKeyStrokes = root.firstChildElement("maxkeystrokes").text().toInt();
#ifdef DEBUG
    qDebug() << "DEBUG: maxKeyStrokes: " << maxKeyStrokes;
#endif

    dbPath = root.firstChildElement("database").text();
#ifdef DEBUG
    qDebug() << "DEBUG: dbPath: " << dbPath;
#endif

    queryTemplate = root.firstChildElement("querytemplate").text();
#ifdef DEBUG
    qDebug() << "DEBUG: queryTemplate: " << queryTemplate;
#endif

    candidateArrows = (root.firstChildElement("candidatearrows").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: candidateArrows: " << candidateArrows;
#endif

    QDomElement keytransform = root.firstChildElement("keytransform");
    QDomNode node = keytransform.firstChild();
    QDomElement nodeElement;
    QChar attr;
    QString keyText;
    int keyValue;
    while (!node.isNull()) {
        nodeElement = node.toElement();
        attr = nodeElement.attribute("value")[0];
        keyText = nodeElement.text();
        if (keyText.contains("0x"))
        {
            bool ok;
            keyValue = keyText.toInt(&ok, 16);
        }
        else
        {
            keyValue = keyText.toInt();
        }
        keyTransform[attr.unicode()] = keyValue;

        node = node.nextSibling();
    }

    /* Initialize members */
    keyIndex = 0;
    keyStrokes = new int[maxKeyStrokes + 1];

    database = QSqlDatabase::addDatabase("QSQLITE", dbPath);
    database.setDatabaseName(dbPath);
    if (!database.open()) {
        qDebug() << "Fatal error: can't find database `" << dbPath << "' ..."
            << "abort.";
        QCoreApplication::exit(1);
    }
    database.setConnectOptions("QSQLITE_OPEN_READONLY");

    useCustomKeyMap = (root.firstChildElement("customkeymap").text() == "true")?
        true : false;
#ifdef DEBUG
    qDebug() << "DEBUG: useCustomKeyMap: " << useCustomKeyMap;
#endif
}

QinTableIMBase::~QinTableIMBase() {
    database.close();
    delete [] keyStrokes;
}

QString QinTableIMBase::getQueryTemplate(void) {
    return queryTemplate;
}

bool QinTableIMBase::isPreEditing(void) {
    return keyIndex != 0;
}

bool QinTableIMBase::getDoPopUp(void) {
    return true;
}

QStringList QinTableIMBase::getPopUpStrings(void) {
    if (results.size()) {
        return results;
    } else {
        QStringList numbers;
        numbers << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0";
        return numbers;
    }
}

int QinTableIMBase::doQuery() {
    int count = 0;
    QString queryTemplate = getQueryTemplate();
    QString query = queryTemplate;

    for (int i = 0; i < maxKeyStrokes; ++i) {
        if (i < keyIndex) {
#if 0
            query = query.arg("m%1=%2%3").arg(i).arg(
                    keyTransform[keyStrokes[i]]);
#else
            query = query.arg("(lang & %1) AND (m%2=%3%4)").arg(langCode).arg(i).arg(
                    keyTransform[keyStrokes[i]]);
#endif
            if (i != keyIndex -1)
                query = query.arg(" AND %1");
            else
                query = query.arg("");
        }
    }

#ifdef DEBUG
    qDebug() << "DEBUG: query: " << query;
#endif

    results.clear();
    QSqlQuery queryResults = database.exec(query);

    while (queryResults.next() && count++ < SELKEY_COUNT)
        results += queryResults.value(0).toString();

#ifdef DEBUG
    qDebug() << "DEBUG: results: " << results;
#endif

    return count;
}

char* QinTableIMBase::getPreEditString(void) {
    char* preEditStr = NULL;
    const char* cstr = NULL;

    if (!results.isEmpty()) {
        //    cstr = results[0].toStdString().c_str();
        // use toLocal8Bit() instead
        QByteArray ba = results[0].toLocal8Bit();
        cstr = ba.constData();
        preEditStr = new char[strlen(cstr) + 1];
        memcpy(preEditStr, cstr, strlen(cstr));
        preEditStr[strlen(cstr)] = 0;
    }

    //#ifdef DEBUG
    //  if (preEditStr) {
    //    qDebug() << "DEBUG: results[0]: " << results[0];
    //    qDebug("DEBUG: preEditStr: %s", preEditStr);
    //  }
    //#endif

    return preEditStr;
}

char* QinTableIMBase::getCommitString(void) {
    char* commitStr = NULL;
    const char* cstr = NULL;

    if (!commitString.isEmpty()) {
#ifdef DEBUG
        qDebug() << "getCommitString " << commitString;
#endif

        // why this returns array with length = 0??,
        // maybe Qt is not configured with STL compatibility enabled.
        //    cstr = commitString.toStdString().c_str();
        // use toLocal8Bit() instead
        QByteArray ba = commitString.toLocal8Bit();
        cstr = ba.constData();

        commitStr = new char[strlen(cstr) + 1];
        memcpy(commitStr, cstr, strlen(cstr));
        commitStr[strlen(cstr)] = 0;
        commitString.clear();
    }
    return commitStr;
}

void QinTableIMBase::handle_Default(int keyId, bool shifted) {
#ifdef DEBUG
    qDebug() << "handle_Default called with keyId = " << keyId;
#endif
    int keys[] = SELKEYS;

    if (maxKeyStrokes == 1) {
        /* reset keyStrokes */
        keyIndex = 0;
        if (keyId >= 34 && results.count() > 1 && !results[0].isEmpty()) {
            commitString = results[0];
            results.clear();
        }
    }

    if (keyIndex == maxKeyStrokes)
        return;

    if (results.size()) {
        for (size_t i = 0; i < SELKEY_COUNT; ++i) {
            if (keyId == keys[i]) {
                commitString = results[i];
                results.clear();
                keyIndex = 0;
                return;
            }
        }
    }
    else if (keyId >= 16 && keyId <= 25) // numbers
    {
        commitString = QString((char) keyId + 32);
        keyIndex = 0;
        return;
    }

    // Special case for apostrophe on AZERTY keyboard
    if (keyId >=34 && keyId <= 39) {
        commitString += QString("'");
        keyIndex = 0;
    }
    else if (keyId == 63)
    {
        commitString += QString("?");
        keyIndex = 0;
    }

    //  if (keyTransform.find(tolower(keyId)) == keyTransform.end())
    if (keyTransform.find(keyId) == keyTransform.end())
    {
#ifdef DEBUG
        qDebug() << "Not found in keyTransform keyID= " << keyId << " text= " << QChar(keyId);
#endif
        return;
    }
    //  int currentkeyIndex = keyIndex;
    keyStrokes[keyIndex] = keyId;
    keyIndex++;
    //  qDebug() << "before doQuery " << keyId << " keyStrokes " << keyStrokes[currentkeyIndex];

    if (doQuery() < 1 && maxKeyStrokes == 1) {
        commitString += QString((QChar) keyId);
        results.clear();
        keyIndex = 0;
    }

}

void QinTableIMBase::handle_Space(void) {
    doQuery();
    if (results.isEmpty()) {
        commitString.clear();
    } else {
        commitString = results[0];
        if (maxKeyStrokes == 1)
            commitString += " ";
        results.clear();
    }

    /* reset keyStrokes */
    keyIndex = 0;
}

void QinTableIMBase::handle_Enter(void) {
    handle_Space();
}

void QinTableIMBase::handle_Backspace(void) {
    if (maxKeyStrokes == 1) {
        results.clear();
        commitString.clear();
    }
    if (keyIndex > 0)
        --keyIndex;
    if (maxKeyStrokes > 1)
        doQuery();
}

void QinTableIMBase::commit_Default(void) {
    doQuery();
    if (results.isEmpty()) {
        commitString.clear();
    } else {
        commitString = results[0];
        results.clear();
    }

    /* reset keyStrokes */
    keyIndex = 0;
}
void QinTableIMBase::setLanguageCode(QString currLang)
{
    if(currLang.contains("da"))
        langCode = DANISH;
    else if(currLang.contains("nl"))
        langCode = DUTCH;
    else if(currLang.contains("fi"))
        langCode = FINNISH;
    else if(currLang.contains("fr"))
        langCode = FRENCH;
    else if(currLang.contains("de"))
        langCode = GERMAN;
    else if(currLang.contains("it"))
        langCode = ITALIAN;
    else if(currLang.contains("jp"))
        langCode = JAPANESE;
    else if(currLang.contains("nb"))
        langCode = NORWEGIAN;
    else if(currLang.contains("pl"))
        langCode = POLISH;
    else if(currLang.contains("ru"))
        langCode = RUSSIAN;
    else if(currLang.contains("es"))
        langCode = SPANISH;
    else if(currLang.contains("sv"))
        langCode = SWEDISH;
    else if(currLang.contains("tr"))
        langCode = TURKISH;
    else if(currLang.contains("pt"))
        langCode = PORTUGUESE;
    else
        langCode = ENGLISH;
}

int QinTableIMBase::getLanguageCode()
{
    return langCode;
}

void QinIMBase::changeLanguage(QString language)
{
#ifdef DEBUG
    qDebug() << "QinIMBase::changeLanguage for " << language;
#endif

    QString xmlPath = ":/data/KeyboardLayout.xml";
    QFile file(xmlPath);
    QString xmlData, err;
    int line, column;

    if (file.open(QFile::ReadOnly)) {
        QTextStream styleIn(&file);
        xmlData = styleIn.readAll();
        file.close();
    } else {
#ifdef DEBUG
        qDebug() << "Fatal error: can't open `" << xmlPath << "' ..." << "abort.";
#endif
        //      QCoreApplication::exit(1);
        return;
    }

    QDomDocument xml;
    if (!xml.setContent(xmlData, &err, &line, &column)) {
#ifdef DEBUG
        qDebug() << "Fatal error: error while parsing `" << xmlPath << "', "
            << line << ", " << column << ": " << err;
#endif
        //      QCoreApplication::exit(1);
        return;
    }

    QDomElement root = xml.documentElement();
    QDomElement keymap = root.firstChildElement("keymap");
    if (keymap.isNull())
    {
#ifdef DEBUG
        qDebug() << "Cannot find keymap";
#endif
        return;
    }
    if (!language.contains("ru") && !language.contains("en"))
    {
        searchAndUpdateKeyMapBy("en", &keymap); //reset keymap for other languages, except Russian & English
    }
    searchAndUpdateKeyMapBy(language, &keymap);
}
void QinIMBase::searchAndUpdateKeyMapBy(QString language, QDomElement* keymap)
{
    //    QString lang = language.left(2); // take first 2 characters
    QStringRef lang(&language, 0, 2);
    QDomElement element = keymap->firstChildElement("language");
    for (; !element.isNull(); element = element.nextSiblingElement("language")) {
        QString locale = element.attribute("value");

        if(0 == QString::compare(locale, lang, Qt::CaseInsensitive))
        {
            setupKeyMapUnicode(element);
            return;
        }
    }
}

void QinIMBase::setupKeyMapUnicode(const QDomElement& keymap) {
    if (keymap.isNull()) return;
#ifdef DEBUG
    qDebug() << "QinIMBase::setupKeyMapUnicode ";
#endif

    /* Mapping normal mode keymap */
    QDomElement normal = keymap.firstChildElement("normal");
    if (normal.isNull()) {
#ifdef DEBUG
        qDebug() << "Fatal error: normal keymap not set!";
#endif
        //    QCoreApplication::exit(1);
        return;
    }

    QDomNode node = normal.firstChild();
    QDomElement nodeElement;
    QString attr;
    while (!node.isNull()) {
        nodeElement = node.toElement();
        attr = nodeElement.attribute("value");
        QString key = nodeElement.text();
        if(key.contains("0x"))
        {
            fromStdKBUnicode_hash[attr] = key;
            // converts to QChar array
            bool ok;
            uint hex = key.toUInt(&ok, 16);
            if (ok)
            {
                key = QString(QChar(hex));
                //            qDebug() << key;
            }
            else
            {
                qDebug() << key << " parse failed";
            }

        }
        fromStdKB_hash[attr] = key;

        //    qDebug() << attr << " " << fromStdKB_hash[attr];
        node = node.nextSibling();
    }

    /* Mapping shift mode keymap */
    QDomElement shift = keymap.firstChildElement("shift");
    if (shift.isNull()) {
#ifdef DEBUG
        qDebug() << "Fatal error: shift keymap not set!";
#endif
        //    QCoreApplication::exit(1);
        return;
    }
    node = shift.firstChild();
    while (!node.isNull()) {
        nodeElement = node.toElement();
        attr = nodeElement.attribute("value");
        QString key = nodeElement.text();
        if(key.contains("0x"))
        {
            fromShiftStdKBUnicode_hash[attr] = key;
            // converts to QChar array
            bool ok;
            uint hex = key.toUInt(&ok, 16);
            if (ok)
            {
                key = QString(QChar(hex));
                //            qDebug() << key;
            }
            else
            {
#ifdef DEBUG
                qDebug() << key << " parse failed";
#endif
            }
        }

        fromShiftStdKB_hash[attr] = key;
        //    qDebug() << attr << " " << fromShiftStdKB_hash[attr];
        node = node.nextSibling();
    }
}
