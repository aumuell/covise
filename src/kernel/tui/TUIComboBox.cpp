/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

#include <assert.h>
#include <stdio.h>

#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>

#include "TUIComboBox.h"
#include "TUIMain.h"
#include <net/tokenbuffer.h>

/// Constructor
TUIComboBox::TUIComboBox(int id, int type, QWidget *w, int parent, QString name)
    : TUIElement(id, type, w, parent, name)
{
    auto frame = createWidget<QFrame>(w);
    frame->setFrameStyle(QFrame::Plain | QFrame::NoFrame);
    frame->setContentsMargins(0, 0, 0, 0);
    combo = new QComboBox(frame);
    combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(combo, SIGNAL(activated(int)), this, SLOT(valueChanged(int)));
    auto grid = createLayout(frame);
    frame->setLayout(grid);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->addWidget(combo, 1, 0);
}

void TUIComboBox::valueChanged(int)
{
    covise::TokenBuffer tb;
    tb << ID;
    QByteArray ba = combo->currentText().toUtf8();
    tb << ba.data();
    TUIMain::getInstance()->send(tb);
}

const char *TUIComboBox::getClassName() const
{
    return "TUIComboBox";
}

void TUIComboBox::setValue(TabletValue type, covise::TokenBuffer &tb)
{
    //cerr << "setValue " << type << endl;
    if (type == TABLET_ADD_ENTRY)
    {
        const char *en;
        tb >> en;
        QString entry(en);
        combo->addItem(entry);
    }
    else if (type == TABLET_REMOVE_ENTRY)
    {
        int num = combo->count();
        int i;
        const char *en;
        tb >> en;
        QString entry(en);
        for (i = 0; i < num; i++)
        {
            if (combo->itemText(i) == entry)
            {
                combo->removeItem(i);
                break;
            }
        }
    }
    else if (type == TABLET_SELECT_ENTRY)
    {
        int num = combo->count();
        int i;
        const char *en;
        tb >> en;
        QString entry(en);
        for (i = 0; i < num; i++)
        {
            if (combo->itemText(i) == entry)
            {
                combo->setCurrentIndex(i);
                break;
            }
        }
    }
    else if (type == TABLET_REMOVE_ALL)
    {
        combo->clear();
    }
    TUIElement::setValue(type, tb);
}

void TUIComboBox::setLabel(QString textl)
{
    if (textl.isEmpty())
    {
        widgets.erase(label);
        delete label;
        label = nullptr;
    }
    else if (!label)
    {
        label = new QLabel(widget());
        getLayout()->addWidget(label, 0, 0);
    }
    if (label)
        label->setText(textl);
}