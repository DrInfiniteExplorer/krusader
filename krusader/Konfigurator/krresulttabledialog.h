/*****************************************************************************
 * Copyright (C) 2005 Dirk Eschler <deschler@users.sourceforge.net>          *
 * Copyright (C) 2005-2020 Krusader Krew [https://krusader.org]              *
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

#ifndef KRRESULTTABLEDIALOG_H
#define KRRESULTTABLEDIALOG_H

// QtWidgets
#include <QDialog>

#include "../Konfigurator/krresulttable.h"

class KrResultTableDialog : public QDialog
{
public:

    enum DialogType {
        Archiver = 1,
        Tool = 2
    };

    KrResultTableDialog(QWidget *parent, DialogType type, const QString& caption, const QString& heading, const QString& headerIcon = QString(), const QString& hint = QString());
    virtual ~KrResultTableDialog();

    const QString& getHeading() const {
        return _heading;
    }
    const QString& getHint() const {
        return _hint;
    }
    void setHeading(const QString& s) {
        _heading = s;
    }
    void setHint(const QString& s) {
        _hint = s;
    }

public slots:
    void showHelp();

protected:
    QString _heading;
    QString _hint;
    QString helpAnchor;

    KrResultTable* _resultTable;
};

#endif
