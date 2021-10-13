/*
 * misescalcnelast.cpp
 * Copyright (C) 2018-2021 Vitaly Tonkacheyev
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
#include "misescalcnelast.h"
#include <cmath>
#include <QPointF>
#include <QVector>
#include <QLocale>
#ifdef IS_DEBUG
#include <QDebug>
#endif

MisesCalcNonElast::MisesCalcNonElast()
    : iterations_(ZERO), startAngle_(ZERO), youngModule_(ZERO), trussLength_(ZERO), csArea_(ZERO),
      forceAngle_(ZERO), supportStfns_(ZERO), tgA_(ZERO), tgB_(ZERO), sinA_(ZERO), scale_(ZERO),
      extremum_(QPointF()), curveFx_(QVector<QPointF>()), curvedFx_(QVector<QPointF>()),
      extremums_(QVector<QPointF>()), angles_(QVector<QString>())
{
}

double MisesCalcNonElast::getForce(const double &value) const
{
    const double A = fZeroCheck(csArea_) || fZeroCheck(youngModule_)
            ? ZERO
            : value*trussLength_*supportStfns_/(youngModule_*csArea_);
    const double var1 = fZeroCheck(tgA_) ? ZERO : 1.0/tgA_ - value;
    const double var12 = pow2(var1);
    const double var4 = 1.0-value*tgB_;
    const double var5 = 1.0+value*tgB_;
    const double var42 = pow2(var4);
    const double var52 = pow2(var5);
    const double var2 = sqrt(var42 + var12);
    const double var3 = sqrt(var52 + var12);
    const double B = fZeroCheck(var2) ? ZERO : var1/var2;
    const double C = fZeroCheck(var3) ? ZERO : var1/var3;
    const double D = -2.0*sinA_*var1;
    return A+B+C+D;
}

double MisesCalcNonElast::getDfDx(const double &value) const
{
    const double var1 = tgB_*value+1.0;
    const double var2 = fZeroCheck(tgA_) ? ZERO : 1.0/tgA_ - value;
    const double var3 = 1.0-tgB_*value;
    const double var12 = pow2(var1);
    const double var22 = pow2(var2);
    const double var32 = pow2(var3);
    const double var4 = (var12 + var22)*(var12 + var22)*(var12 + var22);
    const double var5 = (var32 + var22)*(var32 + var22)*(var32 + var22);
    const double dA = (-1.0)/sqrt(var12 + var22);
    const double dB = (-1.0)*var2*(2*tgB_*var1-2*var2)/(2*sqrt(var4));
    const double dC = (-1.0)/sqrt(var32+var22);
    const double dD = (-1.0)*var2*(-2*tgB_*var3-2*var2)/(2*sqrt(var5));
    const double dE = fZeroCheck(csArea_) || fZeroCheck(youngModule_)
            ? ZERO
            : trussLength_*supportStfns_/(csArea_*youngModule_);
    return dA+dB+dC+dD+dE + 2*sinA_;
}

void MisesCalcNonElast::countScale(const double &angle)
{
    const double prAngle = qAbs(angle - 45.0) + 1;
    const double val = 3.851e-4 * pow2(prAngle) - 6.068e-2 * prAngle + 2.037;
    const double scale1 = !fZeroCheck(iterations_) ? 3 / iterations_ : ZERO;
    scale_ = fZeroCheck(iterations_) ? qMax(scale1, val) : qMax(scale1, val / iterations_);
}

QPointF MisesCalcNonElast::findExtremum(const double &a, const double &b, const double &epsilon)
{
    double bi = b, ai = a;
    double x1 = getNewX1Point(a, b);
    double x2 = getNewX2Point(a, b);
    double dx = getEpsion(a, b);
#ifdef IS_DEBUG
    qDebug() << "x1 = " << x1;
    qDebug() << "x2 = " << x2;
    qDebug() << "dx = " << dx;
    int i = 0;
#endif
    while(dx > epsilon){
        double y1 = getForce(x1);
        double y2 = getForce(x2);
        if (y1 < y2) {
            ai = x1;
            x1 = x2;
            x2 = getNewX2Point(ai, bi);
        }
        else {
            bi = x2;
            x2 = x1;
            x1 = getNewX1Point(ai, bi);
        }
        dx = getEpsion(ai, bi);
#ifdef IS_DEBUG
        qDebug() << "x1 = " << x1;
        qDebug() << "x2 = " << x2;
        qDebug() << "dx = " << dx;
        ++i;
#endif
    }
#ifdef IS_DEBUG
    qDebug() << "iters = " << i;
#endif
    const double resultX = (bi+ai)/2;
    const double resultY = getForce(resultX);
    return {resultX, resultY};
}

void MisesCalcNonElast::obtainForcesVectors()
{
    curveFx_.clear();
    curvedFx_.clear();
    double i = ZERO;
    while(i <= iterations_) {
        const double x = i * scale_;
        const double y = getForce(x);
        curveFx_ << QPointF(x, y);
        const double dy = getDfDx(x);
        curvedFx_ << QPointF(x, dy);
        ++i;
    }
#ifdef IS_DEBUG
    qDebug() << curveFx_;
    qDebug() << curvedFx_;
#endif

}

QVector<QPointF> MisesCalcNonElast::getAllForces()
{
    return  curveFx_;
}

QVector<QPointF> MisesCalcNonElast::getAllDerivatives()
{
    return curvedFx_;
}

QPointF MisesCalcNonElast::extremum()
{
    double stopPoint = ZERO;
    for (auto& point : curvedFx_) {
        int index = curvedFx_.indexOf(point) + 1;
        if (index < curvedFx_.count()) {
            const QPointF next = curvedFx_.at(index);
            if (point.y() >= ZERO && next.y() < ZERO) {
                stopPoint = next.x();
                break;
            }
        }
    }
    if (stopPoint > ZERO) {
        extremum_ = findExtremum(ZERO, stopPoint, EPSILON_);
    }
    return extremum_;
}

QVector<QPointF> MisesCalcNonElast::getExtremums(const double &startAngle, const double &stopAngle)
{
    calculateExtremums(startAngle, stopAngle);
    return extremums_;
}

QVector<QString> MisesCalcNonElast::getAngles()
{
    return angles_;
}

void MisesCalcNonElast::calculateExtremums(const double &startAngle, const double &stopAngle)
{
    extremums_.clear();
    angles_.clear();
    const QVector<QPointF> oldForces = curveFx_;
    const QVector<QPointF> oldDfxs = curvedFx_;
    const QPointF oldExtremum = extremum_;
    const double oldIters = iterations_;
    double initialAngle = startAngle;
    initialAngle = (initialAngle > forceAngle_) ? initialAngle : forceAngle_ + 1;
    const double finalAngle = stopAngle;
    iterations_ = 100.0;
    while(initialAngle <= finalAngle) {
        double radangle = radians(initialAngle);
        countScale(radangle);
        tgA_ = tan(radangle);
        sinA_ = sin(radangle);
        obtainForcesVectors();
        angles_ << QLocale::system().toString(initialAngle, 'g', 9);
        extremums_ << extremum();
        ++initialAngle;
    }
    curveFx_ = oldForces;
    curvedFx_ = oldDfxs;
    extremum_ = oldExtremum;
    iterations_ = oldIters;
}

void MisesCalcNonElast::setStartAnlge(const double &value)
{
    startAngle_ = radians(value);
    tgA_ = tan(startAngle_);
    sinA_ = sin(startAngle_);
}

void MisesCalcNonElast::setCsArea(const double &value)
{
    csArea_ = value;
}

void MisesCalcNonElast::setYoungModule(const double &value)
{
    youngModule_ = value*1e6;
}

void MisesCalcNonElast::setTrussLength(const double &value)
{
    trussLength_ = value / 2.0;
}

void MisesCalcNonElast::setForceAngle(const double &value)
{
    forceAngle_ = radians(value);
    tgB_ = tan(forceAngle_);
}
void MisesCalcNonElast::setSupportStfns(const double &value)
{
    supportStfns_ = value;
}
void MisesCalcNonElast::setIterations(const double &value)
{
    iterations_ = value;
}

void MisesCalcNonElast::doCalculate()
{
    obtainForcesVectors();
}
