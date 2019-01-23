/*
 * misescalcelast.cpp
 * Copyright (C) 2018-2019 Vitaly Tonkacheyev
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
#include "misescalcelast.h"
//#include "defines.h"
#include <cmath>
#include <QPointF>
#include <QVector>
#include <QLocale>
#ifdef IS_DEBUG
#include <QDebug>
#endif

MisesCalcElast::MisesCalcElast()
    :iterations_(ZERO),
      startAngle_(ZERO),
      youngModule_(ZERO),
      trussLength_(ZERO),
      csArea_(ZERO),
      forceAngle_(ZERO),
      supportStfns_(ZERO),
      hwM_(ZERO),
      Afcal_(ZERO),
      tgA_(ZERO),
      tgB_(ZERO),
      sinA_(ZERO),
      cosA_(ZERO),
      scale_(ZERO),
      extremum_(QPointF()),
      curveFx_(QVector<QPointF>()),
      allkeM_(QVector<QPointF>()),
      extremums_(QVector<QPointF>()),
      angles_(QVector<QString>())
{
}

double MisesCalcElast::getC2(const double &vRel) const
{
    return 1.0-vRel*tgA_;
}

double MisesCalcElast::getA() const
{
    return sinA_*Afcal_*pow2(hwM_)/(2.0*trussLength_);
}

double MisesCalcElast::getB(const double &vRel) const
{
    const double znam = getC2(vRel);
    return fZeroCheck(znam) ? ZERO : atan(sqrt(pow2(tgA_)/pow2(znam))) - startAngle_;
}

double MisesCalcElast::getC(const double &vRel) const
{
    const double znam1 = pow2(tgA_)+pow2(getC2(vRel));
    const double znam2 = trussLength_*csArea_;
    const double part1 = fZeroCheck(znam1) ? ZERO : pow2(tgA_)/znam1;
    const double part2 = fZeroCheck(znam2) ? ZERO : 2.0/znam2;
    return fZeroCheck(znam2)||fZeroCheck(znam1) ? ZERO : part1*part2;
}

double MisesCalcElast::getD(const double &vRel) const
{
    const double C2 = getC2(vRel);
    const double znam3 = sqrt(pow2(tgA_)+pow2(C2));
    return fZeroCheck(znam3) ? ZERO : 2.0*C2/znam3;
}

double MisesCalcElast::getE(const double &vRel) const
{
    const double C2 = getC2(vRel);
    return (-2.0*cosA_*C2);
}

double MisesCalcElast::getKeM(const double &vRel) const
{
    return getA()*getB(vRel);
}

double MisesCalcElast::getForce(const double &value) const
{
    const double C = getC(value);
    const double D = getD(value);
    const double E = getE(value);
    double keM = getKeM(value);
    double result = ZERO;
    keM = (keM > ZERO) ? keM : ZERO;
    result = keM*C+D+E;
#ifdef IS_DEBUG
    qDebug() << "keM=" << keM;
    qDebug() << "C=" << C;
    qDebug() << "D=" << D;
    qDebug() << "E=" << E;
    qDebug() << "result=" << result;
#endif
    return result;
}

void MisesCalcElast::countScale(const double &angle)
{
    const double prAngle = degrees(angle) - 45.0 + 1.0;
    const double P1_ = 0.0003851;
    const double P2_ = 0.06068;
    const double result = (P1_*pow2(prAngle)-P2_*prAngle+2.037);
    scale_ = fZeroCheck(iterations_) ? result : result / iterations_;
}

QPointF MisesCalcElast::findExtremum(const double &a, const double &b, const double &epsilon)
{
    double bi = b, ai = a;
    double x1 = getNewX1Point(a, b);
    double x2 = getNewX2Point(a, b);
    double dx = getEpsion(a, b);
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
    }
    const double resultX = (bi+ai)/2;
    const double resultY = getForce(resultX);
    return QPointF(resultX, resultY);
}

void MisesCalcElast::obtainForcesVectors(const double &angle)
{
    curveFx_.clear();
    allkeM_.clear();
    countScale(angle);
    double i = ZERO;
    while(i <= iterations_) {
        const double x = i*scale_;
        const double y = getForce(x);
        const double k = getKeM(x);
        allkeM_ << QPointF(x, k);
        curveFx_ << QPointF(x, y);
        ++i;
    }
#ifdef IS_DEBUG
    qDebug() << allkeM_;
#endif
}

QPointF MisesCalcElast::extremum()
{
    double stopPoint = ZERO;
    foreach (const QPointF& point, allkeM_) {
        int index = allkeM_.indexOf(point) + 1;
        if (index < allkeM_.count()) {
            const QPointF next = allkeM_.at(index);
            if (point.y() >= ZERO && next.y() < ZERO) {
                stopPoint = (index < curveFx_.count()) ? curveFx_.at(index-1).x() : ZERO;
                break;
            }
        }
    }
    if (stopPoint > ZERO) {
        extremum_ = findExtremum(ZERO, stopPoint, EPSILON_);
    }
    return extremum_;
}

QVector<QPointF> MisesCalcElast::getExtremums(const double &startAngle, const double &stopAngle)
{
    calculateExtremums(startAngle, stopAngle);
    return extremums_;
}

QVector<QString> MisesCalcElast::getAngles()
{
    return angles_;
}

void MisesCalcElast::calculateExtremums(const double &startAngle, const double &stopAngle)
{
    extremums_.clear();
    angles_.clear();
    const QVector<QPointF> oldForces = curveFx_;
    const QVector<QPointF> oldKems = allkeM_;
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
        cosA_ = cos(radangle);
        obtainForcesVectors(radangle);
        angles_ << QLocale::system().toString(initialAngle, 'g', 9);
        extremums_ << extremum();
        ++initialAngle;
    }
    curveFx_ = oldForces;
    allkeM_ = oldKems;
    extremum_ = oldExtremum;
    iterations_ = oldIters;
}

void MisesCalcElast::setStartAnlge(const double &value)
{
    startAngle_ = radians(value);
    tgA_ = tan(startAngle_);
    sinA_ = sin(startAngle_);
    cosA_ = cos(startAngle_);
    countScale(startAngle_);
}

void MisesCalcElast::setIterations(const double &value)
{
    iterations_ = value;
    countScale(startAngle_);
}

void MisesCalcElast::doCalculate()
{
    obtainForcesVectors(startAngle_);
}
