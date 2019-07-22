/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
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

#include "kr7zencryptionchecker.h"

Kr7zEncryptionChecker::Kr7zEncryptionChecker() : KProcess(), encrypted(false), lastData()
{
    setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
    connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedOutput()));
}

void Kr7zEncryptionChecker::setupChildProcess()
{
    // This function is called after the fork but for the exec. We create a process group
    // to work around a broken wrapper script of 7z. Without this only the wrapper is killed.
    setsid(); // make this process leader of a new process group
}

void Kr7zEncryptionChecker::receivedOutput()
{
    QString data = QString::fromLocal8Bit(this->readAllStandardOutput());

    QString checkable = lastData + data;

    QStringList lines = checkable.split('\n');
    lastData = lines[ lines.count() - 1 ];
    for (int i = 0; i != lines.count(); i++) {
        QString line = lines[ i ].trimmed().toLower();
        int ndx = line.indexOf("testing");
        if (ndx >= 0)
            line.truncate(ndx);
        if (line.isEmpty())
            continue;

        if (line.contains("password") && line.contains("enter")) {
            encrypted = true;
            ::kill(- pid(), SIGKILL); // kill the whole process group by giving the negative PID
        }
    }
}

bool Kr7zEncryptionChecker::isEncrypted()
{
    return encrypted;
}
