/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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

#include "kgarchives.h"

// QtCore
#include <QPointer>
// QtWidgets
#include <QPushButton>
#include <QGridLayout>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>

#include "krresulttable.h"
#include "krresulttabledialog.h"
#include "searchobject.h"
#include "../defaults.h"
#include "../krglobal.h"
#include "../Archive/krarchandler.h"

KgArchives::KgArchives(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    QWidget *innerWidget = new QFrame(this);
    setWidget(innerWidget);
    setWidgetResizable(true);
    auto *kgArchivesLayout = new QGridLayout(innerWidget);
    kgArchivesLayout->setSpacing(6);

    //  ------------------------ KRARC GROUPBOX --------------------------------

    QGroupBox *krarcGrp = createFrame(i18n("krarc ioslave"), innerWidget);
    QGridLayout *krarcGrid = createGridLayout(krarcGrp);

    KONFIGURATOR_CHECKBOX_PARAM krarcOptions[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {
        {"kio_krarc", "EnableWrite", false, i18n("Enable Write Support"), false, i18n("Enable writing to archives using the krarc ioslave.")}
    };

    KonfiguratorCheckBoxGroup *krarcCheckBoxes = createCheckBoxGroup(1, 0, krarcOptions, 1, krarcGrp);

    krarcGrid->addWidget(krarcCheckBoxes, 1, 0);
    krarcGrid->addWidget(
            new QLabel(i18n("<b>Caution when moving into archives:</b><br/>"
                            "<b>Failure during the process might result in data loss.</b><br/>"
                            "<b>Moving archives into themselves will delete them.</b>"), krarcGrp),
            2, 0);

    kgArchivesLayout->addWidget(krarcGrp, 1 , 0);

    //  ------------------------ BROWSE GROUPBOX --------------------------------

    QGroupBox *browseGrp = createFrame(i18n("Archives handling"), innerWidget);
    QGridLayout *browseGrid = createGridLayout(browseGrp);

    KONFIGURATOR_CHECKBOX_PARAM browseOptions[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {
        {"Archives", "ArchivesAsDirectories", _ArchivesAsDirectories, i18n("Browse Archives As Folders"), false, i18n("Krusader will browse archives as folders.")}
    };

    KonfiguratorCheckBoxGroup *browseCheckBoxes = createCheckBoxGroup(1, 0, browseOptions, 1, browseGrp);

    browseGrid->addWidget(browseCheckBoxes, 1, 0);

    kgArchivesLayout->addWidget(browseGrp, 2 , 0);

    //  ------------------------ FINE-TUNING GROUPBOX --------------------------------

    QGroupBox *fineTuneGrp = createFrame(i18n("Fine-Tuning"), innerWidget);
    QGridLayout *fineTuneGrid = createGridLayout(fineTuneGrp);

    KONFIGURATOR_CHECKBOX_PARAM finetuners[] =
        //   cfg_class  cfg_name                  default           text                                          restart ToolTip
    {
        {"Archives", "Test Archives",           _TestArchives,    i18n("Test archive after packing"), false,  i18n("Check the archive's integrity after packing it.")},
        {"Archives", "Test Before Unpack",      _TestBeforeUnpack, i18n("Test archive before unpacking"), false,  i18n("Some corrupted archives might cause a crash; therefore, testing is suggested.")}
    };

    KonfiguratorCheckBoxGroup *finetunes = createCheckBoxGroup(1, 0, finetuners, 2, fineTuneGrp);

    disableNonExistingPackers();
    fineTuneGrid->addWidget(finetunes, 1, 0);

    kgArchivesLayout->addWidget(fineTuneGrp, 3 , 0);

    if (first)
        slotAutoConfigure();

}


void KgArchives::slotAutoConfigure()
{
    QPointer<KrResultTableDialog> dlg = new KrResultTableDialog(this,
            KrResultTableDialog::Archiver,
            i18n("Search results"),
            i18n("Searching for packers..."),
            "utilities-file-archiver",
            i18n("Make sure to install new packers in your <code>$PATH</code> (e.g. /usr/bin)"));
    dlg->exec();

    disableNonExistingPackers();

    delete dlg;
}

void KgArchives::disableNonExistingPackers()
{
    KConfigGroup group(krConfig, "Archives");
    group.writeEntry("Supported Packers", KrArcHandler::supportedPackers());
}


