/*
 * misescalcnelast.h
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
#ifndef MISESCALC_NON_ELAST
#define MISESCACL_NON_ELAST

#include <QVector>
#include <QPoint>
#include <QSharedPointer>
#include <cmath>
#include "defines.h"

class MisesCalcNonElast
{
public:
    explicit MisesCalcNonElast();
    typedef QSharedPointer<MisesCalcNonElast> Ptr;

public:
    void setStartAnlge(const double &value);
    void setYoungModule(const double &value);
    void setTrussLength(const double &value);
    void setCsArea(const double &value);
    void setForceAngle(const double &value);
    void setSupportStfns(const double &value);
    void setIterations(const double &value);
    void setScale(const double &value) { scale_ = value; }
    void countScale(const double &angle);
    QVector<QPointF> getAllForces();
    QVector<QPointF> getAllDerivatives();
    QVector<QPointF> getExtremums(const double &startAngle, const double &stopAngle);
    QVector<QString> getAngles();
    QPointF extremum();
    double scale() const {return scale_;}
    void doCalculate();

private:
    double radians(const double &angle) const { return angle*M_PI/180.0; }
    double degrees(const double &angle) const { return angle*180.0/M_PI; }
    double getForce(const double &value) const;
    double getDfDx(const double &value) const;
    double getNewX1Point(const double &a, const double &b) const {return (b - (b-a)/PHI);}
    double getNewX2Point(const double &a, const double &b) const {return (a + (b-a)/PHI);}
    double getEpsion(const double &a, const double &b) const {return qAbs(b-a);}
    QPointF findExtremum(const double& a, const double &b, const double &epsilon);
    bool fZeroCheck(const double &a) const { return (qAbs(a-ZERO) < EPSILON_);}
    void calculateExtremums(const double &startAngle, const double &stopAngle);
    void obtainForcesVectors();
    double pow2(const double &a) const {return a*a;}

private:
    double iterations_;
    double startAngle_;
    double youngModule_;
    double trussLength_;
    double csArea_;
    double forceAngle_;
    double supportStfns_;
    double tgA_;
    double tgB_;
    double sinA_;
    double scale_;
    QPointF extremum_;
    QVector<QPointF> curveFx_;
    QVector<QPointF> curvedFx_;
    QVector<QPointF> extremums_;
    QVector<QString> angles_;
};
#endif // MISESCALC_NON_ELAST
