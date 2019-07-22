/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 * Copyright (C) 2010-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef KRMAINWINDOW_H
#define KRMAINWINDOW_H

#include <QAction>
#include <QWidget>

#include "abstractpanelmanager.h"

class KActionCollection;
class KrPanel;
class KrActions;
class KrView;
class ListPanelActions;
class PopularUrls;
class TabActions;
class ViewActions;

/**
 * @brief Abstract file manager main window
 */
class KrMainWindow
{
public:
    virtual ~KrMainWindow() {}
    virtual QWidget *widget() = 0;
    virtual KrView *activeView() = 0;
    virtual ViewActions *viewActions() = 0;
    virtual KActionCollection *actions() = 0;
    virtual AbstractPanelManager *activeManager() = 0;
    virtual AbstractPanelManager *leftManager() = 0;
    virtual AbstractPanelManager *rightManager() = 0;
    virtual PopularUrls *popularUrls() = 0;
    virtual KrActions *krActions() = 0;
    virtual ListPanelActions *listPanelActions() = 0;
    virtual TabActions *tabActions() = 0;
    virtual void plugActionList(const char *name, QList<QAction*> &list) = 0;

    KrPanel *activePanel() {
        return activeManager()->currentPanel();
    }
    KrPanel *leftPanel() {
        return leftManager()->currentPanel();
    }
    KrPanel *rightPanel() {
        return rightManager()->currentPanel();
    }
};

#endif // KRMAINWINDOW_H
