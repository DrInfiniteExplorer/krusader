/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "feedtolistboxdialog.h"
#include "synchronizer.h"
#include "synchronizergui.h"
#include "../FileSystem/filesystem.h"
#include "../FileSystem/virtualfilesystem.h"
#include "../krglobal.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

// QtWidgets
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>

#include <KConfigCore/KConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#define  S_LEFT        0
#define  S_RIGHT       1
#define  S_BOTH        2

FeedToListBoxDialog::FeedToListBoxDialog(QWidget *parent, Synchronizer *sync,
        QTreeWidget *syncL, bool equOK) : QDialog(parent),
        synchronizer(sync), syncList(syncL), equalAllowed(equOK), accepted(false)
{

    setWindowTitle(i18n("Krusader::Feed to listbox"));
    setWindowModality(Qt::WindowModal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // autodetecting the parameters

    int selectedNum = 0;
    int itemNum = 0;
    int leftExistingNum = 0;
    int rightExistingNum = 0;

    QTreeWidgetItemIterator it(syncList);
    while (*it) {
        auto *item = dynamic_cast<SynchronizerGUI::SyncViewItem *>( * it);
        SynchronizerFileItem *syncItem = item->synchronizerItemRef();

        if (syncItem && syncItem->isMarked()) {
            if (item->isSelected() || syncItem->task() != TT_EQUALS || equalAllowed) {
                itemNum++;
                if (item->isSelected())
                    selectedNum++;

                if (syncItem->existsInLeft())
                    leftExistingNum++;
                if (syncItem->existsInRight())
                    rightExistingNum++;
            }
        }
        it++;
    }

    if (itemNum == 0) {
        hide();
        KMessageBox::error(parent, i18n("No elements to feed."));
        return;
    }

    // guessing the collection name

    VirtualFileSystem virtFilesystem;
    if (!virtFilesystem.scanDir(QUrl("virt:/")))
        return;

    KConfigGroup group(krConfig, "Synchronize");
    int listBoxNum = group.readEntry("Feed To Listbox Counter", 1);
    QString queryName;
    do {
        queryName = i18n("Synchronize results") + QString(" %1").arg(listBoxNum++);
    } while (virtFilesystem.getFileItem(queryName) != nullptr);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    // creating the widget

    QLabel *label = new QLabel(i18n("Here you can name the file collection"), this);
    mainLayout->addWidget(label);

    lineEdit = new QLineEdit(this);
    lineEdit->setText(queryName);
    lineEdit->setClearButtonEnabled(true);
    lineEdit->selectAll();
    mainLayout->addWidget(lineEdit);

    auto * hbox = new QHBoxLayout;

    QLabel *label2 = new QLabel(i18n("Side to feed:"), this);
    label2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hbox->addWidget(label2);

    sideCombo = new QComboBox(this);
    sideCombo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sideCombo->addItem(i18n("Left"));
    sideCombo->addItem(i18n("Right"));
    sideCombo->addItem(i18n("Both"));
    hbox->addWidget(sideCombo);

    if (leftExistingNum == 0) {
        sideCombo->setCurrentIndex(1);
        sideCombo->setEnabled(false);
    } else if (rightExistingNum == 0) {
        sideCombo->setCurrentIndex(0);
        sideCombo->setEnabled(false);
    } else
        sideCombo->setCurrentIndex(2);

    QFrame *line = new QFrame(this);
    line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    hbox->addWidget(line);

    cbSelected = new QCheckBox(i18n("Selected files only"), this);
    cbSelected->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    cbSelected->setEnabled(selectedNum != 0);
    cbSelected->setChecked(selectedNum != 0);
    hbox->addWidget(cbSelected);

    mainLayout->addLayout(hbox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FeedToListBoxDialog::slotOk);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FeedToListBoxDialog::reject);

    exec();
}

void FeedToListBoxDialog::slotOk()
{
    int side = sideCombo->currentIndex();
    bool selected = cbSelected->isChecked();
    QString name = lineEdit->text();
    QList<QUrl> urlList;

    QTreeWidgetItemIterator it(syncList);
    for (;*it; it++) {
        auto *item = dynamic_cast<SynchronizerGUI::SyncViewItem *>( * it);
        SynchronizerFileItem *syncItem = item->synchronizerItemRef();

        if (!syncItem || !syncItem->isMarked())
            continue;
        if (selected && !item->isSelected())
            continue;
        if (!equalAllowed && syncItem->task() == TT_EQUALS && (!selected || !item->isSelected()))
            continue;

        if ((side == S_BOTH || side == S_LEFT) && syncItem->existsInLeft()) {
            QString leftDirName = syncItem->leftDirectory().isEmpty() ? "" : syncItem->leftDirectory() + '/';
            QUrl leftURL = Synchronizer::fsUrl(synchronizer->leftBaseDirectory() + leftDirName + syncItem->leftName());
            urlList.push_back(leftURL);
        }

        if ((side == S_BOTH || side == S_RIGHT) && syncItem->existsInRight()) {
            QString rightDirName = syncItem->rightDirectory().isEmpty() ? "" : syncItem->rightDirectory() + '/';
            QUrl leftURL = Synchronizer::fsUrl(synchronizer->rightBaseDirectory() + rightDirName + syncItem->rightName());
            urlList.push_back(leftURL);
        }
    }

    QUrl url = QUrl(QString("virt:/") + name);
    VirtualFileSystem virtFilesystem;
    if (!virtFilesystem.refresh(url)) { // create directory if it does not exist
        KMessageBox::error(parentWidget(), i18n("Cannot open %1.", url.toDisplayString()));
        return;
    }
    virtFilesystem.addFiles(urlList);
    ACTIVE_MNG->slotNewTab(url);
    accepted = true;
    accept();
}

