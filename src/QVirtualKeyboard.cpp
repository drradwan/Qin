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

#include "QVirtualKeyboard.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QPushButton>
#include <QSignalMapper>
#include <QTextStream>

#include "QinEngine.h"
#include "QinIMBases.h"

QVirtualKeyboard::QVirtualKeyboard(QinEngine* im)
:QWidget(0, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    /* setup UI */
    setupUi(this);
    this->move((QApplication::desktop()->width() - width())/2,
            QApplication::desktop()->height() - height());

    /* Setup selectPanel */
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(2, 0, 2, 0);
    layout->setSpacing(6);
    selectPanel = new QWidget(this, Qt::Tool | Qt::FramelessWindowHint);
    selectPanel->move((QApplication::desktop()->width() - width())/2,
            QApplication::desktop()->height() - height() - 70);
    selectPanel->setMinimumSize(width(), 70);
    selectPanel->setMaximumSize(width(), 70);
    selectPanel->setLayout(layout);
    clearCandStrBar(false);
    selectPanel->hide();

    /* Setup members */
    imEngine = im;
    Capsed = false;
    Shifted = false;
    location = 0;
    IMIndex = 0;
    candSignalMapper = NULL;
    isQWERTY = true;
    keysAllowed = true;

    /* Setup buttons */
    allButtons = findChildren<QToolButton*>();
    signalMapper = new QSignalMapper(this);

    QFile data1("/usr/local/lib/qin/SelectPanel.qss");
    if (data1.open(QFile::ReadOnly)) {
        QTextStream ssin1(&data1);
        selectPanel->setStyleSheet(ssin1.readAll());
        data1.close();
    } else {
#ifdef DEBUG
        qDebug() << "Error: failed to set style sheet for selectPanel!";
#endif
    }

    QFile data2("/usr/local/lib/qin/KeyboardLayout.qss");
    if (data2.open(QFile::ReadOnly)) {
        QTextStream ssin2(&data2);
        this->setStyleSheet(ssin2.readAll());
        data2.close();
    } else {
#ifdef DEBUG
        qDebug() << "Error: failed to set style sheet for ui!";
#endif
    }


    int i = 0;
    for (i; i < allButtons.count(); i++) {
        connect(allButtons.at(i), SIGNAL(clicked()), signalMapper, SLOT(map()));
        signalMapper->setMapping(allButtons.at(i), i);
    }

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(s_on_btn_clicked(int)));
    connect(this, SIGNAL(keyboardFinished()), imEngine, SIGNAL(keyboardFinished()));
    connect(this, SIGNAL(keyPressed(int)), imEngine, SIGNAL(keyPressed(int)));
    numbers << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0";

    // to avoid stealing focus
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 2014-10-09, new setup for buttons
    btnNext->setText(tr("OK"));
    btnBackSpace->setText("");
    btnBackSpace->setIcon(QIcon(":/images/kb_deleteKey.png"));
    btnBackSpace->setIconSize(QSize(100, 56));
}

QVirtualKeyboard::~QVirtualKeyboard() {
    delete signalMapper;
}

void QVirtualKeyboard::debounce() {
    keysAllowed = true;
}

void QVirtualKeyboard::setDefaultIMName() {
    if (imEngine->activeInputMethods.size() > 1)
        btnIMToggle->setText(imEngine->activeInputMethods[1]->imName);
    else
        btnIMToggle->setText(imEngine->activeInputMethods[0]->imName);
}

void QVirtualKeyboard::hideAll(void) {
    clearCandStrBar(false);
    hide();
}

void QVirtualKeyboard::setShift(bool shifted, bool capsed) {
    // default: Shifted = false; Capsed = false; btnShiftLeft = unchecked
    // this function is used to set state
    btnNext->setText(tr("OK"));
    if (imEngine->currentIM->symbols) {
        if (symbolCapsed) {
            symbolCapsed = false;
            changeNormalKeyMap(imEngine->currentIM);
            btnShiftLeft->setText(QString::fromUtf8("€¥¿"));
        } else {
            symbolCapsed = true;
            changeShiftKeyMap(imEngine->currentIM);
            btnShiftLeft->setText(QString::fromUtf8(".?!@"));
        }
        btnShiftLeft->setIcon(QIcon());
        return;
    }
    Shifted = shifted;
    Capsed = capsed;
    Pressed = false;
    btnShiftLeft->setText("");
    btnShiftLeft->setIconSize(QSize(100, 56));
    if (Capsed) // state: 1x
    {
#ifdef DEBUG
        qDebug("To state 1x - Capsed");
#endif
        btnShiftLeft->setIcon(QIcon(":/images/kb_caps-arrow.png"));
        changeShiftKeyMap(imEngine->currentIM);
    }
    else if (Shifted) // state: 01
    {
#ifdef DEBUG
        qDebug("To state 01 - Shifted");
#endif
        btnShiftLeft->setIcon(QIcon(":/images/kb_shift-arrow.png"));
        changeShiftKeyMap(imEngine->currentIM);
    }
    else // state: 00
    {
#ifdef DEBUG
        qDebug("To state 00 - Normal");
#endif
        btnShiftLeft->setIcon(QIcon(":/images/kb_shift-arrow-hollow.png"));
        changeNormalKeyMap(imEngine->currentIM);
    }
    //  btnShiftLeft->repaint();
}

void QVirtualKeyboard::on_btnNext_clicked(void) {
    if (!keysAllowed)
        return;
    emit keyboardFinished();
    keysAllowed = false;  
    QTimer::singleShot(500, this, SLOT(debounce()));
#ifdef DEBUG
    qDebug() << "emitting keyboardFinished() from Qin";
#endif
}

void QVirtualKeyboard::s_on_btn_clicked(int btn) {
    QString strKeyId = allButtons.at(btn)->whatsThis();
    bool isOK;
    int keyId = strKeyId.toInt(&isOK, 16);
    bool istextkey = isTextKey(keyId);

    if (keysAllowed == false && keyId != Qt::Key_Backspace)
        return;

    if (strKeyId.isEmpty() || !isOK)
        return;

    Qt::KeyboardModifiers Modifier = Qt::NoModifier;

    if ((Shifted || Capsed) && imEngine->currentIM->shiftable) {
        Modifier = Modifier | Qt::ShiftModifier;
        //    Database handles both capital and non-capital characters, hence no need conversion here
        //    keyId = tolower(keyId);
    }

    QString ch = allButtons.at(btn)->text().trimmed();
    int uni = 0;

    if (!istextkey) {
        ch = QString();
    }

    if (keyId == Qt::Key_Space)
        ch = QString(" ");

    if (keyId == Qt::Key_Backspace)
        clearCandStrBar(true);

    uni = ch.unicode()[0].unicode();
    QWSServer::sendKeyEvent(uni, keyId, Modifier, true, false);

    // Only change from Shifted to Normal when not Symbols keyboard
    if (!imEngine->currentIM->symbols && istextkey && Shifted) {
#ifdef DEBUG
        qDebug() << "Change from Shifted to Normal";
#endif
        setShift(false, false);
    }

    char* preedit= imEngine->currentIM->getPreEditString();
    if (preedit)
    {
        keyPressed(strlen(preedit));
        delete [] preedit;
    }
    else // Sent to the GUI to know how many characters are in the preedit string
        keyPressed(0);

    keysAllowed = false;  
    QTimer::singleShot(100, this, SLOT(debounce()));
}

void QVirtualKeyboard::on_btnShiftLeft_clicked(bool checked)
{
#ifdef DEBUG
    qDebug() << "btnShiftLeft_clicked";
#endif
    if (!keysAllowed)
        return;
#ifdef DEBUG
    qDebug()<< "btnShiftLeft_clicked Capsed: " << Capsed << "; Shifted: " << Shifted << " ;checked: " << checked;
#endif
    if (Capsed)
    {
        setShift(false, false);
    }
    else if (Shifted)
    {
        if (!imEngine->currentIM->capsable)
        {
            setShift(false, false);
        }
        else
        {
            setShift(false, true);
        }
    }
    else
    {
        if (imEngine->currentIM->shiftable)
        {
            setShift(true, false);
        }
        else if (imEngine->currentIM->capsable)
        {
            setShift(false, true);
        }
    }
    keysAllowed = false;
    QTimer::singleShot(300, this, SLOT(debounce()));
}

void QVirtualKeyboard::on_btnIMToggle_clicked(void) {
    if (!keysAllowed)
        return;

    bool doClearCandidateList = imEngine->currentIM->isPreEditing();
#ifdef DEBUG
    qDebug() << "Before IMToggle: isPreEditing = " << doClearCandidateList;
#endif

    // next IM to switch
    IMIndex = (IMIndex + 1) % imEngine->activeInputMethods.size();
    QinIMBase* nextIMBase = imEngine->activeInputMethods[IMIndex];
    // Do we need to clear candidates list?
    if (doClearCandidateList && nextIMBase->symbols)
    {
#ifdef DEBUG
        qDebug() << "commit first candidate from list and exit pre-edit mode before toggling Input Method";
#endif
        // exit preedit mode
        QWSServer::sendKeyEvent(0, Qt::Key_Execute, Qt::NoModifier, true, false);
    }

    // Check capsable and shiftable
    Capsed &= nextIMBase->capsable;
    Shifted &= nextIMBase->shiftable;

    imEngine->setCurrentIM(IMIndex);
    btnIMToggle->setText(imEngine->nextIM->name());
    if (imEngine->currentIM->getUseCustomKeyMap()) {
        LayoutTypes type = imEngine->currentIM->symbols? QWERTY_1: getLayoutType(imEngine->currentLanguage);
        switchLayoutType(type);
    }

    if (imEngine->currentIM->symbols) {
#ifdef DEBUG
        qDebug() << "Go to symbols";
#endif
        symbolCapsed = false;
        btnShiftLeft->setIcon(QIcon());
        btnShiftLeft->setText(QString::fromUtf8("€¥¿"));
        changeNormalKeyMap(imEngine->currentIM);
    } else {
        setShift(Shifted, Capsed);
    }
    keysAllowed = false;  
    QTimer::singleShot(300, this, SLOT(debounce()));
}

void QVirtualKeyboard::changeNormalKeyMap(QinIMBase* imb) {

    btnQ->setText(imb->fromStdKB("q"));
    btnW->setText(imb->fromStdKB("w"));
    btnE->setText(imb->fromStdKB("e"));
    btnR->setText(imb->fromStdKB("r"));
    btnT->setText(imb->fromStdKB("t"));
    btnY->setText(imb->fromStdKB("y"));
    btnU->setText(imb->fromStdKB("u"));
    btnI->setText(imb->fromStdKB("i"));
    btnO->setText(imb->fromStdKB("o"));
    btnP->setText(imb->fromStdKB("p"));
    btnAddition1->setText(imb->fromStdKB("r1_11"));

    btnA->setText(imb->fromStdKB("a"));
    btnS->setText(imb->fromStdKB("s"));
    btnD->setText(imb->fromStdKB("d"));
    btnF->setText(imb->fromStdKB("f"));
    btnG->setText(imb->fromStdKB("g"));
    btnH->setText(imb->fromStdKB("h"));
    btnJ->setText(imb->fromStdKB("j"));
    btnK->setText(imb->fromStdKB("k"));
    btnL->setText(imb->fromStdKB("l"));
    btnMalt->setText(imb->fromStdKB("r2_10"));
    btnAddition2->setText(imb->fromStdKB("r2_11"));

    btnZ->setText(imb->fromStdKB("z"));
    btnX->setText(imb->fromStdKB("x"));
    btnC->setText(imb->fromStdKB("c"));
    btnV->setText(imb->fromStdKB("v"));
    btnB->setText(imb->fromStdKB("b"));
    btnN->setText(imb->fromStdKB("n"));

    btnM->setText(imb->fromStdKB("m"));


    if (imb->getPreEditable())
    {
        changeNormalUnicodeKeyMap(imb);
    }

}
void QVirtualKeyboard::changeNormalUnicodeKeyMap(QinIMBase* imb)
{
    btnQ->setWhatsThis(imb->fromStdKBUnicode("q"));
    btnW->setWhatsThis(imb->fromStdKBUnicode("w"));
    btnE->setWhatsThis(imb->fromStdKBUnicode("e"));
    btnR->setWhatsThis(imb->fromStdKBUnicode("r"));
    btnT->setWhatsThis(imb->fromStdKBUnicode("t"));
    btnY->setWhatsThis(imb->fromStdKBUnicode("y"));
    btnU->setWhatsThis(imb->fromStdKBUnicode("u"));
    btnI->setWhatsThis(imb->fromStdKBUnicode("i"));
    btnO->setWhatsThis(imb->fromStdKBUnicode("o"));
    btnP->setWhatsThis(imb->fromStdKBUnicode("p"));
    btnAddition1->setWhatsThis(imb->fromStdKBUnicode("r1_11"));

    btnA->setWhatsThis(imb->fromStdKBUnicode("a"));
    btnS->setWhatsThis(imb->fromStdKBUnicode("s"));
    btnD->setWhatsThis(imb->fromStdKBUnicode("d"));
    btnF->setWhatsThis(imb->fromStdKBUnicode("f"));
    btnG->setWhatsThis(imb->fromStdKBUnicode("g"));
    btnH->setWhatsThis(imb->fromStdKBUnicode("h"));
    btnJ->setWhatsThis(imb->fromStdKBUnicode("j"));
    btnK->setWhatsThis(imb->fromStdKBUnicode("k"));
    btnL->setWhatsThis(imb->fromStdKBUnicode("l"));
    btnMalt->setWhatsThis(imb->fromStdKBUnicode("r2_10"));
    btnAddition2->setWhatsThis(imb->fromStdKBUnicode("r2_11"));

    btnZ->setWhatsThis(imb->fromStdKBUnicode("z"));
    btnX->setWhatsThis(imb->fromStdKBUnicode("x"));
    btnC->setWhatsThis(imb->fromStdKBUnicode("c"));
    btnV->setWhatsThis(imb->fromStdKBUnicode("v"));
    btnB->setWhatsThis(imb->fromStdKBUnicode("b"));
    btnN->setWhatsThis(imb->fromStdKBUnicode("n"));
    btnM->setWhatsThis(imb->fromStdKBUnicode("m"));
}
void QVirtualKeyboard::changeShiftUnicodeKeyMap(QinIMBase* imb)
{
    btnQ->setWhatsThis(imb->fromShiftStdKBUnicode("q"));
    btnW->setWhatsThis(imb->fromShiftStdKBUnicode("w"));
    btnE->setWhatsThis(imb->fromShiftStdKBUnicode("e"));
    btnR->setWhatsThis(imb->fromShiftStdKBUnicode("r"));
    btnT->setWhatsThis(imb->fromShiftStdKBUnicode("t"));
    btnY->setWhatsThis(imb->fromShiftStdKBUnicode("y"));
    btnU->setWhatsThis(imb->fromShiftStdKBUnicode("u"));
    btnI->setWhatsThis(imb->fromShiftStdKBUnicode("i"));
    btnO->setWhatsThis(imb->fromShiftStdKBUnicode("o"));
    btnP->setWhatsThis(imb->fromShiftStdKBUnicode("p"));
    btnAddition1->setWhatsThis(imb->fromShiftStdKBUnicode("r1_11"));

    btnA->setWhatsThis(imb->fromShiftStdKBUnicode("a"));
    btnS->setWhatsThis(imb->fromShiftStdKBUnicode("s"));
    btnD->setWhatsThis(imb->fromShiftStdKBUnicode("d"));
    btnF->setWhatsThis(imb->fromShiftStdKBUnicode("f"));
    btnG->setWhatsThis(imb->fromShiftStdKBUnicode("g"));
    btnH->setWhatsThis(imb->fromShiftStdKBUnicode("h"));
    btnJ->setWhatsThis(imb->fromShiftStdKBUnicode("j"));
    btnK->setWhatsThis(imb->fromShiftStdKBUnicode("k"));
    btnL->setWhatsThis(imb->fromShiftStdKBUnicode("l"));
    btnMalt->setWhatsThis(imb->fromShiftStdKBUnicode("r2_10"));
    btnAddition2->setWhatsThis(imb->fromShiftStdKBUnicode("r2_11"));

    btnZ->setWhatsThis(imb->fromShiftStdKBUnicode("z"));
    btnX->setWhatsThis(imb->fromShiftStdKBUnicode("x"));
    btnC->setWhatsThis(imb->fromShiftStdKBUnicode("c"));
    btnV->setWhatsThis(imb->fromShiftStdKBUnicode("v"));
    btnB->setWhatsThis(imb->fromShiftStdKBUnicode("b"));
    btnN->setWhatsThis(imb->fromShiftStdKBUnicode("n"));
    btnM->setWhatsThis(imb->fromShiftStdKBUnicode("m"));
}

void QVirtualKeyboard::changeShiftKeyMap(QinIMBase* imb) {

    btnQ->setText(imb->fromShiftStdKB("q"));
    btnW->setText(imb->fromShiftStdKB("w"));
    btnE->setText(imb->fromShiftStdKB("e"));
    btnR->setText(imb->fromShiftStdKB("r"));
    btnT->setText(imb->fromShiftStdKB("t"));
    btnY->setText(imb->fromShiftStdKB("y"));
    btnU->setText(imb->fromShiftStdKB("u"));
    btnI->setText(imb->fromShiftStdKB("i"));
    btnO->setText(imb->fromShiftStdKB("o"));
    btnP->setText(imb->fromShiftStdKB("p"));
    btnAddition1->setText(imb->fromShiftStdKB("r1_11"));

    btnA->setText(imb->fromShiftStdKB("a"));
    btnS->setText(imb->fromShiftStdKB("s"));
    btnD->setText(imb->fromShiftStdKB("d"));
    btnF->setText(imb->fromShiftStdKB("f"));
    btnG->setText(imb->fromShiftStdKB("g"));
    btnH->setText(imb->fromShiftStdKB("h"));
    btnJ->setText(imb->fromShiftStdKB("j"));
    btnK->setText(imb->fromShiftStdKB("k"));
    btnL->setText(imb->fromShiftStdKB("l"));
    btnMalt->setText(imb->fromShiftStdKB("r2_10"));
    btnAddition2->setText(imb->fromShiftStdKB("r2_11"));

    btnZ->setText(imb->fromShiftStdKB("z"));
    btnX->setText(imb->fromShiftStdKB("x"));
    btnC->setText(imb->fromShiftStdKB("c"));
    btnV->setText(imb->fromShiftStdKB("v"));
    btnB->setText(imb->fromShiftStdKB("b"));
    btnN->setText(imb->fromShiftStdKB("n"));
    btnM->setText(imb->fromShiftStdKB("m"));

    if (imb->getPreEditable())
    {
        changeShiftUnicodeKeyMap(imb);
    }
}

void QVirtualKeyboard::switchToAZERTY(QinIMBase* imb) {
    QString tempStr = btnQ->whatsThis();
    btnQ->setWhatsThis(btnA->whatsThis());
    btnA->setWhatsThis(tempStr);

    tempStr = btnW->whatsThis();
    btnW->setWhatsThis(btnZ->whatsThis());
    btnZ->setWhatsThis(tempStr);

    btnMalt->setWhatsThis(btnM->whatsThis());
    btnM->setText(QString("'"));
    btnM->setWhatsThis(QString("0x27"));
    btnMalt->setVisible(true);

    horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
    isQWERTY = false;
}

void QVirtualKeyboard::switchToQWERTY(QinIMBase* imb) {
    QString tempStr = btnQ->whatsThis();
    btnQ->setWhatsThis(btnA->whatsThis());
    btnA->setWhatsThis(tempStr);

    tempStr = btnW->whatsThis();
    btnW->setWhatsThis(btnZ->whatsThis());
    btnZ->setWhatsThis(tempStr);

    btnM->setVisible(true);
    btnMalt->setVisible(false);
    if (Shifted || Capsed)
        btnM->setText(imb->fromShiftStdKB("m"));
    else
        btnM->setText(imb->fromStdKB("m"));
    btnM->setWhatsThis(QString("0x6d"));

    horizontalLayout_3->setContentsMargins(40, 0, 40, 0);
    isQWERTY = true;
}


bool QVirtualKeyboard::isTextKey(int keyId)
{
    return !(keyId == Qt::Key_Shift
            || keyId == Qt::Key_Control
            || keyId == Qt::Key_Tab
            || keyId == Qt::Key_Return
            || keyId == Qt::Key_Enter
            || keyId == Qt::Key_CapsLock
            || keyId == Qt::Key_Backspace
            || keyId == Qt::Key_Alt);
}

void QVirtualKeyboard::s_on_btnCands_clicked(int btn) {
    if (!keysAllowed)
        return;
    QString strKeyId = candButtons[btn]->whatsThis();

    int uni = 0;
    bool isOK;
    int keyId = strKeyId.toInt(&isOK, 16);
    if (keyId)
        uni = strKeyId.unicode()[0].unicode();

#ifdef DEBUG
    qDebug() << "DEBUG: selected = " << btn << "(" << strKeyId << ")";
#endif
    if (imEngine->currentIM->getCandidateArrows() && !numbersVisible) {
        if (btn == 0)
            QWSServer::sendKeyEvent(0, Qt::Key_Left, Qt::NoModifier, true, false);
        else if (btn == candButtons.size()-1)
            QWSServer::sendKeyEvent(0, Qt::Key_Right, Qt::NoModifier, true, false);
        else
            QWSServer::sendKeyEvent(uni, keyId, Qt::NoModifier, true, false);
    } else {
        QWSServer::sendKeyEvent(uni, keyId, Qt::NoModifier, true, false);
    }
    keysAllowed = false;  
    QTimer::singleShot(200, this, SLOT(debounce()));
}

void QVirtualKeyboard::clearCandStrBar(bool showNumbers) {
    if (showNumbers) {
        if (!numbersVisible) {
            for (int i = 0; i < candButtons.size(); ++i) {
                selectPanel->layout()->removeWidget(candButtons[i]);
                delete candButtons[i];
            }
            candButtons.clear();
            showCandStrBar(numbers);
            numbersVisible = true;
        }
        return;
    } else {
        for (int i = 0; i < candButtons.size(); ++i) {
            selectPanel->layout()->removeWidget(candButtons[i]);
            delete candButtons[i];
        }
        candButtons.clear();
        QFile data("/usr/local/lib/qin/SelectPanel.qss");
        if (data.open(QFile::ReadOnly)) {
            QTextStream ssin(&data);
            selectPanel->setStyleSheet(ssin.readAll());
            data.close();
        } else {
#ifdef DEBUG
            qDebug() << "Error: failed to set style sheet for selectPanel!";
#endif
        }
        selectPanel->hide();
        numbersVisible = false;
    }
}

void QVirtualKeyboard::showCandStrBar(QStringList strlist) {
    QPushButton* button = NULL;
    int keys[] = SELKEYS;
    int i = 0;
    bool showArrows;

#ifdef DEBUG
    qDebug() << "DEBUG: cands: " << strlist;
#endif

    /* Make sure previous is cleared */
    if (strlist == numbers) {
        if (numbersVisible) {
            return;
        } else {
            clearCandStrBar(false);
            numbersVisible = true;
        }
    } else {
        clearCandStrBar(false);
        numbersVisible = false;
    }

    if (!strlist.size()) return;

    selectPanel->show();

    showArrows = (imEngine->currentIM->getCandidateArrows() && !numbersVisible);  
    if (showArrows) {
        button = new QPushButton("←");
        candButtons.push_back(button);
        selectPanel->layout()->addWidget(button);
        button->show();
    }

    // Question: Will candidates list for Chinese exceed 11?
    for (i = 0; i < strlist.size(); ++i) {
        button = new QPushButton(strlist[i]);
        if (showArrows)
            button->setDown(true);
        candButtons.push_back(button);
        selectPanel->layout()->addWidget(button);
        button->show();
    }
    if (showArrows) {
        button = new QPushButton("→");
        candButtons.push_back(button);
        selectPanel->layout()->addWidget(button);
        button->show();
    }

    if (candSignalMapper) {
        delete candSignalMapper;
        candSignalMapper = NULL;
    }

    candSignalMapper = new QSignalMapper(selectPanel);
    for (i = 0; i < candButtons.size(); i++) {
        if (showArrows && i > 0)
            candButtons[i]->setWhatsThis(QString("%1").arg(keys[i-1], 2, 16));  // candidate string starts from 2nd button since 1st button is arrow
        else
            candButtons[i]->setWhatsThis(QString("%1").arg(keys[i], 2, 16)); // candidate string starts from 1st button
        connect(candButtons[i], SIGNAL(clicked()), candSignalMapper, SLOT(map()));
        candSignalMapper->setMapping(candButtons[i], i);
    }
    connect(candSignalMapper, SIGNAL(mapped(int)), this,
            SLOT(s_on_btnCands_clicked(int)));
}

bool QVirtualKeyboard::getNumbersVisible() {
    return numbersVisible;
}

void QVirtualKeyboard::reset()
{
    IMIndex = 0;
}

QVirtualKeyboard::LayoutTypes QVirtualKeyboard::getLayoutType(QString language)
{
    QStringRef langCode(&language, 0, 2);
    QString lang = langCode.toString();
    LayoutTypes type = QWERTY_1; //default
    QStringList listQwerty_1;
    // English, German, Italian, Polish, Turkish, Portuguese, Chinese
    listQwerty_1 << "en" << "de" << "it" << "pl" << "tr" << "pt" << "zh";
    QStringList listQwerty_2;
    // Spanish
    listQwerty_2 << "es";
    QStringList listQwerty_3;
    // Danish, Norwegian, Finnish, Swedish
    listQwerty_3 << "da" << "nb" << "fi" << "sv";
    QStringList listQwerty_4;
    // Russian
    listQwerty_4 << "ru";
    QStringList listAzerty_1;
    // French
    listAzerty_1 << "fr";
    QStringList listAzerty_2;
    // French
    listAzerty_2 << "nl";
    if (listAzerty_1.contains(lang, Qt::CaseInsensitive))
    {
        type = AZERTY_1;
    }
    else if (listAzerty_2.contains(lang, Qt::CaseInsensitive))
    {
        type = AZERTY_2;
    }
    else if (listQwerty_1.contains(lang, Qt::CaseInsensitive))
    {
        type = QWERTY_1;
    }
    else if (listQwerty_2.contains(lang, Qt::CaseInsensitive))
    {
        type = QWERTY_2;
    }
    else if (listQwerty_3.contains(lang, Qt::CaseInsensitive))
    {
        type = QWERTY_3;
    }
    else if (listQwerty_4.contains(lang, Qt::CaseInsensitive))
    {
        type = QWERTY_4;
    }
    else
    {
#ifdef DEBUG
        qDebug() << "Unsupported language " << lang;
#endif
    }
    return type;
}
void QVirtualKeyboard::switchLayoutType(LayoutTypes type)
{    
    switch(type)
    {
        case QWERTY_2:  // [10, 10, 7]
            btnMalt->setVisible(true);
            btnAddition1->setVisible(false);
            btnAddition2->setVisible(false);
            btnAddition3->setVisible(false);
            btnAddition4->setVisible(false);
            horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
            break;
        case QWERTY_3:  // [11, 11, 7]
            btnMalt->setVisible(true);
            btnAddition1->setVisible(true);
            btnAddition2->setVisible(true);
            btnAddition3->setVisible(false);
            btnAddition4->setVisible(false);
            horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
            break;
        case QWERTY_4:  // [11, 11, 9]
            btnMalt->setVisible(true);
            btnAddition1->setVisible(true);
            btnAddition2->setVisible(true);
            // 2 more button for horizontalLayout_4
            btnAddition3->setVisible(true);
            btnAddition4->setVisible(true);
            horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
            break;
        case AZERTY_1:  // [10, 10, 7]
            btnMalt->setVisible(true);
            btnAddition1->setVisible(false);
            btnAddition2->setVisible(false);
            btnAddition3->setVisible(false);
            btnAddition4->setVisible(false);
            horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
            break;
        case QWERTY_1: // [10, 9, 7]
        case AZERTY_2:
        default:
            //        btnM->setVisible(true);
            btnMalt->setVisible(false);
            btnAddition1->setVisible(false);
            btnAddition2->setVisible(false);
            btnAddition3->setVisible(false);
            btnAddition4->setVisible(false);
            horizontalLayout_3->setContentsMargins(40, 0, 40, 0);
            break;
    }
}
