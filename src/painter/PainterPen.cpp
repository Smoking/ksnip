/*
 * Copyright (C) 2017 Damir Porobic <https://github.com/damirporobic>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*
 * Credit for the smooth algorithm goes to Bojan Kverh, see his post here:
 * https://www.toptal.com/c-plus-plus/rounded-corners-bezier-curves-qpainter
 */

#include "PainterPen.h"

PainterPen::PainterPen(const QPointF& pos, const QPen& attributes) :
    AbstractPainterItem(attributes),
    mPath(new QPainterPath)
{
    // Place the path at the right location and draw the first point, which is
    // actually a line just moved one pixel as QT won't draw a line if the point
    // B is equal to point A
    mPath->moveTo(pos);
    mPath->lineTo(pos + QPointF(1, 1));

    setCapStyle(Qt::RoundCap);
    setJoinStyle(Qt::RoundJoin);
}

PainterPen::PainterPen(const PainterPen& other) : AbstractPainterItem(other)
{
    this->mPath = new QPainterPath(*other.mPath);
}

PainterPen::~PainterPen()
{
    delete mPath;
}

void PainterPen::addPoint(const QPointF& pos, bool modifier)
{
    Q_UNUSED(modifier);

    prepareGeometryChange();
    mPath->lineTo(pos);
    updateShape();
}

void PainterPen::moveTo(const QPointF& newPos)
{
    prepareGeometryChange();
    mPath->translate(newPos - offset() - boundingRect().topLeft());
    updateShape();
}

/*
 * Simple function that smooths out the existing path. The function basically
 * collects all points in a path and skips points that are too close to each
 * other, to close points prevent smoothing out. Then in next step on every
 * point we remove the corner between two lines and replace it with an quadTo
 * Bezier curve. The min factor is used to test the distance between two points,
 * distance below min is ignored and points skipped.
 */
void PainterPen::smoothOut(float factor)
{
    QList<QPointF> points;
    QPointF p;
    for (auto i = 0; i < mPath->elementCount() - 1; i++) {
        p = QPointF(mPath->elementAt(i).x, mPath->elementAt(i).y);

        // Except for first and last points, check what the distance between two
        // points is and if its less the min, don't add them to the list.
        if (points.count() > 1 && (i < mPath->elementCount() - 2) && (MathHelper::distanceBetweenPoints(points.last(), p) < factor)) {
            continue;
        }
        points.append(p);
    }

    // Don't proceed if we only have 3 or less points.
    if (points.count() < 3) {
        return;
    }

    QPointF pt1;
    QPointF pt2;
    QPainterPath path;

    for (auto i = 0; i < points.count() - 1; i++) {
        pt1 = MathHelper::getBeginOfRounding(points[i], points[i + 1]);
        if (i == 0) {
            path.moveTo(pt1);
        } else {
            path.quadTo(points[i], pt1);
        }
        pt2 = MathHelper::getEndOfRounding(points[i], points[i + 1]);
        path.lineTo(pt2);
    }

    prepareGeometryChange();
    mPath->swap(path);
    updateShape();
}

void PainterPen::updateShape()
{
    // Should be fixed when we paint only parent path from shape
    changeShape(*mPath);
}

void PainterPen::paint(QPainter* painter, const QStyleOptionGraphicsItem* style, QWidget* widget)
{
    Q_UNUSED(style);
    Q_UNUSED(widget);

    painter->setPen(attributes().color());
    painter->setBrush(attributes().color());
    painter->drawPath(shape());

    paintDecoration(painter);
}
