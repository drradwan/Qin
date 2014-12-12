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

#ifndef __QIN_SRC_PLUGIN_QIN_PINYIN_H__
#define __QIN_SRC_PLUGIN_QIN_PINYIN_H__

#include "QinConfig.h"
#include "QinIMBases.h"

#include <string>
using std::string;

#include <sunpinyin.h>

#define PREEDIT_LENGTH_MAX 50
#define SUNPINYIN_USER_DIR (homedir S_SEPARATOR ".sunpinyin")

class QinPinyin;

class CQinWinHandler : public CIMIWinHandler
{
public:
    CQinWinHandler(QinPinyin *);

    /* inherited methods implementation */
    virtual ~CQinWinHandler();

    /** commit a string, normally the converted result */
    virtual void commit(const TWCHAR* wstr);

    /** when a key is not processed */
    virtual void throwBackKey(unsigned keycode, unsigned keyvalue,
                              unsigned modifier);

    /** Update window's preedit area using a GUI widget. */
    virtual void updatePreedit(const IPreeditString* ppd);

    /** Update window's candidates area using a GUI widget. */
    virtual void updateCandidates(const ICandidateList* pcl);

private:
    QinPinyin *engine;
};

class QinPinyin: public QinIMBase {
  private:
    CIMIView* pv;
    CQinWinHandler* wh;
    CHotkeyProfile* hotkey_profile;
    CSunpinyinSessionFactory::EPyScheme pinyin_scheme;
    QStringList candidates;
    ICandidateList* currentCandidates;

    string preeditStr;
    string commitStr;

  private:
    char* querySentence(const char* str);

  public:
    QinPinyin(void);
    virtual ~QinPinyin(void);

    virtual bool isPreEditing(void);
    virtual bool getDoPopUp(void);
    virtual QStringList getPopUpStrings(void);

    void update_user_data_dir();

    /* CQinWinHandler helpers */
    void update_candidates(const ICandidateList& cands);
    void update_preedit_string(const IPreeditString& preedits);
    void update_commit_string(const string str);

    /** I/O related **/
    virtual void chopPreEditString(int length);
    virtual char* getPreEditString(void);
    virtual char* getCommitString(void);
    virtual void reset(void);

    /** Key handling APIs **/
    virtual void handle_Default(int keyId, bool shifted);
    virtual void handle_Space(void);
    virtual void handle_Esc(void);
    virtual void handle_Enter(void);
    virtual void handle_Del(void);
    virtual void handle_Backspace(void);
    virtual void handle_Left(void);
    virtual void handle_Right(void);
    
    virtual void handle_Candidate(int candNum);
    virtual void commit_Default(void);
};

#endif /* __QIN_SRC_PLUGIN_QIN_PINYIN_H__ */
