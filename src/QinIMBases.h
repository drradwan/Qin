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

#ifndef __QIN_SRC_QIN_IM_BASE_H__
#define __QIN_SRC_QIN_IM_BASE_H__

#include <QtXml/QDomElement>
#include <QHash>
#include <QSqlDatabase>
#include <QStringList>

/* Number of selection keys */
// extending selection keys for Spanish to accommodate 11 candidates
#define SELKEY_COUNT 11  // = max number of candidates
#define SELKEYS { 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x10, 0x1A }


const int ENGLISH           = 0;
const int DANISH            = 1;
const int DUTCH             = 2;
const int FINNISH           = 4;
const int FRENCH            = 8;
const int GERMAN            = 16;
const int ITALIAN           = 32;
const int JAPANESE          = 64;
const int NORWEGIAN         = 128;
const int POLISH            = 256;
const int RUSSIAN           = 512;
const int SPANISH           = 1024;
const int SWEDISH           = 2048;
const int TURKISH           = 4096;
const int PORTUGUESE        = 8192;
const int PORTUGUESE_BRA    = 16384;
const int SIMPLIFIED_CHINESE = 32768;

/**
 * @name  QinIMBase
 * @brief Base class for all input method for Qin. New input methods should
 * derive from QinIMBase. 
 */
class QinIMBase {
    protected:
        QString xmlPath;
        bool useCustomKeyMap;
        bool preEditable;
        bool candidateArrows;

    public:
        /** Public members **/
        QString imName;
        QHash<QString, QString> fromStdKB_hash;
        QHash<QString, QString> fromShiftStdKB_hash;

        /** Public methods **/
        QinIMBase(QString xmlpath);
        virtual ~QinIMBase();

        QString name(void) const;
        void setUseCustomKeyMap(bool s);
        bool getUseCustomKeyMap(void);
        void setPreEditable(bool s);
        bool getPreEditable(void);
        void setCandidateArrows(bool s);
        bool getCandidateArrows(void);

        bool capsable;
        bool shiftable;
        bool symbols;
        int  langCode;

        void setupKeyMap(const QDomElement& keymap);
        virtual bool isPreEditing(void);
        virtual bool getDoPopUp(void);
        virtual QStringList getPopUpStrings(void);

        /** I/O related **/
        /* Caller must free it */
        virtual char* getPreEditString(void);
        virtual char* getCommitString(void);
        virtual int cursorCurrent(void);
        virtual void setCursor(int index);
        QString fromStdKB(QString str);
        QString fromShiftStdKB(QString str);
        virtual void reset(void);

        /** Key handling APIs **/
        virtual void handle_Alt(void);
        virtual void handle_Backspace(void);
        virtual void handle_Capslock(void);
        virtual void handle_Ctrl(void);
        virtual void handle_Default(int, bool);
        virtual void handle_Del(void);
        virtual void handle_Down(void);
        virtual void handle_End(void);
        virtual void handle_Enter(void);
        virtual void handle_Esc(void);
        virtual void handle_Home(void);
        virtual void handle_Left(void);
        virtual void handle_PageDown(void);
        virtual void handle_PageUp(void);
        virtual void handle_Right(void);
        virtual void handle_Space(void);
        virtual void handle_Tab(void);
        virtual void handle_Up(void);

        /* Added to force commit for table */
        virtual void commit_Default(void);

        virtual void handle_Candidate(int);

        /*For support Latin-based languages*/
        virtual void setLanguageCode(QString);
        virtual int getLanguageCode();

        /*To read in keyboard layout, i.e. related to btn->whatsthis, for different languages*/
        virtual void changeLanguage(QString language);
        virtual void searchAndUpdateKeyMapBy(QString lang, QDomElement* keymap);
        virtual void setupKeyMapUnicode(const QDomElement& keymap);
        virtual QString fromStdKBUnicode(QString str);
        virtual QString fromShiftStdKBUnicode(QString str);

    protected:
        // hash table to contain unicode character value as string
        QHash<QString, QString> fromStdKBUnicode_hash;
        QHash<QString, QString> fromShiftStdKBUnicode_hash;
};

/**
 * @name  QinTableIMBase
 * @brief Base class for table input methods.
 */
class QinTableIMBase: public QinIMBase {
    protected:
        QString dbPath;
        QSqlDatabase database;
        QString commitString;
        QStringList results;
        QString queryTemplate;
        QHash<int, int> keyTransform;
        int* keyStrokes;
        int maxKeyStrokes;
        int keyIndex;

    public:
        /** Public methods **/
        QinTableIMBase(QString xmlpath);
        virtual ~QinTableIMBase();

        void setLanguageCode(QString code);
        int getLanguageCode();

        virtual bool isPreEditing(void);
        virtual bool getDoPopUp(void);
        virtual QStringList getPopUpStrings(void);
        virtual int doQuery(void);
        virtual QString getQueryTemplate(void);

        /** I/O related **/
        /* Caller must free it */
        virtual char* getPreEditString(void);
        virtual char* getCommitString(void);
        //    QString fromStdKB(QString str);
        //    QString fromShiftStdKB(QString str);

        /** Key handling APIs **/
        virtual void handle_Default(int keyId, bool shifted);
        virtual void handle_Enter(void);
        virtual void handle_Space(void);
        virtual void handle_Backspace(void);

        /* Added to force commit the first choice on the candidate bar */
        virtual void commit_Default(void);

};

#endif /* __QIN_SRC_QIN_IM_BASE_H__ */
