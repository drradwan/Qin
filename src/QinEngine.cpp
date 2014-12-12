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

#include "QinEngine.h"

#include <QDebug>
#include <QCoreApplication>
#include <string>

#include "plugins/QinPinyin.h"
//#include "plugins/QinChewing.h"
#include "QinIMBases.h"
#include "QVirtualKeyboard.h"

QinEngine::QinEngine(QString lang) {
    vkeyboard = new QVirtualKeyboard(this);
    regInputMethod(new QinIMBase(":/data/English.xml"));    // 0
    regInputMethod(new QinTableIMBase(":/data/Latin.xml")); // 1
    regInputMethod(new QinIMBase(":/data/Symbols.xml"));    // 2
    //regInputMethod(new QinTableIMBase(":/data/Boshiamy.xml"));
    //regInputMethod(new QinChewing());
    regInputMethod(new QinPinyin());                        // 3
    changeLanguage(NULL, lang);
    numbers << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0";
}

QinEngine::~QinEngine() {
    delete vkeyboard;
    for (QVector<QinIMBase*>::iterator it = inputMethods.begin();
            it != inputMethods.end(); ++it)
        delete *it;
}

void QinEngine::regInputMethod(QinIMBase* imb) {
    if (!imb) {
#ifdef DEBUG
        qDebug("Error: no input method specified\n");
#endif
        return;
    }

    inputMethods.push_back(imb);
    //vkeyboard->insertInputMethod(imb);
}

void QinEngine::setCurrentIM(int index) {
    currentIM = activeInputMethods[index];
    if (currentIM)
        pinyinEnabled = true;
    else
        pinyinEnabled = false;
    if (activeInputMethods.size() == index+1) {
        nextIM = activeInputMethods[0];
    } else {
        nextIM = activeInputMethods[index+1];
    }
    currentIM->reset();
}

bool QinEngine::filter(int uni, int keyId, int mod, bool isPress, bool autoRepeat) {
    bool doSendEvent = true;
    bool skipDefault = false;
    bool shifted = (mod & Qt::ShiftModifier) ? true : false;

    if (!isPress) //returns if key released
        return false;

#ifdef DEBUG
    qDebug("DEBUG: KeyPressed: %d, %x", uni, keyId);
#endif

    if (!currentIM->getPreEditable()) {
#ifdef DEBUG
        qDebug("Non-preeditable");
#endif
        if (keyId >= 16 && keyId <= 25) { // numbers
            sendCommitString(QString((char) keyId + 32));
            return true;
        } 

        return false; //allow further processing
    } else {
#ifdef DEBUG
        qDebug("Preeditable");
#endif
        if (keyId >= 16 && keyId <= 25) { // Either numbers of candidates
            if (vkeyboard->getNumbersVisible()) { // numbers
                sendCommitString(QString((char) keyId + 32));
                return true;
            }
            else { // candidates
                if (currentIM->getCandidateArrows()) {
                    currentIM->handle_Candidate(keyId-17); // exclude 1st button which is "go-left" arrow
#ifdef DEBUG
                    qDebug("handle_Candidate() called from QinEngine");
#endif
                    skipDefault = true;
                }
            }
        }
    }

    switch (keyId) {
        case Qt::Key_Space:
            if (currentIM->isPreEditing()) doSendEvent = false;
            currentIM->handle_Space();
            break;
        case Qt::Key_Escape: currentIM->handle_Esc(); break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            if (currentIM->isPreEditing()) doSendEvent = false;
            currentIM->handle_Enter();
            break;
        case Qt::Key_Delete: currentIM->handle_Del(); break;
        case Qt::Key_Backspace:
            if (currentIM->isPreEditing()) doSendEvent = false;
            currentIM->handle_Backspace();
            break;
        case Qt::Key_Tab: currentIM->handle_Tab(); break;
        case Qt::Key_Left:
            if (currentIM->isPreEditing()) doSendEvent = false;
            currentIM->handle_Left();
            break;
        case Qt::Key_Right:
            if (currentIM->isPreEditing()) doSendEvent = false;
            currentIM->handle_Right();
            break;
        case Qt::Key_Up: currentIM->handle_Up(); break;
        case Qt::Key_Home: currentIM->handle_Home(); break;
        case Qt::Key_End: currentIM->handle_End(); break;
        case Qt::Key_PageUp: currentIM->handle_PageUp(); break;
        case Qt::Key_PageDown: currentIM->handle_PageDown(); break;
        case Qt::Key_Down: currentIM->handle_Down(); break;
        case Qt::Key_CapsLock: currentIM->handle_Capslock(); break;
        case Qt::Key_Control: currentIM->handle_Ctrl(); break;
        case Qt::Key_Alt: currentIM->handle_Alt(); break;
        case Qt::Key_Execute: commitPreEdit(); break;
        case Qt::Key_Refresh: return true; break;
        default:
            if (keyId & Qt::Key_Escape)
                return true;
            if (!skipDefault) {
                currentIM->handle_Default(keyId, shifted);
#ifdef DEBUG
                qDebug() << "DEBUG: handle_Default() being called for currentIM";
#endif
            }
            doSendEvent = false;
    }

    updateCommitString();

    if (currentIM->getPreEditable())
        updatePreEditBuffer();

    if (currentIM->getDoPopUp())
        vkeyboard->showCandStrBar(currentIM->getPopUpStrings());

    selectPreEditWord(currentIM->cursorCurrent());
#ifdef DEBUG
    qDebug() << "QinEngine::filter-- " << doSendEvent;
#endif
    return !doSendEvent;
}

void QinEngine::updateCommitString() {
    char* commit_str = currentIM->getCommitString();
    if (commit_str) {
        sendCommitString(commit_str);
#ifdef DEBUG
        qDebug() << "DEBUG: Sending commit string: " << commit_str;
#endif
        QCoreApplication::processEvents();
    }
    delete [] commit_str;
}

void QinEngine::updatePreEditBuffer() {
    char* preedit = currentIM->getPreEditString();
    inputBuffer = QString(preedit);
    if (preedit) {
        sendPreeditString(inputBuffer, 1);
#ifdef DEBUG
        qDebug() << "DEBUG: Sending preedit string: " << inputBuffer;
#endif
        QCoreApplication::processEvents();
    } else {
        sendPreeditString(QString(), 1);
#ifdef DEBUG
        qDebug() << "DEBUG: Sending blank preedit string";
#endif
    }
    delete [] preedit;
}

void QinEngine::updateHandler(int type) {
    switch (type) {
        case QWSInputMethod::FocusIn:
            {
                if (vkeyboard->Shifted && vkeyboard->Capsed) {
                    vkeyboard->hideAll();
                    break;
                }
                currentIM->reset();
                vkeyboard->show();
                vkeyboard->showCandStrBar(numbers);
#if 0
                if (currentLanguage.contains("fr") && vkeyboard->isQWERTY) {
                    vkeyboard->switchToAZERTY(currentIM);
                } else if ((!currentLanguage.contains("fr") && !vkeyboard->isQWERTY) || currentIM->symbols) {
                    vkeyboard->switchToQWERTY(currentIM);
                }
#else
                QVirtualKeyboard::LayoutTypes layoutType = currentIM->symbols? QVirtualKeyboard::QWERTY_1: vkeyboard->getLayoutType(currentLanguage);
                vkeyboard->switchLayoutType(layoutType);
#endif
                if (vkeyboard->Shifted || vkeyboard->Capsed)
                    vkeyboard->changeShiftKeyMap(currentIM);
                else
                    vkeyboard->changeNormalKeyMap(currentIM);
            }
            break;

        case QWSInputMethod::FocusOut:
            inputBuffer.clear();
            currentIM->reset();
            vkeyboard->hideAll();
            break;
    }
}

void QinEngine::mouseHandler(int offset, int state) {
    if (state == QWSServer::MousePress && offset >= 0) {
        currentIM->setCursor(offset);
#ifdef DEBUG
        qDebug() << "mouseHandler, sendPreeditString " << inputBuffer << " " << offset << " " << state;
#endif
        sendPreeditString(inputBuffer, offset, 1);
        selected = offset;
    }
}

void QinEngine::selectPreEditWord(int index) {
    if (index != -1)
    {
#ifdef DEBUG
        qDebug() << "selectPreEditWord, sendPreeditString " << inputBuffer << " " << index;
#endif
        sendPreeditString(inputBuffer, index, 1);
    }
}

void QinEngine::setShift(bool shifted, bool capsed) {
#ifdef DEBUG
    qDebug() << "DEBUG: setShift(" << shifted << ", " << capsed << ") called by signal";
#endif
    getLineEditFocus();
    vkeyboard->reset();
    setCurrentIM(0);
    vkeyboard->setDefaultIMName();
    vkeyboard->setShift(shifted, capsed);
}

void QinEngine::commitPreEdit() {
#ifdef DEBUG
    qDebug() << "DEBUG: commitPreEdit() called by signal";
#endif
    currentIM->commit_Default();
    updateCommitString();
    vkeyboard->clearCandStrBar(true);
}

void QinEngine::getLineEditFocus() {
    // Kind of a hack here, as our QLineEdit isn't grabbing focus properly without this
    sendPreeditString(QString("A"), 1);
    sendCommitString(QString());
#ifdef DEBUG
    qDebug() << "Called getLineEditFocus()";
#endif
}

void QinEngine::changeLanguage(QString oldLang, QString newLang) {
#ifdef DEBUG
    qDebug() << "DEBUG: Qin is changing to language: " << newLang;
#endif
    currentLanguage = newLang;
    if (newLang.contains("en")) {
        activeInputMethods.clear();
        activeInputMethods.push_back(inputMethods[0]);
        activeInputMethods.push_back(inputMethods[2]);
    } else if (newLang.contains("zh")) {
        activeInputMethods.clear();
        activeInputMethods.push_back(inputMethods[3]);
        activeInputMethods.push_back(inputMethods[2]);
        activeInputMethods.push_back(inputMethods[0]);
    } else {
        // Latin-based
        activeInputMethods.clear();
        activeInputMethods.push_back(inputMethods[1]);
        activeInputMethods.push_back(inputMethods[2]);

        // setup Keymap
        inputMethods[1]->changeLanguage(currentLanguage);
    }
    defaultIM = activeInputMethods[0];
    defaultIM->setLanguageCode(currentLanguage);
    vkeyboard->reset(); //reset IMIndex to match with currentIM
    setCurrentIM(0);
    vkeyboard->setDefaultIMName();
}
