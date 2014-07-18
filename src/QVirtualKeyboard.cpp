/*
 * Modified by Wei-Ning Huang
 *
 * Copyright 2010 Wei-Ning Huang <aitjcize@gmail.com>
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
  layout->setContentsMargins(1, 1, 1, 0);
  layout->setSpacing(0);
  selectPanel = new QWidget(this, Qt::Tool |
                                  Qt::FramelessWindowHint);
  selectPanel->move((QApplication::desktop()->width() - width())/2,
      QApplication::desktop()->height() - height() - 55);
  selectPanel->setMinimumSize(width(), 55);
  selectPanel->setMaximumSize(width(), 55);
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

  /* Setup buttons */
  allButtons = findChildren<QToolButton*>();
  signalMapper = new QSignalMapper(this);

  QFile data(":/data/selectPanel.qss");
  if (data.open(QFile::ReadOnly)) {
    QTextStream ssin(&data);
    selectPanel->setStyleSheet(ssin.readAll());
    data.close();
  } else {
    qDebug() << "Error: failed to set style sheet for selectPanel!";
  }

  for (int i = 0; i < allButtons.count(); i++) {
    connect(allButtons.at(i), SIGNAL(clicked()), signalMapper, SLOT(map()));
    signalMapper->setMapping(allButtons.at(i), i);
  }

  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(s_on_btn_clicked(int)));
  connect(this, SIGNAL(keyboardFinished()), imEngine, SIGNAL(keyboardFinished()));
  numbers << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8" << "9" << "0";
}

QVirtualKeyboard::~QVirtualKeyboard() {
  delete signalMapper;
}

//void QVirtualKeyboard::insertInputMethod(const QinIMBase* im) {
  //regedIMs.push_back(im->name());
  //if (regedIMs.size() > 1)
  //  btnIMToggle->setText(regedIMs[1]);
  //else
  //  btnIMToggle->setText(regedIMs[0]);
  //imEngine->setCurrentIM(0);
//}

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
  qDebug() << "QVirtualKeyboard::setShift() called with " << shifted << ", " << capsed;
  Shifted = shifted;
  Capsed = capsed;
  Pressed = false;
  QTimer::singleShot(100, this, SLOT(setShift2()));
}

void QVirtualKeyboard::setShift2() {
  qDebug() << "QVirtualKeyboard::setShift2() called with " << Shifted << ", " << Capsed;
  if (Capsed) {
    btnShiftLeft->setChecked(true);
    btnShiftLeft->setText(QString::fromUtf8("⇪A"));
    btnShiftLeft->repaint();
    btnShiftLeft->setChecked(true);
    btnShiftLeft->setText(QString::fromUtf8("⇪A"));
    btnShiftLeft->repaint();
    btnShiftLeft->setChecked(true);
    btnShiftLeft->setText(QString::fromUtf8("⇪A"));
    btnShiftLeft->repaint();
    changeShiftKeyMap(imEngine->currentIM);
    qDebug() << "Capsed called";
  } else if (Shifted) {
    btnShiftLeft->setChecked(true);
    btnShiftLeft->setText(QString::fromUtf8("⇧B"));
    btnShiftLeft->repaint();
    btnShiftLeft->setChecked(true);
    btnShiftLeft->setText(QString::fromUtf8("⇧B"));
    btnShiftLeft->repaint();
    btnShiftLeft->setChecked(true);
    btnShiftLeft->setText(QString::fromUtf8("⇧B"));
    btnShiftLeft->repaint();
    changeShiftKeyMap(imEngine->currentIM);
    qDebug() << "Shifted called";
  } else {
    btnShiftLeft->setChecked(false);
    btnShiftLeft->setText(QString::fromUtf8("⇧C"));
    btnShiftLeft->repaint();
    btnShiftLeft->setChecked(false);
    btnShiftLeft->setText(QString::fromUtf8("⇧C"));
    btnShiftLeft->repaint();
    btnShiftLeft->setChecked(false);
    btnShiftLeft->setText(QString::fromUtf8("⇧C"));
    btnShiftLeft->repaint();
    changeNormalKeyMap(imEngine->currentIM);
    qDebug() << "None called";
  }
}

void QVirtualKeyboard::on_btnNext_clicked(void) {
  emit keyboardFinished();
  qDebug() << "emitting keyboardFinished() from Qin";
}

void QVirtualKeyboard::s_on_btn_clicked(int btn) {
  QString strKeyId = allButtons.at(btn)->whatsThis();
  bool isOk;
  int keyId = strKeyId.toInt(&isOk, 16);
  int involvedKeys = 1;
  bool istextkey = isTextKey(keyId);

  if (strKeyId.isEmpty() || !isOk)
    return;

  Qt::KeyboardModifiers Modifier = Qt::NoModifier;

  if (Shifted || Capsed) {
    Modifier = Modifier | Qt::ShiftModifier;
    involvedKeys++;

    switch (keyId) {
      case 0x2c:
      case 0x2e:
      case 0x2f: keyId += 0x10; break;
      case 0x3b: keyId = 0x3a; break;
      case 0x27: keyId = 0x22; break;
      case 0x5b:
      case 0x5c:
      case 0x5d: keyId += 0x20; break;
      case 0x31: keyId = 0x21; break;
      case 0x32: keyId = 0x40; break;
      case 0x33: keyId = 0x23; break;
      case 0x34: keyId = 0x24; break;
      case 0x35: keyId = 0x25; break;
      case 0x36: keyId = 0x5e; break;
      case 0x37: keyId = 0x26; break;
      case 0x38: keyId = 0x2a; break;
      case 0x39: keyId = 0x28; break;
      case 0x30: keyId = 0x29; break;
      case 0x2d: keyId = 0x5f; break;
      case 0x3d: keyId = 0x2b; break;
      default: keyId = tolower(keyId);
    }
  }

  QString ch = allButtons.at(btn)->text().trimmed();
  int uni = 0;

  if (!istextkey) {
    ch = QString();
    if (keyId == Qt::Key_Tab)
      uni = 9;
    else
      uni = 65535;
  } else {
      Pressed = true;
  }

  if (keyId == Qt::Key_Space)
    ch = QString(" ");
    
  if (keyId == Qt::Key_Backspace)
    clearCandStrBar(true);

  uni = ch.unicode()[0].unicode();
  QWSServer::sendKeyEvent(uni, keyId, Modifier, true, false);

  if (istextkey && Shifted) {
    btnShiftLeft->setChecked(false);
    changeNormalKeyMap(imEngine->currentIM);
  }
}

void QVirtualKeyboard::on_btnShiftLeft_toggled(bool checked) {
  if (Capsed) {
    Capsed = false;
    Shifted = false;
    changeNormalKeyMap(imEngine->currentIM);
    btnShiftLeft->setText(QString::fromUtf8("⇧"));
    btnShiftLeft->setChecked(false);
  } else if (Shifted) {
    if (Pressed) {
      Shifted = false;
      Capsed = false;
      changeNormalKeyMap(imEngine->currentIM);
      btnShiftLeft->setText(QString::fromUtf8("⇧"));
      btnShiftLeft->setChecked(false);
    } else {
      Shifted = false;
      Capsed = true;
      changeShiftKeyMap(imEngine->currentIM);
      btnShiftLeft->setText(QString::fromUtf8("⇪"));
      btnShiftLeft->setChecked(true);
    }
  } else {
    Capsed = false;
    Shifted = true;
    Pressed = false;
    changeShiftKeyMap(imEngine->currentIM);
    btnShiftLeft->setText(QString::fromUtf8("⇧"));
    btnShiftLeft->setChecked(true);
  }
}

void QVirtualKeyboard::on_btnIMToggle_clicked(void) {
  //IMIndex = (IMIndex + 1) % regedIMs.size();
  IMIndex = (IMIndex + 1) % imEngine->activeInputMethods.size();
  imEngine->setCurrentIM(IMIndex);
  btnIMToggle->setText(imEngine->nextIM->name());

  if (imEngine->currentIM->getUseCustomKeyMap()) {
    if (Capsed || Shifted)
      changeShiftKeyMap(imEngine->currentIM);
    else
      changeNormalKeyMap(imEngine->currentIM);
  } else {
    if (Capsed || Shifted)
      changeShiftKeyMap(imEngine->defaultIM);
    else
      changeNormalKeyMap(imEngine->defaultIM);
  }
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

  btnA->setText(imb->fromStdKB("a"));
  btnS->setText(imb->fromStdKB("s"));
  btnD->setText(imb->fromStdKB("d"));
  btnF->setText(imb->fromStdKB("f"));
  btnG->setText(imb->fromStdKB("g"));
  btnH->setText(imb->fromStdKB("h"));
  btnJ->setText(imb->fromStdKB("j"));
  btnK->setText(imb->fromStdKB("k"));
  btnL->setText(imb->fromStdKB("l"));

  btnZ->setText(imb->fromStdKB("z"));
  btnX->setText(imb->fromStdKB("x"));
  btnC->setText(imb->fromStdKB("c"));
  btnV->setText(imb->fromStdKB("v"));
  btnB->setText(imb->fromStdKB("b"));
  btnN->setText(imb->fromStdKB("n"));
  btnM->setText(imb->fromStdKB("m"));
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

  btnA->setText(imb->fromShiftStdKB("a"));
  btnS->setText(imb->fromShiftStdKB("s"));
  btnD->setText(imb->fromShiftStdKB("d"));
  btnF->setText(imb->fromShiftStdKB("f"));
  btnG->setText(imb->fromShiftStdKB("g"));
  btnH->setText(imb->fromShiftStdKB("h"));
  btnJ->setText(imb->fromShiftStdKB("j"));
  btnK->setText(imb->fromShiftStdKB("k"));
  btnL->setText(imb->fromShiftStdKB("l"));

  btnZ->setText(imb->fromShiftStdKB("z"));
  btnX->setText(imb->fromShiftStdKB("x"));
  btnC->setText(imb->fromShiftStdKB("c"));
  btnV->setText(imb->fromShiftStdKB("v"));
  btnB->setText(imb->fromShiftStdKB("b"));
  btnN->setText(imb->fromShiftStdKB("n"));
  btnM->setText(imb->fromShiftStdKB("m"));
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
  QString strKeyId = candButtons[btn]->whatsThis();
  bool isOk;
  int keyId = strKeyId.toInt(&isOk, 16);

#ifdef DEBUG
  qDebug() << "DEBUG: selected = " << btn << "(" << strKeyId << ")";
#endif

  QWSServer::sendKeyEvent(0, keyId, Qt::NoModifier, true, false);
  clearCandStrBar(true);
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
    selectPanel->hide();
    numbersVisible = false;
  }
}

void QVirtualKeyboard::showCandStrBar(QStringList strlist) {
  QPushButton* button = NULL;
  int keys[] = SELKEYS;

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
  
  for (int i = 0; i < strlist.size(); ++i) {
    button = new QPushButton(strlist[i]);
    //button->setFont(QFont("WenQuanYiMicroHeiLight", 13));
    candButtons.push_back(button);
    selectPanel->layout()->addWidget(button);
    button->show();
  }

  /* Fix border for the rightmost color, the sequence of the CSS must be
   * border-right then border-style else it won't work */
  candButtons.last()->setStyleSheet("QPushButton { border-right: 1px "
      "#8A8A8A; border-style: groove; }");

  if (candSignalMapper) {
    delete candSignalMapper;
    candSignalMapper = NULL;
  }

  candSignalMapper = new QSignalMapper(selectPanel);

  for (int i = 0; i < candButtons.size(); i++) {
    candButtons[i]->setWhatsThis(QString("%1").arg(keys[i], 2, 16));
    connect(candButtons[i], SIGNAL(clicked()), candSignalMapper, SLOT(map()));
    candSignalMapper->setMapping(candButtons[i], i);
  }
  connect(candSignalMapper, SIGNAL(mapped(int)), this,
      SLOT(s_on_btnCands_clicked(int)));
}
