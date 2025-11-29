#include "ShapeFactory.h"

Shape *New_Point(double x, double y)
{
    return new Point(x, y);
}

Shape *New_Rectangle(double x1, double y1, double x2, double y2)
{
    return new Rectangle(x1, y1, x2, y2);
}

Shape *New_Circle(double x, double y, double r)
{
    return new Circle(x, y, r);
}
