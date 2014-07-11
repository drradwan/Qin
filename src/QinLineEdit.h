/**
 * @file   QinLineEdit.h
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

#ifndef __QIN_SRC_QIN_LINE_EDIT_H__
#define __QIN_SRC_QIN_LINE_EDIT_H__

#include "QVirtualKeyboard.h"
#include "QinIMBases.h"
#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QVirtualKeyboard;
QT_END_NAMESPACE

class QinLineEdit: public QWidget {
  Q_OBJECT

  friend class QVirtualKeyboard;

  public:
    QinLineEdit(QString lang, QLineEdit* le);
    ~QinLineEdit();
    void regInputMethod(QinIMBase* imb);
    void setCurrentIM(int index);
    
  private:
    QLineEdit* lineEdit;
    QVirtualKeyboard* vkeyboard;
    QString inputBuffer;
    QVector<QinIMBase*> inputMethods;
    QinIMBase* defaultIM;
    QinIMBase* currentIM;
    QinIMBase* nextIM;
    int selected;
    QStringList numbers;

    bool filter(int uni, int keyId, int mod, bool isPress, bool autoRepeat);
    void updateHandler(int type);
    void updateCommitString();
    void updatePreEditBuffer(void);
    //void selectPreEditWord(int index);
};

#endif /* __QIN_SRC_QIN_LINE_EDIT_H__ */
