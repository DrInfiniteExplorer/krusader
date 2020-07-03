/*****************************************************************************
 * Copyright (C) 2005 Jonas Bähr <jonas.baehr@web.de>                        *
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

#include "krjs.h"

#include "../krglobal.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

#include <KWidgetsAddons/KMessageBox>

KrJS::KrJS() : KJSEmbed::KJSEmbedPart()
{

    // make this object, the object Krusader, available for scripting as "Krusader":
    addObject(krMainWindow, "Krusader");

    // make this object available for scripting
    addObject(ACTIVE_MNG, "PanelManager");
}

bool KrJS::runFile(const QString & filename)
{

    KJS::ExecState *exec = globalExec();

    // set up the variable scriptDir with the directory of the current script
    putValue("scriptDir", KJSEmbed::convertToValue(exec, QUrl(filename).directory(false)));

    bool ok = KJSEmbedPart::runFile(filename);

    KJS::Completion jsres = completion();

    // this is based on [this example-code](https://invent.kde.org/frameworks/kjsembed/-/blob/master/src/kjscmd/kjscmd.cpp)
    if (jsres.complType() != KJS::Normal) {
        switch (jsres.complType()) {
        case KJS::Break:
        case KJS::Continue:
            // TODO: find out what this means
            qWarning() << "JavaScript: " << jsres.value().toString(exec).qstring();
            break;
        case KJS::ReturnValue:
            // that's only needed when a specific function is called
            //someString = jsres.value().toString(exec).qstring();
            break;
        case KJS::Throw:
            /*
              This is the interesting part: If the JS throws an exception which is not handled inside the script,
              we can catch it here and handle it with c++
            */
        {
            KJS::Object exception = jsres.value().toObject(exec);
            int line = int(exception.get(exec, KJS::Identifier("line")).toNumber(exec));
            QString type = exception.get(exec, KJS::Identifier("name")).toString(exec).qstring();
            QString message = exception.get(exec, KJS::Identifier("message")).toString(exec).qstring();

            qWarning() << "JavaScript: Uncaught " << type << " exception at line " << line <<  " in " << filename;
            qWarning() << message;

            KMessageBox::error(0,   //parent
                               (line < 0 ?
                                QString(i18n("In %1:\nUncaught JavaScript exception '%2'\n%3", filename, type, message)) :
                                QString(i18n("In %1:\nUncaught JavaScript exception '%2' at line %3\n%4", filename, type, line, message))
                               ), //text
                               i18n("JavaScript Error"),  //caption
                               KMessageBox::Dangerous);
            break;
        }
        default:
            qWarning() << "JavaScript: Unknown error.";
            break;
        } // switch
    } // if


    qWarning() << "JS: done";

    return ok;
}

