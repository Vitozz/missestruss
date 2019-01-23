/*
 * mainwindow.h
 * Copyright (C) 2015-2019 Vitaly Tonkacheyev
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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QVector>
#include <QPoint>
#include <QTableWidget>
#include <qwt_plot_grid.h>
#include <QSharedPointer>
#include "../core/misescalcnelast.h"
#include "../core/misescalcelast.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    //DoubleSpinbox slots
    void startAnlgeChanged(const double &value);
    void youngModuleChanged(const double &value);
    void trussLengthChanged(const double &value);
    void csAreaChnged(const double &value);
    void forceAngleChanged(const double &value);
    void supportStfnsChanged(const double &value);
    void iterationChanged(const double &value);
    void hwChanged(const double &value);
    void AfcalChanged(const double &value);
    //Buttons
    void onCountBtn();
    void onCountExtremums();
    void onSaveBtn();
    void calculateKv();
    void onPlus();
    void onSubst();
    //Table1
    void onTableSelectionChanged();
    void onPopup(const QPoint& point);
    void onCopy(bool);
    //TabWidget
    void onTabChanged(int index);
    //Menu
    void onExit();
    void onAbout();

protected:
    void closeEvent(QCloseEvent *e);

private:
    void calculate();
    void calculateExtremums();
    void checkVars();
    void keyPressEvent(QKeyEvent * event);
    void copyToClipboard();
    void saveSelection();
    void saveSettings();
    void showDoneDialog(const QString &text);
    bool fZeroCheck(const double &a) const { return (qAbs(a-ZERO) < EPSILON_);}
    double TextToDouble(const QString &text) const;
    QString DoubleToText(const double &value, const int &size) const;
    void enableButtons(bool enabled);

private:
    Ui::MainWindow *ui;
    MisesCalcNonElast::Ptr misesNE_;
    MisesCalcElast::Ptr misesE_;
    QSharedPointer<QwtPlotGrid> grid_;
    QString buffer_;
    QString lastDir_;
    QString log_;
    QMenu *popupMenu_;
    QAction *copyAction_;
};

#endif // MAINWINDOW_H
