/**
 * @file   QinIMBases.cpp
 * @brief  
 * @author Wei-Ning Huang (AZ) <aitjcize@gmail.com>
 *
 * Copyright (C) 2010 -  Wei-Ning Huang (AZ) <aitjcize@gmail.com>
 * All Rights reserved.
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
    true: false;
#ifdef DEBUG
  qDebug() << "DEBUG: preEditable: " << preEditable;
#endif

  useCustomKeyMap = (root.firstChildElement("customkeymap").text() == "true")?
    true: false;
#ifdef DEBUG
  qDebug() << "DEBUG: useCustomKeyMap: " << useCustomKeyMap;
#endif
  setupKeyMap(root.firstChildElement("keymap"));
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
  return (fromStdKB_hash.find(str) != fromStdKB_hash.end())?
    fromStdKB_hash[str]: str;
}

QString QinIMBase::fromShiftStdKB(QString str) {
  return (fromShiftStdKB_hash.find(str) != fromShiftStdKB_hash.end())?
    fromShiftStdKB_hash[str]: str.toUpper();
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

  QDomElement keytransform = root.firstChildElement("keytransform");
  QDomNode node = keytransform.firstChild();
  QDomElement nodeElement;
  QChar attr;
  while (!node.isNull()) {
    nodeElement = node.toElement();
    attr = nodeElement.attribute("value")[0];
    keyTransform[attr.toAscii()] = nodeElement.text().toInt();
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
    true: false;
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
      query = query.arg("m%1=%2%3").arg(i).arg(
          keyTransform[keyStrokes[i]]);
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
  
  while (queryResults.next() && count++ < 10)
    results += queryResults.value(0).toString();
    
  return count;
}

char* QinTableIMBase::getPreEditString(void) {
  char* preEditStr = NULL;
  const char* cstr = NULL;

  if (!results.isEmpty()) {
    cstr = results[0].toStdString().c_str();
    preEditStr = new char[strlen(cstr) + 1];
    memcpy(preEditStr, cstr, strlen(cstr));
    preEditStr[strlen(cstr)] = 0;
  }

#ifdef DEBUG
  if (preEditStr) {
    qDebug() << "DEBUG: results[0]: " << results[0];
    qDebug("DEBUG: preEditStr: %s", preEditStr);
  }
#endif

  return preEditStr;
}

char* QinTableIMBase::getCommitString(void) {
  char* commitStr = NULL;
  const char* cstr = NULL;

  if (!commitString.isEmpty()) {
    cstr = commitString.toStdString().c_str();
    commitStr = new char[strlen(cstr) + 1];
    memcpy(commitStr, cstr, strlen(cstr));
    commitStr[strlen(cstr)] = 0;
    commitString.clear();
  }
  return commitStr;
}

QString QinTableIMBase::fromStdKB(QString str) {
  return str;
}

QString QinTableIMBase::fromShiftStdKB(QString str) {
  return str.toUpper();
}

void QinTableIMBase::handle_Default(int keyId, bool shifted) {
#ifdef DEBUG
  qDebug() << "handle_Default called with keyId = " << keyId;
#endif
  int keys[] = SELKEYS;

  if (maxKeyStrokes == 1) {
    /* reset keyStrokes */
    keyIndex = 0;
    if (keyId > 57 && results.count() > 1 && !results[0].isEmpty()) {
      commitString = results[0];
      results.clear();
    }
  }

  if (keyIndex == maxKeyStrokes)
    return;

  if (results.size()) {
    for (size_t i = 0; i < SELKEY_COUNT; ++i) {
      qDebug() << "Commit String = " << keys[i] << " Key Id = " << keyId;
      if (keyId == keys[i]) {
        if (shifted)
          commitString = results[i].toUpper();
        else
          commitString = results[i];
        results.clear();
        keyIndex = 0;
        return;
      }
    }
  } else if (keyId >= 16 && keyId <= 25) {
    commitString = QString((char) keyId + 32);
    keyIndex = 0;
    return;
  }
  
  if (keyTransform.find(tolower(keyId)) == keyTransform.end())
    return;
  if (shifted) {
    keyStrokes[keyIndex++] = keyId - 32;
  } else { 
    keyStrokes[keyIndex++] = keyId;
  }

  if (doQuery() < 1 && maxKeyStrokes == 1) {
    if (shifted)
      commitString += QString((char) keyId).toUpper();
    else
      commitString += QString((char) keyId);
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
