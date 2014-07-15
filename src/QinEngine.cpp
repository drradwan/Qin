/**
 * @file   QinEngine.cpp
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

#include "QinEngine.h"

#include <QDebug>

#include "plugins/QinPinyin.h"
#include "plugins/QinChewing.h"
#include "QinIMBases.h"
#include "QVirtualKeyboard.h"

QinEngine::QinEngine(QString lang) {
  vkeyboard = new QVirtualKeyboard(this);
  if (lang.contains("en"))
    regInputMethod(new QinIMBase(":/data/English.xml"));
  if (lang.contains("en") || lang.contains("fr"))
    regInputMethod(new QinTableIMBase(":/data/Latin.xml"));
  if (lang.contains("en") || lang.contains("fr"))
    regInputMethod(new QinIMBase(":/data/Symbols.xml"));
  //regInputMethod(new QinChewing());
  //regInputMethod(new QinPinyin());
  if (lang.contains("zh"))
    regInputMethod(new QinTableIMBase(":/data/Boshiamy.xml"));
  if (lang.contains("en") || lang.contains("fr") || lang.contains("zh"))
    defaultIM = inputMethods[0];
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
    qDebug("Error: no input method specified\n");
    return;
  }

  inputMethods.push_back(imb);
  vkeyboard->insertInputMethod(imb);
}

void QinEngine::setCurrentIM(int index) {
  currentIM = inputMethods[index];
  if (inputMethods.size() == index+1) {
    nextIM = inputMethods[0];
  } else {
    nextIM = inputMethods[index+1];
  }
  currentIM->reset();
  if (currentIM->getDoPopUp())
    vkeyboard->showCandStrBar(currentIM->getPopUpStrings());
  else
    vkeyboard->showCandStrBar(numbers);
  //vkeyboard->pressShiftKey();
}

bool QinEngine::filter(int uni, int keyId, int mod, bool isPress,
    bool autoRepeat) {
  bool doSendEvent = true;
  bool shifted = (mod & Qt::ShiftModifier) ? true : false;

  if (!isPress)
    return false;

#ifdef DEBUG
  qDebug("DEBUG: KeyPressed: %d, %x", uni, keyId);
#endif

  if (!currentIM->getPreEditable()) {
    if (keyId >= 16 && keyId <= 25) {
      sendCommitString(QString((char) keyId + 32));
      return true;
    } else {
      return false;
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
      vkeyboard->clearCandStrBar(true);
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
    default:
      if (keyId & Qt::Key_Escape)
        return true;
      currentIM->handle_Default(keyId, shifted);

      doSendEvent = false;
  }

  updateCommitString();

  if (currentIM->getPreEditable())
    updatePreEditBuffer();

  if (currentIM->getDoPopUp())
    vkeyboard->showCandStrBar(currentIM->getPopUpStrings());

  selectPreEditWord(currentIM->cursorCurrent());

  return !doSendEvent;
}

void QinEngine::updateCommitString() {
  char* commit_str = currentIM->getCommitString();
  if (commit_str) {
    sendCommitString(commit_str);
    qDebug() << "Sending commit string: " << commit_str;
    delete commit_str;
  }
}

void QinEngine::updatePreEditBuffer() {
  char* preedit = currentIM->getPreEditString();
  inputBuffer = QString(preedit);
  sendPreeditString(inputBuffer, 1);
    qDebug() << "Sending preedit string: " << inputBuffer;
  delete preedit;
}

void QinEngine::updateHandler(int type) {
  switch (type) {
    case QWSInputMethod::FocusIn:
      currentIM->reset();
      vkeyboard->show();
      vkeyboard->showCandStrBar(numbers);
      vkeyboard->pressShiftKey();
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
    sendPreeditString(inputBuffer, offset, 1);
    selected = offset;
  }
}

void QinEngine::selectPreEditWord(int index) {
  if (index != -1)
    sendPreeditString(inputBuffer, index, 1);
}

void QinEngine::setShift(bool shifted, bool capsed) {
  vkeyboard->setShift(shifted, capsed);
}

void QinEngine::commitPreEdit() {
  currentIM->handle_Space();
}
