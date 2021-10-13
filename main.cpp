/*
 * main.cpp
 * Copyright (C) 2015-2021 Vitaly Tonkacheyev
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "gui/mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QDir>
#include "defines.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName(QString(ORGANIZATION));
    QApplication::setApplicationName(QString(APPNAME));
    QApplication::setApplicationVersion(QString(APPVERISON));
    QTranslator translator;
    const QStringList localeDirs({qApp->applicationDirPath(),
                                  QString("%1/langs").arg(QDir::currentPath()),
                                  QString("%1/langs").arg(qApp->applicationDirPath()),
                                  QString("../share/%1/langs").arg(APPNAME),
                                  QString("/usr/share/%1/langs").arg(QString(APPNAME)),
                                  QString("/usr/local/share/%1/langs").arg(QString(APPNAME)),
                                  QString("%1/.local/share/%2/langs").arg(QDir::home().absolutePath(),QString(APPNAME))});
    const QString langFile(APPNAME);
    for (auto& dir : localeDirs) {
        if (translator.load(QLocale::system(),langFile, "_", dir )) {
            qApp->installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();

    return QApplication::exec();
}
