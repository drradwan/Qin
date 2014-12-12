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

#include "QinPinyin.h"
#include "QinIMBases.h"

#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
using std::vector;

#include <iterator>
using std::back_inserter;

#include <algorithm>
using std::copy;

#include <QStringList>
#include <QDebug>

#include <QString>
#include <QChar>

#include <QDomDocument>
#include <QDomElement>

/* Helper functions */

string TWCHAR2str(const unsigned int* twchar, const int size)
{
  QString retVal;
  int mSize = 1;
  if (size > -1) {
    mSize = size;
  }
  for (int i=0; i < mSize; ++i) {
    QChar currChar = QChar(*((uint*)(twchar)+i));
    retVal.append(currChar);
  }
  return retVal.toUtf8().constData();
}

// string_pairs parse_pairs(const vector<string>& strings)
// {
//     string_pairs pairs;
//     for (vector<string>::const_iterator pair = strings.begin();
//         pair != strings.end(); ++pair) {

//         std::string::size_type found = pair->find(':');
//         if (found == pair->npos || pair->length() < 3)
//             continue;
//         if (found == 0 && (*pair)[0] == ':')
//             found = 1;

//         pairs.push_back(make_pair(pair->substr(0, found),
//                                   pair->substr(found+1)));
//     }
//     return pairs;
// }

/* CQinWinHandler implementation */

CQinWinHandler::CQinWinHandler(QinPinyin* engine)
  : CIMIWinHandler(), engine(engine)
{
}

CQinWinHandler::~CQinWinHandler()
{
}

void CQinWinHandler::commit(const TWCHAR* wstr)
{
  if (wstr) {
    engine->update_commit_string(TWCHAR2str(wstr, WCSLEN(wstr)));
  }
}

void CQinWinHandler::updatePreedit(const IPreeditString* ppd)
{
  if (ppd)
    engine->update_preedit_string(*ppd);
}

void CQinWinHandler::updateCandidates(const ICandidateList* pcl)
{
  if (pcl)
    engine->update_candidates(*pcl);
}

void CQinWinHandler::throwBackKey(unsigned keycode, unsigned keyvalue,
    unsigned modifier)
{
  if (keyvalue > 0x0 && keyvalue < 0x7f) {
    printf("%c", keyvalue);
    fflush(stdout);
  }
}

QinPinyin::QinPinyin(void): QinIMBase(":/data/Pinyin.xml") {
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    pinyin_scheme = CSunpinyinSessionFactory::QUANPIN;

    factory.setPinyinScheme(pinyin_scheme);
    factory.setLanguage(CSunpinyinSessionFactory::SIMPLIFIED_CHINESE);

    update_user_data_dir();

    factory.setCandiWindowSize(QIN_ENGINE_MAX_CHINESE_SYMBOL_LEN - 4);

    pv = factory.createSession();

    hotkey_profile = new CHotkeyProfile();
    pv->setHotkeyProfile(hotkey_profile);

    wh = new CQinWinHandler(this);
    pv->attachWinHandler(wh);
}

QinPinyin::~QinPinyin(void) {
    if (pv) {
        CSunpinyinSessionFactory& factory =
            CSunpinyinSessionFactory::getFactory();
        factory.destroySession(pv);
    }

    delete wh;
    delete hotkey_profile;
}

void QinPinyin::update_user_data_dir()
{
  ASimplifiedChinesePolicy::instance().setUserDataDir(SUNPINYIN_USER_DIR);
}

void QinPinyin::update_candidates(const ICandidateList& cands)
{
  candidates.clear();
  for (int i = 0; i < cands.size(); ++i) {
    candidates += TWCHAR2str(cands.candiString(i), cands.candiSize(i)).c_str();
  }
}

void QinPinyin::update_preedit_string(const IPreeditString& preedit)
{
  const int len = preedit.size();
  if (len > 0) {
    preeditStr = TWCHAR2str(preedit.string(), preedit.size());
  } else {
    preeditStr.clear();
  }
}

void QinPinyin::update_commit_string(const string str)
{
  commitStr = str;
}

bool QinPinyin::isPreEditing(void) {
  return preeditStr.length() > 0;
}

bool QinPinyin::getDoPopUp(void) {
  return true;
}

QStringList QinPinyin::getPopUpStrings(void) {
  if (candidates.size()) {
    return candidates;
  } else {
    QStringList numbers;
    numbers << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0";
    return numbers;
  }
}

void QinPinyin::chopPreEditString(int length) {
  int strLen = preeditStr.length();
  preeditStr = preeditStr.substr(strLen-length);
#ifdef DEBUG
  qDebug() << "chopPreEditString(" << length << ") results in preeditStr = " << preeditStr.c_str();
#endif
}

char* QinPinyin::getPreEditString(void) {

    // Unify code with QinTableImBase
    // Use "new char[]" instead of strdup() so that callers can use "delete []"
    char* returnedStr = NULL;
    const char* cstr = preeditStr.c_str();
    int strSize = preeditStr.size();
    qDebug() << "QinPinyin::getPreEditString " << strSize;

    if (!preeditStr.empty()) {
        returnedStr = new char[strSize + 1];
        memcpy(returnedStr, cstr, strSize);
        returnedStr[strSize] = 0; // null terminator
    }
    return returnedStr;
}

char* QinPinyin::getCommitString(void) {
    // Unify code with QinTableImBase
    // Use "new char[]" instead of strdup() so that callers can use "delete []"
    char* returnedStr = NULL;
    const char* cstr = commitStr.c_str();
    int strSize = commitStr.size();
    qDebug() << "QinPinyin::getCommitString " << strSize;

    if (!commitStr.empty()) {
        returnedStr = new char[strSize + 1];
        memcpy(returnedStr, cstr, strSize);
        returnedStr[strSize] = 0; // null terminator

        commitStr.clear();
    }
    return returnedStr;
}

void QinPinyin::reset(void) {
}

void QinPinyin::handle_Default(int keyId, bool shifted) {
#ifdef DEBUG
  qDebug() << "QinPinyin::handle_Default(" << keyId << ", " << shifted << ") called";
#endif
  pv->onKeyEvent(CKeyEvent(keyId, keyId, 0));
}

void QinPinyin::handle_Space(void) {
  pv->onKeyEvent(CKeyEvent(IM_VK_SPACE, IM_VK_SPACE, 0));
}

void QinPinyin::handle_Esc(void) {
}

void QinPinyin::handle_Enter(void) {
  commitStr = preeditStr;
  preeditStr.clear();
  pv->onKeyEvent(CKeyEvent(IM_VK_ENTER, IM_VK_ENTER, 0));
}

void QinPinyin::handle_Del(void) {
  pv->onKeyEvent(CKeyEvent(IM_VK_DELETE, IM_VK_DELETE, 0));
}

void QinPinyin::handle_Backspace(void) {
  pv->onKeyEvent(CKeyEvent(IM_VK_BACK_SPACE, IM_VK_BACK_SPACE, 0));
}

void QinPinyin::handle_Left(void) {
#ifdef DEBUG
  qDebug() << "handle_Left called";
#endif
  if (preeditStr.length() > 0) {
    pv->onCandidatePageRequest(-1, true);
    //pv->getCandidateList(*currentCandidates, 0, 8);
    //update_candidates(*currentCandidates);
  }
}

void QinPinyin::handle_Right(void) {
#ifdef DEBUG
  qDebug() << "handle_Right called";
#endif
  if (preeditStr.length() > 0) {
    pv->onCandidatePageRequest(1, true);
    //pv->getCandidateList(*currentCandidates, 0, 8);
    //update_candidates(*currentCandidates);
  }
}

void QinPinyin::handle_Candidate(int candNum) {
  pv->onCandidateSelectRequest(candNum);
}

void QinPinyin::commit_Default(void) {
  pv->onCandidatePageRequest(0, false); 
  pv->onCandidateSelectRequest(0);
}
