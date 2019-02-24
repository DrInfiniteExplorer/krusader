/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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



#include "krspecialwidgets.h"
#include "krmaskchoice.h"
#include "newftpgui.h"
#include "../krglobal.h"

// QtGui
#include <QPaintEvent>

#include <KI18n/KLocalizedString>
#include <utility>

/////////////////////////////////////////////////////////////////////////////
/////////////////////// Pie related widgets /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// The pie-related widgets use hard-coded coordinates to create the look.
// This is ok since the whole widget is fitted into an existing view and thus
// no re-alignments are needed.

#define LEFT       10
#define BOTTOM     150
#define WIDTH      120
#define HEIGHT     40
#define Z_HEIGHT   10
#define STARTANGLE 0
#define DEG(x)     (16*(x))

QColor KRPie::colors[ 12 ] = {Qt::red, Qt::blue, Qt::green, Qt::cyan, Qt::magenta, Qt::gray,
                              Qt::black, Qt::white, Qt::darkRed, Qt::darkBlue, Qt::darkMagenta,
                              Qt::darkCyan
                             };

//////////////////////////////////////////////////////////////////////////////
/////////////// KRFSDisplay - Filesystem / Freespace Display /////////////////
//////////////////////////////////////////////////////////////////////////////
// This is the full constructor: use it for a mounted filesystem
KRFSDisplay::KRFSDisplay(QWidget *parent, QString _alias, QString _realName,
                         KIO::filesize_t _total, KIO::filesize_t _free) : QWidget(parent), totalSpace(_total),
        freeSpace(_free), alias(std::move(_alias)), realName(std::move(_realName)), mounted(true),
        empty(false), supermount(false)
{

    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    resize(150 + leftMargin + rightMargin, 200 + topMargin + bottomMargin);
    setMinimumSize(150 + leftMargin + rightMargin, 200 + topMargin + bottomMargin);
    show();
}

// Use this one for an unmounted filesystem
KRFSDisplay::KRFSDisplay(QWidget *parent, QString _alias, QString _realName, bool sm) :
        QWidget(parent), alias(std::move(_alias)), realName(std::move(_realName)), mounted(false),
        empty(false), supermount(sm)
{

    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    resize(150 + leftMargin + rightMargin, 200 + topMargin + bottomMargin);
    setMinimumSize(150 + leftMargin + rightMargin, 200 + topMargin + bottomMargin);
    show();
}

// This is used only when an empty widget needs to be displayed (for example:
// when filesystem statistics haven't been calculated yet)
KRFSDisplay::KRFSDisplay(QWidget *parent) : QWidget(parent), empty(true)
{

    int leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
    resize(150 + leftMargin + rightMargin, 200 + topMargin + bottomMargin);
    setMinimumSize(150 + leftMargin + rightMargin, 200 + topMargin + bottomMargin);
    show();
}


// The main painter!
void KRFSDisplay::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    if (!empty) {
        int leftMargin, topMargin, rightMargin, bottomMargin;
        getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);
        // create the text
        // first, name and location
        QFont font = paint.font();
        font.setWeight(QFont::Bold);
        paint.setFont(font);
        paint.drawText(leftMargin + 10, topMargin + 20, alias);

        font.setWeight(QFont::Normal);
        paint.setFont(font);
        paint.drawText(leftMargin + 10, topMargin + 37, '(' + realName + ')');

        if (mounted) {    // incase the filesystem is already mounted
            // second, the capacity
            paint.drawText(leftMargin + 10, topMargin + 70, i18n("Capacity: %1", KIO::convertSizeFromKiB(totalSpace)));
            // third, the 2 boxes (used, free)
            QPen systemPen = paint.pen();
            paint.setPen(Qt::black);
            paint.drawRect(leftMargin + 10, topMargin + 90, 10, 10);
            paint.fillRect(leftMargin + 11, topMargin + 91, 8, 8, QBrush(Qt::gray));
            paint.drawRect(leftMargin + 10, topMargin + 110, 10, 10);
            paint.fillRect(leftMargin + 11, topMargin + 111, 8, 8, QBrush(Qt::white));
            // now, the text for the boxes
            paint.setPen(systemPen);
            paint.drawText(leftMargin + 25, topMargin + 100, i18n("Used: %1", KIO::convertSizeFromKiB(totalSpace - freeSpace)));
            paint.drawText(leftMargin + 25, topMargin + 120, i18n("Free: %1", KIO::convertSizeFromKiB(freeSpace)));
            // first, create the empty pie
            // bottom...
            paint.setPen(Qt::black);
            paint.setBrush(Qt::white);
            paint.drawPie(leftMargin + LEFT, topMargin + BOTTOM, WIDTH, HEIGHT, STARTANGLE, DEG(360));
            // body...
            paint.setPen(Qt::lightGray);
            for (int i = 1; i < Z_HEIGHT; ++i)
                paint.drawPie(leftMargin + LEFT, topMargin + BOTTOM - i, WIDTH, HEIGHT, STARTANGLE, DEG(360));
            // side lines...
            paint.setPen(Qt::black);
            paint.drawLine(leftMargin + LEFT, topMargin + BOTTOM + HEIGHT / 2, LEFT, BOTTOM + HEIGHT / 2 - Z_HEIGHT);
            paint.drawLine(leftMargin + LEFT + WIDTH, topMargin + BOTTOM + HEIGHT / 2, LEFT + WIDTH, BOTTOM + HEIGHT / 2 - Z_HEIGHT);
            // top of the pie
            paint.drawPie(leftMargin + LEFT, topMargin + BOTTOM - Z_HEIGHT, WIDTH, HEIGHT, STARTANGLE, DEG(360));
            // the "used space" slice
            float i = ((float)(totalSpace - freeSpace) / (totalSpace)) * 360.0;
            paint.setBrush(Qt::gray);
            paint.drawPie(leftMargin + LEFT, topMargin + BOTTOM - Z_HEIGHT, WIDTH, HEIGHT, STARTANGLE, (int) DEG(i));
            // if we need to draw a 3d stripe ...
            if (i > 180.0) {
                for (int j = 1; j < Z_HEIGHT; ++j)
                    paint.drawArc(leftMargin + LEFT, topMargin + BOTTOM - j, WIDTH, HEIGHT, STARTANGLE - 16 * 180, (int)(DEG(i - 180.0)));
            }
        } else {  // if the filesystem is unmounted...
            font.setWeight(QFont::Bold);
            paint.setFont(font);
            paint.drawText(leftMargin + 10, topMargin + 60, i18n("Not mounted."));
        }
    } else {  // if the widget is in empty situation...

    }
}

////////////////////////////////////////////////////////////////////////////////
KRPie::KRPie(KIO::filesize_t _totalSize, QWidget *parent) : QWidget(parent), totalSize(_totalSize)
{
    slices.push_back(KRPieSlice(100, Qt::yellow, "DEFAULT"));
    sizeLeft = totalSize;
    resize(300, 300);
}

void KRPie::paintEvent(QPaintEvent *)
{
    QPainter paint(this);
    // now create the slices
    float sAngle = STARTANGLE;
    for (int ndx = 0; ndx != slices.count(); ndx++) {
        paint.setBrush(slices[ndx].getColor());
        paint.setPen(slices[ndx].getColor());
        // angles are negative to create a clock-wise drawing of slices
        float angle = -(slices[ndx].getPerct() / 100 * 360) * 16;
        for (int i = 1; i < Z_HEIGHT; ++i)
            paint.drawPie(LEFT, BOTTOM + i, WIDTH, HEIGHT, (int) sAngle, (int) angle);
        sAngle += angle;
    }
    paint.setPen(Qt::yellow);     // pen
    paint.setBrush(Qt::yellow);   // fill
    // for (int i=1; i<Z_HEIGHT; ++i)
    //  paint.drawPie(LEFT,BOTTOM+i,WIDTH,HEIGHT,sAngle,360*16-(STARTANGLE-sAngle));
    sAngle = STARTANGLE;
    for (int ndx = 0; ndx != slices.count(); ndx++) {
        paint.setBrush(slices[ndx].getColor());
        paint.setPen(slices[ndx].getColor());
        // angles are negative to create a clock-wise drawing of slices
        float angle = -(slices[ndx].getPerct() / 100 * 360) * 16;
        paint.drawPie(LEFT, BOTTOM, WIDTH, HEIGHT, (int) sAngle, (int) angle);
        sAngle += angle;
    }


    paint.setPen(Qt::black);
    // the pie
    //  paint.drawPie(LEFT,BOTTOM,WIDTH,HEIGHT,STARTANGLE,360*16);
    ///////////////////////// end of empty pie /////////////////////////
    // now, the pie is ready to draw slices on...
    // to make a good look on the perimeter, draw another black circle
    paint.setPen(Qt::black);
    paint.drawArc(LEFT, BOTTOM, WIDTH, HEIGHT, STARTANGLE, 360 * 16);

}

void KRPie::addSlice(KIO::filesize_t size, QString label)
{
    int i = (slices.count() % 12);
    slices.removeLast();
    slices.push_back(KRPieSlice(size * 100 / totalSize, colors[ i ], std::move(label)));
    sizeLeft -= size;
    slices.push_back(KRPieSlice(sizeLeft * 100 / totalSize, Qt::yellow, "DEFAULT"));
}
