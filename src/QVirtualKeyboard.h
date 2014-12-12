/*
 * Copyright (C) 2014 - Adam Radwan <adam@radwan.us>
 * All rights reserved.
 * 
 * Copyright (C) 2013 - Wei-Ning Huang (AZ) <aitjcize@gmail.com>
 * All rights reserved.
 *
 * Copyright 2009 EMBITEL (http://www.embitel.com)
 * 
 * This file is part of Virtual Keyboard Project.
 * 
 * Virtual Keyboard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation
 * 
 * Virtual Keyboard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Virtual Keyboard. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QVIRTUALKEYBOARD_H
#define QVIRTUALKEYBOARD_H

#include "ui_QVirtualKeyboard.h"
#include "QinEngine.h"
#include "QinIMBases.h"

#include <QPushButton>
#include <QStringList>
#include <QVector>
#include <QWidget>
#include <QWSInputMethod>
#include <QHideEvent>
#include <QTimer>

QT_BEGIN_NAMESPACE
class QinEngine;
class QSignalMapper;
QT_END_NAMESPACE

class QVirtualKeyboard : public QWidget, public Ui::QVirtualKeyboard {
    Q_OBJECT

    public:
        QVirtualKeyboard(QinEngine* im);
        ~QVirtualKeyboard();

        enum LayoutTypes {
            QWERTY_1 = 0,   // [10, 9, 7], e.g. English
            QWERTY_2 = 1,   // [10, 10, 7], e.g. Spanish
            QWERTY_3 = 2,   // [11, 11, 7], e.g. Danish
            QWERTY_4 = 3,   // [11, 11, 9], e.g. Russian
            AZERTY_1 = 5,   // [10, 10, 7], e.g. French
            AZERTY_2 = 6,   // [10, 9, 7] e.g. Dutch
        };

        void setDefaultIMName();
        //void insertInputMethod(const QinIMBase* im);
        void clearCandStrBar(bool showNumbers);
        void showCandStrBar(QStringList strlist);
        void hideAll(void);
        bool Capsed;
        bool Shifted;
        bool isQWERTY;
        bool symbolCapsed;
        void setShift(bool shifted, bool capsed);
        void switchToAZERTY(QinIMBase* imb);
        void switchToQWERTY(QinIMBase* imb);
        void changeNormalKeyMap(QinIMBase* imb);
        void changeShiftKeyMap(QinIMBase* imb);
        bool getNumbersVisible();
        void reset(); // to reset variables before showing keyboard
        LayoutTypes getLayoutType(QString lang);
        //    Languages getLanguageEnum(QString lang);
        void switchLayoutType(LayoutTypes type);


    signals:
        void keyboardFinished();
        void keyPressed(int numChars);

    private:
        QinEngine* imEngine;
        QWidget* selectPanel;
        bool Pressed;
        bool location;
        int IMIndex;
        int opacity;
        int* selkeys;
        //QVector<QString> regedIMs;
        QSignalMapper *signalMapper;
        QSignalMapper *candSignalMapper;
        QList<QToolButton*> allButtons;
        QVector<QPushButton*> candButtons;
        QStringList numbers;
        bool numbersVisible;
        bool keysAllowed;
        //Ui::QVirtualKeyboard *ui;

    private slots:
        void s_on_btn_clicked(int btn);
        void s_on_btnCands_clicked(int btn);
        void on_btnShiftLeft_clicked(bool checked);
        void on_btnIMToggle_clicked(void);
        void on_btnNext_clicked(void);
        void debounce(void);
        bool isTextKey(int keyId);

        void changeNormalUnicodeKeyMap(QinIMBase* imb);
        void changeShiftUnicodeKeyMap(QinIMBase* imb);
};

#endif /* QVIRTUALKEYBOARD_H */
