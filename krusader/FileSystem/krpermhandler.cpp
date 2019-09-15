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

#include "krpermhandler.h"

// QtCore
#include <QLocale>

#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>

QSet<int> KRpermHandler::currentGroups;
QHash<int, QString> KRpermHandler::uidCache;
QHash<int, QString> KRpermHandler::gidCache;

QString KRpermHandler::mode2QString(mode_t m)
{
    char perm[ 11 ];
    for (int i = 0; i != 10; i++)
        perm[ i ] = '-';
    perm[ 10 ] = 0;

    if (S_ISLNK(m)) perm[ 0 ] = 'l';      // check for symLink
    if (S_ISDIR(m)) perm[ 0 ] = 'd';      // check for directory

    //ReadUser = 0400, WriteUser = 0200, ExeUser = 0100, Suid = 04000
    if (m & 0400) perm[ 1 ] = 'r';
    if (m & 0200) perm[ 2 ] = 'w';
    if (m & 0100) perm[ 3 ] = 'x';
    if (m & 04000) perm[ 3 ] = 's';
    //ReadGroup = 0040, WriteGroup = 0020, ExeGroup = 0010, Gid = 02000
    if (m & 0040) perm[ 4 ] = 'r';
    if (m & 0020) perm[ 5 ] = 'w';
    if (m & 0010) perm[ 6 ] = 'x';
    if (m & 02000) perm[ 6 ] = 's';
    //ReadOther = 0004, WriteOther = 0002, ExeOther = 0001, Sticky = 01000
    if (m & 0004) perm[ 7 ] = 'r';
    if (m & 0002) perm[ 8 ] = 'w';
    if (m & 0001) perm[ 9 ] = 'x';
    if (m & 01000) perm[ 9 ] = 't';

    return QString(perm);
}

void KRpermHandler::init()
{
    // set the umask to 022
    //umask( 022 );

    // 200 groups should be enough
    gid_t groupList[200];
    int groupNo = getgroups(200, groupList);

// In kdewin32 implementation as of 4.1.2, getpwent always returns the same struct
#ifndef Q_OS_WIN
    // fill the UID cache
    struct passwd *pass;
    while ((pass = getpwent()) != nullptr) {
        uidCache.insert(pass->pw_uid, pass->pw_name);
    }
    delete pass;
    endpwent();

    // fill the GID cache
    struct group *gr;
    while ((gr = getgrent()) != nullptr) {
        gidCache.insert(gr->gr_gid, QString(gr->gr_name));
    }
    delete gr;
    endgrent();
#endif

    // fill the groups for the current user
    for (int i = 0; i < groupNo; ++i) {
        currentGroups.insert(groupList[i]);
    }
    // just to be sure add the effective gid...
    currentGroups.insert(getegid());
}

char KRpermHandler::readable(const QString &perm, gid_t gid, uid_t uid)
{
    return getLocalPermission(perm, gid, uid, 0);
}

char KRpermHandler::writeable(const QString &perm, gid_t gid, uid_t uid)
{
    return getLocalPermission(perm, gid, uid, 1);
}

char KRpermHandler::executable(const QString &perm, gid_t gid, uid_t uid)
{
    return getLocalPermission(perm, gid, uid, 2, true);
}

char KRpermHandler::getLocalPermission(const QString &perm, gid_t gid, uid_t uid, int permOffset,
                                       bool ignoreRoot)
{
    // root override
    if (!ignoreRoot && getuid() == 0)
        return ALLOWED_PERM;
    // first check other permissions.
    if (perm[7 + permOffset] != '-')
        return ALLOWED_PERM;
    // now check group permission
    if ((perm[4 + permOffset] != '-') && currentGroups.contains(gid))
        return ALLOWED_PERM;
    // the last chance - user permissions
    if ((perm[1 + permOffset] != '-') && (uid == getuid()))
        return ALLOWED_PERM;
    // sorry !
    return NO_PERM;
}

char KRpermHandler::ftpReadable(const QString &fileOwner, const QString &userName, const QString &perm)
{
    return getFtpPermission(fileOwner, userName, perm, 0);
}

char KRpermHandler::ftpWriteable(const QString &fileOwner, const QString &userName, const QString &perm)
{
    return getFtpPermission(fileOwner, userName, perm, 1);
}

char KRpermHandler::ftpExecutable(const QString &fileOwner, const QString &userName, const QString &perm)
{
    return getFtpPermission(fileOwner, userName, perm, 2);
}

char KRpermHandler::getFtpPermission(const QString &fileOwner, const QString &userName,
                                     const QString &perm, int permOffset)
{
    // first check other permissions.
    if (perm[7 + permOffset] != '-')
        return ALLOWED_PERM;
    // can't check group permission !
    // so check the user permissions
    if ((perm[1 + permOffset] != '-') && (fileOwner == userName))
        return ALLOWED_PERM;
    if ((perm[1 + permOffset] != '-') && (userName.isEmpty()))
        return UNKNOWN_PERM;
    if (perm[4 + permOffset] != '-')
        return UNKNOWN_PERM;
    return NO_PERM;
}

QString KRpermHandler::parseSize(KIO::filesize_t val)
{
    return QLocale().toString(val);
}

QString KRpermHandler::gid2group(gid_t groupId)
{
   return gidCache.value(groupId, QStringLiteral("???"));
}

QString KRpermHandler::uid2user(uid_t userId)
{
    return uidCache.value(userId, QStringLiteral("???"));
}
