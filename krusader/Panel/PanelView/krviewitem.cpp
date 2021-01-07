/*****************************************************************************
 * Copyright (C) 2000-2002 Shie Erlich <erlich@users.sourceforge.net>        *
 * Copyright (C) 2000-2002 Rafi Yanai <yanai@users.sourceforge.net>          *
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

#include "krviewitem.h"

#include "krinterview.h"
#include "listmodel.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/krpermhandler.h"

// QtCore
#include <QLocale>
#include <QMimeDatabase>
#include <QMimeType>
// QtGui
#include <QPixmap>

#include <KI18n/KLocalizedString>

KrViewItem::KrViewItem(FileItem *fileitem, KrInterView *parentView):
        _fileitem(fileitem), _view(parentView), _viewProperties(parentView->properties()), _hasExtension(false),
        _hidden(false), _extension("")
{
    dummyFileItem = parentView->_model->dummyFileItem() == fileitem;

    if (fileitem) {
        // check if the file has an extension
        const QString& fileitemName = fileitem->getName();
        int loc = fileitemName.lastIndexOf('.');
        if (loc > 0) { // avoid mishandling of .bashrc and friend
            // check if it has one of the predefined 'atomic extensions'
            for (const auto & atomicExtension : _viewProperties->atomicExtensions) {
                if (fileitemName.endsWith(atomicExtension)) {
                    loc = fileitemName.length() - atomicExtension.length();
                    break;
                }
            }
            _name = fileitemName.left(loc);
            _extension = fileitemName.mid(loc + 1);
            _hasExtension = true;
        }

        if (fileitemName.startsWith('.'))
            _hidden = true;
    }
}

const QString& KrViewItem::name(bool withExtension) const
{
    if (!withExtension && _hasExtension) return _name;
    else return _fileitem->getName();
}

QString KrViewItem::description() const
{
    if (dummyFileItem)
        return i18n("Climb up the folder tree");

    // else is implied

    QString mimeTypeComment;
    QMimeType mt = QMimeDatabase().mimeTypeForName(_fileitem->getMime());
    if (mt.isValid())
        mimeTypeComment = mt.comment();

    QString text = _fileitem->getName();
    if (_fileitem->isSymLink()) {
        text += " -> " + _fileitem->getSymDest() + "  ";
        if (_fileitem->isBrokenLink())
            text += i18n("(Broken Link)");
        else if (mimeTypeComment.isEmpty())
            text += i18n("Symbolic Link");
        else
            text += i18n("%1 (Link)", mimeTypeComment);

    } else {
        if (_fileitem->isDir())
            text += '/';

        if (_fileitem->getUISize() != (KIO::filesize_t)-1) {
            const QString size = KrView::sizeText(_viewProperties, _fileitem->getUISize());
            text += QString("  (%1)").arg(size);
        }

        text += "  " + mimeTypeComment;
    }
    return text;
}

QPixmap KrViewItem::icon()
{
    if (dummyFileItem || !_viewProperties->displayIcons)
        return QPixmap();
    else return KrView::getIcon(_fileitem, true);
}

bool KrViewItem::isSelected() const {
    return _view->isSelected(_fileitem);
}

void KrViewItem::setSelected(bool s) {
    _view->setSelected(_fileitem, s);
    if(!_view->op()->isMassSelectionUpdate()) {
        redraw();
        _view->op()->emitSelectionChanged();
    }
}

QRect KrViewItem::itemRect() const {
    return _view->itemRect(_fileitem);
}

void KrViewItem::redraw() {
    _view->_itemView->viewport()->update(itemRect());
}

void KrViewItem::setSize(KIO::filesize_t size)
{
    _fileitem->setSize(size);
}
