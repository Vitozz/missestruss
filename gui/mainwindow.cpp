/*
 * mainwindow.cpp
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
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPoint>
#include <QPixmap>
#include <QPainter>
#include <QMimeData>
#include <QClipboard>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QFile>
#include <QIODevice>
#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QDoubleSpinBox>
#include <QRegExp>
#ifdef IS_DEBUG
#include <QDebug>
#endif
#include "defines.h"
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_series_data.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    misesNE_(MisesCalcNonElast::Ptr(new MisesCalcNonElast())),
    misesE_(MisesCalcElast::Ptr(new MisesCalcElast())),
    grid_(new QwtPlotGrid()),
    buffer_(QString()),
    lastDir_(QDir::home().absolutePath()),
    log_(QString()),
    popupMenu_(new QMenu(this)),
    copyAction_(new QAction(tr("&Copy Data"), popupMenu_))
{
    ui->setupUi(this);
    QSettings settings(this);
    ui->iterations->setValue(settings.value(NumIter, 1.0).toDouble());
    ui->alpha_0->setValue(settings.value(NumStartAngle, 45.0).toDouble());
    ui->arcLength->setValue(settings.value(NumTrussLength, 1.0).toDouble());
    ui->cross_area->setValue(settings.value(NumCrossArea, 1.0).toDouble());
    ui->forceAngle->setValue(settings.value(NumForceAngle, ZERO).toDouble());
    ui->YoungModule->setValue(settings.value(NumYoungModule, ZERO).toDouble());
    ui->kv->setValue(settings.value(NumSupportStiffness, ZERO).toDouble());
    ui->plotScale->setValue(settings.value(NumPlotScale, ZERO).toDouble());
    ui->plotScale2->setValue(settings.value(NumPlotScaleElast, ZERO).toDouble());
    ui->hw->setValue(settings.value(NumHw, ZERO).toDouble());
    ui->afcal->setValue(settings.value(NumAfCal, ZERO).toDouble());
    ui->isDeriv->setChecked(settings.value(BoolDeriv, false).toBool());
    popupMenu_->addAction(copyAction_);
    ui->miscalc->setValue(EPSILON_);
    ui->tabWidget->setCurrentIndex(0);
    //connecting QDoubleSpinboxes
    connect(ui->alpha_0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::startAnlgeChanged);
    connect(ui->arcLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::trussLengthChanged);
    connect(ui->cross_area, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::csAreaChnged);
    connect(ui->forceAngle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::forceAngleChanged);
    connect(ui->kv, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::supportStfnsChanged);
    connect(ui->YoungModule, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MainWindow::youngModuleChanged);
    connect(ui->iterations,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &MainWindow::iterationChanged);
    connect(ui->plotScale,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [this](auto value) { misesNE_->setScale(value); });
    connect(ui->plotScale2,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            [this](auto value) { misesE_->setScale(value); });
    //connecting QPushButtons
    connect(ui->countBtn, &QPushButton::pressed, this, &MainWindow::onCountBtn);
    connect(ui->calcExtr, &QPushButton::pressed, this, &MainWindow::onCountExtremums);
    connect(ui->saveBtn, &QPushButton::pressed, this, &MainWindow::onSaveBtn);
    connect(ui->calcKvBtn, &QPushButton::pressed, this, &MainWindow::calculateKv);
    //connecting QTableWidgets
    connect(ui->tableWidget, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &MainWindow::onPopup);
    connect(ui->results_Bilyk, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(ui->results_Bilyk, &QTableWidget::customContextMenuRequested, this, &MainWindow::onPopup);
    connect(ui->extremums, &QTableWidget::itemSelectionChanged, this, &MainWindow::onTableSelectionChanged);
    connect(ui->extremums, &QTableWidget::customContextMenuRequested, this, &MainWindow::onPopup);
    //connecting QAction
    connect(copyAction_, &QAction::triggered, this, &MainWindow::onCopy);
    //connecting QTabWidget
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    //QMenu
    connect(ui->actionE_xit, &QAction::triggered, this, &MainWindow::onExit);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onAbout);
    connect(ui->actionA_bout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);

    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, tr("Relative force, P[rel]"));
    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, tr("Relative vertical displacement, v[rel]"));
    grid_->attach(ui->qwtPlot);
    auto legend(new QwtLegend(ui->qwtPlot));
    ui->qwtPlot->insertLegend(legend, QwtPlot::TopLegend, 0.33);
    ui->saveBtn->setEnabled(false);
    ui->calcKvBtn->setEnabled(false);
    enableButtons(false);
    checkVars();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete copyAction_;
    delete popupMenu_;
    delete ui;
}

void MainWindow::startAnlgeChanged(const double &value)
{
    misesNE_->setStartAnlge(value);
    misesE_->setStartAnlge(value);
}

void MainWindow::trussLengthChanged(const double &value)
{
    misesNE_->setTrussLength(value);
    misesE_->setTrussLength(value);
}

void MainWindow::csAreaChnged(const double &value)
{
    misesNE_->setCsArea(value);
    misesE_->setCsArea(value);
}

void MainWindow::forceAngleChanged(const double &value)
{
    misesNE_->setForceAngle(value);
}

void MainWindow::supportStfnsChanged(const double &value)
{
    misesNE_->setSupportStfns(value);
}

void MainWindow::youngModuleChanged(const double &value)
{
    misesNE_->setYoungModule(value);
    misesE_->setYoungModule(value);
}

void MainWindow::iterationChanged(const double &value)
{
    misesE_->setIterations(value);
    misesNE_->setIterations(value);
    misesNE_->countScale(ui->alpha_0->value());
    misesE_->countScale(ui->alpha_0->value());
    ui->plotScale->setValue(misesNE_->scale());
    ui->plotScale2->setValue(misesE_->scale());
}

void MainWindow::onTabChanged(int index)
{
    switch (index) {
    case 4:
        ui->saveBtn->setEnabled((ui->extremums->rowCount() >= 1));
        break;
    default:
        break;
    }
}

void MainWindow::onCountBtn()
{
    calculate();
}

void MainWindow::onCountExtremums()
{
    calculateExtremums();
}

void MainWindow::onSaveBtn()
{
    saveSelection();
}

void MainWindow::onExit()
{
    qApp->quit();
}

void MainWindow::onAbout()
{
    const QString message(tr("<!DOCTYPE html><html><body>"
                             "<p><b>Calculation of von Mises truss parameters</b></p>"
                             "<p>Written using Qt</p>"
                             "<p>2015-2019 (c) Vitaly Tonkacheyev <address><a href=\"mailto:thetvg@gmail.com\">&lt;EMail&gt;</a></address></p>"
                             "<a href=\"http://sites.google.com/site/thesomeprojects/\">Program WebSite</a>"
                             "<p>version: <b>%1</b></p></body></html>").arg(APPVERISON));
    const QString appLogo = ":/icons/f(x)48.png";
    QMessageBox about;
    about.setIconPixmap(QPixmap(appLogo));
    about.setWindowIcon(this->windowIcon());
    about.setText(message);
    about.setWindowTitle(tr("About program"));
    about.exec();
}

void MainWindow::checkVars()
{
    misesNE_->setStartAnlge(ui->alpha_0->value());
    misesNE_->setYoungModule(ui->YoungModule->value());
    misesNE_->setTrussLength(ui->arcLength->value());
    misesNE_->setCsArea(ui->cross_area->value());
    misesNE_->setForceAngle(ui->forceAngle->value());
    misesNE_->setSupportStfns(ui->kv->value());
    misesNE_->setIterations(ui->iterations->value());
    misesNE_->setScale(ui->plotScale->value());
    misesE_->setStartAnlge(ui->alpha_0->value());
    misesE_->setYoungModule(ui->YoungModule->value());
    misesE_->setTrussLength(ui->arcLength->value());
    misesE_->setCsArea(ui->cross_area->value());
    misesE_->setHwM(ui->hw->value());
    misesE_->setAfcal(ui->afcal->value());
    misesE_->setIterations(ui->iterations->value());
    misesE_->setScale(ui->plotScale2->value());
}

void MainWindow::calculate()
{
    enableButtons(false);
    log_.clear();
    ui->extremumX->setText(QString());
    ui->extremumY->setText(QString());
    ui->extremumX2->setText(QString());
    ui->extremumY2->setText(QString());
    ui->tableWidget->clearContents();
    ui->results_Bilyk->clearContents();
    ui->qwtPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true);
    checkVars();
    misesNE_->doCalculate();
    misesE_->doCalculate();
    const QVector<QPointF> curveFx_ = misesNE_->getAllForces();
    const QVector<QPointF> curvedFx_ = misesNE_->getAllDerivatives();
    const QVector<QPointF> elastCurve = misesE_->getAllForces();
    const QVector<QPointF> kems = misesE_->getAllKeM();
    //
    ui->tableWidget->setRowCount(curveFx_.count());
    ui->results_Bilyk->setRowCount(elastCurve.count());
    ui->progressBar->setRange(0, curveFx_.count());
    foreach (const QPointF& point, curveFx_) {
        int index = curveFx_.indexOf(point);
        auto item1 = new QTableWidgetItem(DoubleToText(point.x(), 12));
        auto item2 = new QTableWidgetItem(DoubleToText(point.y(), 12));
        ui->tableWidget->setItem(index, 0, item1);
        ui->tableWidget->setItem(index, 1, item2);
        auto item3 = new QTableWidgetItem(DoubleToText(curvedFx_.at(index).y(), 12));
        ui->tableWidget->setItem(index, 2, item3);
        if(index < elastCurve.count()) {
            const QPointF elastPoint = elastCurve.at(index);
            auto item4 = new QTableWidgetItem(DoubleToText(elastPoint.x(), 12));
            auto item5 = new QTableWidgetItem(DoubleToText(elastPoint.y(), 12));
            ui->results_Bilyk->setItem(index, 0, item4);
            ui->results_Bilyk->setItem(index, 1, item5);
            auto item6 = new QTableWidgetItem(DoubleToText(kems.at(index).y(), 12));
            ui->results_Bilyk->setItem(index, 2, item6);
        }
        ui->progressBar->setValue(index+1);
    }
    log_ = tr("Number of items in curve: %1\n").arg(QString::number(curveFx_.count()))
            +tr("Number of iderivatives: %1\n").arg(QString::number(curvedFx_.count()))
            +tr("Number of KeM: %1\n").arg(QString::number(kems.count()))
            +tr("Number of item in elastic calculation: %1\n").arg(QString::number(elastCurve.count()))
            +tr("GCS Plot Scale: %1\n").arg(QString::number(misesNE_->scale()))
            +tr("S. Bilyk Plot Scale: %1\n").arg(QString::number(misesE_->scale()));

    ui->cLog->setText(log_);
    ui->tableWidget->setColumnWidth(0, 150);
    ui->tableWidget->setColumnWidth(1, 150);
    ui->results_Bilyk->setColumnWidth(0,150);
    ui->results_Bilyk->setColumnWidth(1, 150);
    ui->results_Bilyk->setColumnWidth(2, 150);
    //
    auto curve(new QwtPlotCurve(tr("Function GCS, P[rel]")));
    auto myData(new QwtPointSeriesData);
    myData->setSamples(curveFx_);
    curve->setData(myData);
    curve->attach(ui->qwtPlot);
    if(ui->isDeriv->checkState() == Qt::Checked) {
        auto curve2(new QwtPlotCurve(tr("Derivative of function, P'[rel]")));
        auto myData2(new QwtPointSeriesData);
        myData2->setSamples(curvedFx_);
        curve2->setData(myData2);
        curve2->setPen(QPen(QColor(Qt::red)));
        curve2->attach(ui->qwtPlot);
    }
    ui->qwtPlot->replot();
    //
    auto curve3(new QwtPlotCurve(tr("Function S. Bilyk, P[rel]")));
    auto myData3(new QwtPointSeriesData);
    myData3->setSamples(elastCurve);
    curve3->setData(myData3);
    curve3->setPen(QPen(QColor(Qt::blue)));
    curve3->attach(ui->qwtPlot);
    ui->qwtPlot->replot();
    //
    const QPointF extremum = misesNE_->extremum();
    const QPointF extremum2 = misesE_->extremum();
    const QString extremumV1 = DoubleToText(extremum.x(), 12);
    const QString extremumP1 = DoubleToText(extremum.y(), 12);
    const QString extremumV2 = fZeroCheck(extremum2.x()) ? extremumV1 : DoubleToText(extremum2.x(), 12);
    const QString extremumP2 = fZeroCheck(extremum2.y()) ? extremumP1 : DoubleToText(extremum2.y(), 12);
    if (!extremumP1.isNull() && !extremumP2.isNull()) {
        ui->extremumX->setText(extremumV1);
        ui->extremumY->setText(extremumP1);
        ui->extremumX2->setText(extremumV2);
        ui->extremumY2->setText(extremumP2);
    }
    ui->centralWidget->adjustSize();
    saveSettings();
    ui->calcKvBtn->setEnabled(true);
    showDoneDialog(tr("Calculation successfully finished!"));
}

void MainWindow::showDoneDialog(const QString &text)
{
    QMessageBox::information(this, tr("Calculation information"), text);
}

void MainWindow::calculateExtremums()
{
    checkVars();
    double initialAngle = ui->initStAngle->value();
    const double forceAngle = ui->forceAngle->value();
    initialAngle = (initialAngle > forceAngle) ? initialAngle : forceAngle + 1.0;
    const double finalAngle = ui->finStAngle->value();
    ui->extremums->setRowCount(static_cast<int>(finalAngle - initialAngle + 1.0));
    int index = 0;
    const QVector<QPointF> extremums = misesNE_->getExtremums(initialAngle, finalAngle);
    const QVector<QString> angles = misesNE_->getAngles();
    foreach (const QPointF &point, extremums) {
        const QString& angle = angles.at(index);
        auto item1 = new QTableWidgetItem(angle);
        auto item2 = new QTableWidgetItem(DoubleToText(point.x(), 12));
        auto item3 = new QTableWidgetItem(DoubleToText(point.y(), 12));
        ui->extremums->setItem(index, 0, item1);
        ui->extremums->setItem(index, 1, item2);
        ui->extremums->setItem(index, 2, item3);
        ++index;
    }
    ui->extremums->setColumnWidth(1, 120);
    ui->centralWidget->adjustSize();
    ui->saveBtn->setEnabled(true);
    showDoneDialog(tr("Calculation successfully finished!"));
}

double MainWindow::TextToDouble(const QString &text) const
{
    const QLocale loca(QLocale::C);
    const QChar point = loca.decimalPoint();
    QRegExp space("\\s");
    QString result = text;
    result.replace(QLocale::system().decimalPoint(), point);
    result.replace(space, QString());
    bool ok;
    return result.toDouble(&ok);
}

QString MainWindow::DoubleToText(const double &value, const int &size) const
{
    QRegExp space("\\s");
    return QLocale::system().toString(value, 'g', size).replace(space, QString());
}

void MainWindow::calculateKv()
{
    if(ui->extremumY->text().isEmpty() || ui->extremumY2->text().isEmpty()) {
        return;
    }

    const double extremumPrelNE = TextToDouble(ui->extremumY->text());
    const double extremumVrelE = TextToDouble(ui->extremumX2->text());
    const double extremunPrelE = TextToDouble(ui->extremumY2->text());
    const double _EA = ui->cross_area->value()*ui->YoungModule->value()*1e6;
    const double a0 = ui->arcLength->value() / 2.0;
    const double result = qAbs(extremunPrelE - extremumPrelNE)/(extremumVrelE*a0)*_EA;
    log_ += tr("Extremum P[rel] by GCS: %1\n").arg(DoubleToText(extremumPrelNE, 12));
    log_ += tr("Extremum V[rel] by elastic method: %1\n").arg(DoubleToText(extremumVrelE, 12));
    log_ += tr("Extremum P[rel] by elastic method: %1\n").arg(DoubleToText(extremunPrelE, 12));
    log_ += tr("Stiffness of a rod, N: %1\n").arg(DoubleToText(_EA, 12));
    log_ += tr("Half span of a truss, m: %1\n").arg(DoubleToText(a0, 12));
    ui->cLog->setText(log_);
    ui->cKv->setText(DoubleToText(result, 12));
    enableButtons(true);
}

void MainWindow::onTableSelectionChanged()
{
    auto *widget = qobject_cast<QTableWidget*>(sender());
    QModelIndexList list = widget->selectionModel()->selectedIndexes();
    buffer_.clear();
    std::sort(list.begin(), list.end());
    QString text;
    int currRow = 0;
    foreach(const QModelIndex& cell, list)
    {
        if (text.length() > 0 && cell.row() != currRow ) {
            text+= "\n";
        }
        else if (text.length() > 0) {
            text+= "\t";
        }
        currRow = cell.row();
        text += cell.data().toString();
    }
    buffer_ = text;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_C && event->modifiers() & Qt::ControlModifier) {
        copyToClipboard();
    }
}

void MainWindow::onCopy(bool)
{
    copyToClipboard();
}

void MainWindow::copyToClipboard()
{
    if (!buffer_.isEmpty()) {
        auto mimeData = new QMimeData();
        mimeData->setData("text/plain",buffer_.toLocal8Bit());
        QApplication::clipboard()->setMimeData(mimeData);
    }
}

void MainWindow::saveSelection()
{
    QStringList lines({QString("%1\t%2\t%3\n").arg("alpha_0")
                       .arg(ui->extremums->horizontalHeaderItem(1)->text())
                       .arg(ui->extremums->horizontalHeaderItem(2)->text())});
    for (int row =0; row < ui->extremums->rowCount(); ++row) {
        QStringList line;
        for (int column = 0; column < ui->extremums->columnCount(); ++column) {
            line << ui->extremums->item(row, column)->text();
        }
        lines << QString("%1\n").arg(line.join("\t"));
    }
    QString result = lines.join("");
    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save file"),
                                                    lastDir_,
                                                    tr("CSV Table (*.csv)"),
                                                    nullptr,
                                                    nullptr);
    if (!filename.isEmpty()) {
        int dotIndex = filename.lastIndexOf(".");
        if (dotIndex < 0) {
            filename += ".csv";
        }
        QFile file( filename );
        QDir dir(lastDir_);
        lastDir_ = dir.absoluteFilePath(filename);
        if ( file.open(QIODevice::WriteOnly) )
        {
            QTextStream stream( &file );
            stream << result << endl;
            file.close();
        }
    }

}

void MainWindow::onPopup(const QPoint &point)
{
    auto *widget = qobject_cast<QTableWidget*>(sender());
    QPoint popup = widget->mapToGlobal(point);
    popupMenu_->exec(popup);
}

void MainWindow::saveSettings()
{
    if(!ui)
        return;
    QSettings settings(this);
    settings.setValue(NumIter, ui->iterations->value());
    settings.setValue(NumStartAngle, ui->alpha_0->value());
    settings.setValue(NumYoungModule, ui->YoungModule->value());
    settings.setValue(NumTrussLength, ui->arcLength->value());
    settings.setValue(NumCrossArea, ui->cross_area->value());
    settings.setValue(NumForceAngle, ui->forceAngle->value());
    settings.setValue(NumSupportStiffness, ui->kv->value());
    settings.setValue(NumPlotScale, ui->plotScale->value());
    settings.setValue(NumPlotScaleElast, ui->plotScale2->value());
    settings.setValue(NumAfCal, ui->afcal->value());
    settings.setValue(NumHw, ui->hw->value());
    settings.setValue(BoolDeriv, (ui->isDeriv->checkState()==Qt::Checked));
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    saveSettings();
    e->accept();
}

void MainWindow::onPlus()
{
    const double val = ui->kv->value();
    QString _kv = ui->cKv->text();
    const double nval = TextToDouble(_kv);
    const double result = val+nval;
    if(!fZeroCheck(result)) {
        ui->kv->setValue(result);
    }
#ifdef IS_DEBUG
    qDebug() << "Plus pressed" <<_kv;
    qDebug() << result;
#endif
}

void MainWindow::onSubst()
{
    const double nval = TextToDouble(ui->cKv->text());
    if(!fZeroCheck(nval)) {
        ui->kv->setValue(nval);
    }
}

void MainWindow::enableButtons(bool enabled)
{
    ui->plusBtn->setEnabled(enabled);
    ui->subsBtn->setEnabled(enabled);
    if(enabled) {
        connect(ui->plusBtn, &QPushButton::pressed, this, &MainWindow::onPlus);
        connect(ui->subsBtn, &QPushButton::pressed, this, &MainWindow::onSubst);
    }
    else{
        ui->plusBtn->disconnect();
        ui->subsBtn->disconnect();
    }
}
