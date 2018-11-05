/*****************************************************************************
 * Copyright (C) 2009 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2009-2018 Krusader Krew [https://krusader.org]              *
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

#include "krpreviewjob.h"

#include "krpreviews.h"
#include "PanelView/krview.h"
#include "PanelView/krviewitem.h"
#include "../FileSystem/fileitem.h"
#include "../defaults.h"

#include <stdio.h>

// QtWidgets
#include <QWidget>

#define ASSERT(what) if(!what) abort();

// how much items to process by a single job
// view becomes unresponsive during load if set too high
#define MAX_CHUNK_SIZE 50


KrPreviewJob::KrPreviewJob(KrPreviews *parent) : _job(0), _parent(parent)
{
    _timer.setSingleShot(true);
    _timer.setInterval(0);
    connect(&_timer, &QTimer::timeout, this, &KrPreviewJob::slotStartJob);
}

KrPreviewJob::~KrPreviewJob()
{
    doKill();
}

void KrPreviewJob::scheduleItem(KrViewItem *item)
{
    if(!_scheduled.contains(item)) {
        _scheduled.append(item);
        setTotalAmount(KJob::Files, totalAmount(KJob::Files) + 1);
    }
    if(!_job)
        _timer.start();
}

void KrPreviewJob::removeItem(KrViewItem *item)
{
    setTotalAmount(KJob::Files, totalAmount(KJob::Files) - _scheduled.removeAll(item));
    if(_job) {
        doKill();
        if(!_scheduled.isEmpty())
            _timer.start();
    }
    if(_scheduled.isEmpty())
        emitResult();
}

void KrPreviewJob::slotFailed(const KFileItem & item)
{
    slotGotPreview(item, QPixmap());
}

void KrPreviewJob::slotGotPreview(const KFileItem & item, const QPixmap & preview)
{
    KrViewItem *vi = _hash[item];
    ASSERT(vi);

    _scheduled.removeOne(vi);

    const FileItem *file = vi->getFileItem();
    _parent->addPreview(file, preview);
    vi->redraw();

    setProcessedAmount(KJob::Files, processedAmount(KJob::Files) + 1);
    emitPercent(processedAmount(KJob::Files), totalAmount(KJob::Files));
}

void KrPreviewJob::slotStartJob()
{
    ASSERT(_job == 0);
    ASSERT(!_scheduled.isEmpty());

    _hash.clear();
    sort();

    int size = _parent->_view->fileIconSize();

    KFileItemList list;
    for(int i = 0; i < _scheduled.count() && i < MAX_CHUNK_SIZE; i++) {
        KFileItem fi(_scheduled[i]->getFileItem()->getUrl(), 0, 0);
        list.append(fi);
        _hash.insert(fi, _scheduled[i]);
    }
    QStringList allPlugins = KIO::PreviewJob::availablePlugins();
    _job = new KIO::PreviewJob(list, QSize(size, size), &allPlugins);
    _job->setOverlayIconAlpha(0);
    _job->setOverlayIconSize(0);
    _job->setScaleType(KIO::PreviewJob::ScaledAndCached);
    connect(_job, &KIO::PreviewJob::gotPreview, this, &KrPreviewJob::slotGotPreview);
    connect(_job, &KIO::PreviewJob::failed, this, &KrPreviewJob::slotFailed);
    connect(_job, &KIO::PreviewJob::result, this, &KrPreviewJob::slotJobResult);
}

void KrPreviewJob::slotJobResult(KJob *job)
{
    (void) job;

    if(!disconnect(_job, 0, this, 0))
        abort();

    _job = 0;
    _hash.clear();

    if(_scheduled.isEmpty())
        emitResult();
    else
         _timer.start();
}

// move currently visible items to beginning of the list
void KrPreviewJob::sort()
{
    for(int i = 0, visible_end = 0; i < _scheduled.count(); i++) {
        KrViewItem *item = _scheduled[i];
        if(_parent->_view->widget()->rect().intersects(item->itemRect())) {
            if(i != visible_end)
                _scheduled.move(i, visible_end);
            visible_end++;
        }
    }
}

bool KrPreviewJob::doKill()
{
    _timer.stop();
    if(_job) {
        if(!disconnect(_job, 0, this, 0))
            abort();
        if(!_job->kill())
            abort();
        _job = 0;
    }
    _hash.clear();
    return true;
}
