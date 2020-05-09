/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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


#ifndef KCMDLINE_H
#define KCMDLINE_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QToolButton>

#include <KCompletion/KLineEdit>
#include <KIOWidgets/KShellCompletion>

#include "../GUI/krhistorycombobox.h"
#include "../UserAction/kractionbase.h"

class KCMDModeButton;

class CmdLineCombo : public KrHistoryComboBox
{
    Q_OBJECT
public:
    explicit CmdLineCombo(QWidget *parent);

    bool eventFilter(QObject *watched, QEvent *e) override;

    QString path() {
        return _path;
    }
    void setPath(QString path);

signals:
    void returnToPanel();

protected slots:
    void doLayout();

protected:
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

    void updateLineEditGeometry();

    QLabel *_pathLabel;
    QString _path;
    bool _handlingLineEditResize;
};


class KCMDLine : public QWidget, KrActionBase
{
    Q_OBJECT
public:
    explicit KCMDLine(QWidget *parent = nullptr);
    ~KCMDLine() override;
    void setCurrent(const QString &path);
    //virtual methods from KrActionBase
    void setText(const QString& text);
    QString command() const override;
    ExecType execType() const override;
    QString startpath() const override;
    QString user() const override;
    QString text() const override;
    bool acceptURLs() const override;
    bool confirmExecution() const override;
    bool doSubstitution() const override;

signals:
    void signalRun();

public slots:
    void slotReturnFocus(); // returns keyboard focus to panel
    void slotRun();
    void addPlaceholder();
    void addText(const QString& text) {
        cmdLine->lineEdit()->setText(cmdLine->lineEdit()->text() + text);
    }
    void popup() {
        cmdLine->showPopup();
    }

protected:
    void focusInEvent(QFocusEvent*) override {
        cmdLine->setFocus();
    }

    void calcLabelSize();

private:
    CmdLineCombo *cmdLine;
    KCMDModeButton *terminal;
    QToolButton *buttonAddPlaceholder;
    KShellCompletion completion;
};

#endif
