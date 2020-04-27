/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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

#ifndef KRUSADER_H
#define KRUSADER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "krmainwindow.h"

// QtCore
#include <QCommandLineParser>
#include <QEvent>
#include <QPointer>
#include <QStringList>
#include <QTimer>
// QtGui
#include <QHideEvent>
#include <QMoveEvent>
#include <QResizeEvent>
#include <QShowEvent>
// QtWidgets
#include <QAction>

#include <KConfigCore/KConfig>
#include <KConfigCore/KConfigGroup>
#include <KConfigWidgets/KStandardAction>
#include <KNotifications/KStatusNotifierItem>
#include <KParts/MainWindow>

#ifdef __KJSEMBED__
class KrJS;
#endif

class KStartupInfoData;
class KStartupInfoId;

class KrusaderStatus;
class KrPleaseWaitHandler;
class PopularUrls;
class ViewActions;
class ListPanelActions;
class TabActions;
class KrView;

/**
 * @brief The main window of this file manager
 */
class Krusader : public KParts::MainWindow, public KrMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.krusader.Instance")

public:
    explicit Krusader(const QCommandLineParser &parser);
    ~Krusader() override;

    void setTray(bool forceCreation = false);

    // KrMainWindow implementation
    QWidget *widget() override {
        return this;
    }
    KrView *activeView() override;
    ViewActions *viewActions() override {
        return _viewActions;
    }
    KActionCollection *actions() override {
        return actionCollection();
    }
    AbstractPanelManager *activeManager() override;
    AbstractPanelManager *leftManager() override;
    AbstractPanelManager *rightManager() override;
    PopularUrls *popularUrls() override {
        return _popularUrls;
    }
    KrActions *krActions() override {
        return _krActions;
    }
    ListPanelActions *listPanelActions() override {
        return _listPanelActions;
    }
    TabActions *tabActions() override {
        return _tabActions;
    }
    void plugActionList(const char *name, QList<QAction*> &list) override {
        KParts::MainWindow::plugActionList(name, list);
    }

    /**
     * Icon name that depends on whether krusader runs with root-privileges or not
     *
     * @return icon name
     */
    static const char* appIconName();

public slots:
    void quit();
    void moveToTop();
    void statusBarUpdate(const QString& mess);
    // in use by Krusader only
    void saveSettings();
    void savePosition();
    void updateUserActions();

protected slots:
    void doOpenUrl();
    void slotGotNewStartup(const KStartupInfoId &id, const KStartupInfoData &data);
    void slotGotRemoveStartup(const KStartupInfoId &id, const KStartupInfoData &data);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool queryClose() override;
    void setupActions();
    bool versionControl();  // handle version differences in krusaderrc
    bool event(QEvent *) override;

public Q_SLOTS:
    Q_SCRIPTABLE bool isRunning();
    Q_SCRIPTABLE bool isLeftActive();
    Q_SCRIPTABLE bool openUrl(QString url);

public:
    static Krusader *App;       // a kApp style pointer
    static QString   AppName;   // the name of the application
    PopularUrls *_popularUrls; // holds a sorted list of the most popular urls visited

    // the internal progress bar variales + functions
    KrPleaseWaitHandler* plzWait;
    void startWaiting(QString msg = "Please Wait", int count = 0 , bool cancel = false);
    void stopWait();
    bool wasWaitingCancelled() const;

#ifdef __KJSEMBED__
    static KrJS *js;
#endif

signals:
    void changeMessage(QString);
    // emitted when we are about to quit
    void shutdown();

private:
    void acceptClose();

private:
    KrActions *_krActions;
    ViewActions *_viewActions;
    ListPanelActions *_listPanelActions;
    TabActions *_tabActions;
    QPointer<KStatusNotifierItem> sysTray;
    bool         isStarting;
    QTimer      _openUrlTimer;
    QString     _urlToOpen;
    bool        _quit;
};


// main modules
#define krApp        Krusader::App

#ifdef __KJSEMBED__
#define krJS   Krusader::App->js
#endif

#endif
