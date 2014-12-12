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

#ifndef __QIN_SRC_QIN_ENGINE_H__
#define __QIN_SRC_QIN_ENGINE_H__

#include <QWSInputMethod>
#include "QVirtualKeyboard.h"
#include "QinIMBases.h"

QT_BEGIN_NAMESPACE
class QVirtualKeyboard;
QT_END_NAMESPACE

class QinEngine: public QWSInputMethod {
    Q_OBJECT

    friend class QVirtualKeyboard;

    public:
        QinEngine(QString lang);
        ~QinEngine();
        void regInputMethod(QinIMBase* imb);
        void setCurrentIM(int index);
        void getLineEditFocus();

    public slots:
        void setShift(bool shifted, bool capsed);
        void commitPreEdit();
        void changeLanguage(QString oldLang, QString newLang);

    signals:
        void keyboardFinished();
        void keyPressed(int numChars);

    protected:
        QVirtualKeyboard* vkeyboard;
        QString inputBuffer;
        QVector<QinIMBase*> inputMethods;
        QVector<QinIMBase*> activeInputMethods;
        QinIMBase* defaultIM;
        QinIMBase* currentIM;
        QinIMBase* nextIM;
        int selected;
        QStringList numbers;
        QString currentLanguage;
        bool pinyinEnabled;

        bool filter(int uni, int keyId, int mod, bool isPress, bool autoRepeat);
        void updateHandler(int type);
        void mouseHandler(int offset, int state);
        void updateCommitString();
        void updatePreEditBuffer(void);
        void selectPreEditWord(int index);
};

#endif /* __QIN_SRC_QIN_ENGINE_H__ */
