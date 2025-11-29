#ifndef SHAPE_FACTORY_H_INCLUDED
#define SHAPE_FACTORY_H_INCLUDED

#include "Shape.h"
#include "Point.h"
#include "Rectangle.h"
#include "Circle.h"

Shape *New_Point(double x, double y);
Shape *New_Rectangle(double x1, double y1, double x2, double y2);
Shape *New_Circle(double x, double y, double r);

#endif
