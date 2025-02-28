/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "krpanel.h"

#include "panelfunc.h"
#include "../abstractpanelmanager.h"

QUrl KrPanel::virtualPath() const
{
    return func->virtualDirectory();
}

KrPanel *KrPanel::otherPanel() const
{
    return _manager->otherManager()->currentPanel();
}

bool KrPanel::isLeft() const
{
    return _manager->isLeft();
}
