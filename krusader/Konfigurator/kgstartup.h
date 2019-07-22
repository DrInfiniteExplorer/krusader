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

#ifndef KGSTARTUP_H
#define KGSTARTUP_H

#include "konfiguratorpage.h"

class KgStartup : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgStartup(bool first, QWidget* parent = nullptr);

public slots:
    void slotDisable();

protected:
    KonfiguratorRadioButtons  *saveRadio;
    KonfiguratorCheckBoxGroup *uiCbGroup;
    KonfiguratorComboBox      *profileCombo;
};

#endif /* __KGSTARTUP_H__ */
