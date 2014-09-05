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
#ifdef DEBUG
    qDebug() << "DEBUG: Current char: " << currChar;
#endif
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
    engine->update_commit_string(TWCHAR2str(wstr, -1));
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

    factory.setCandiWindowSize(QIN_ENGINE_MAX_CHINESE_SYMBOL_LEN - 2);

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
  return candidates;
}

char* QinPinyin::getPreEditString(void) {
  return strdup(preeditStr.c_str());
}

char* QinPinyin::getCommitString(void) {
  string str = commitStr;
  commitStr.clear();
  return strdup(str.c_str());
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
  if (preeditStr.length() > 0) {
    pv->onCandidatePageRequest(-1, true);
  }
}

void QinPinyin::handle_Right(void) {
  if (preeditStr.length() > 0) {
    pv->onCandidatePageRequest(1, true);
  }
}

void QinPinyin::handle_Candidate(int candNum) {
  pv->onCandidateSelectRequest(candNum);
}
