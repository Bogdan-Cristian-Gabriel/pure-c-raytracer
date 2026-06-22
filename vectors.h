#ifndef VECTORS_H
#define VECTORS_H

typedef struct vector {
    double x, y, z;
} Vector;

Vector initVector(double x, double y, double z);
Vector addVector(Vector v1, Vector v2);
Vector subVector(Vector v1, Vector v2);
Vector scalarMultipl(Vector v, double t);
Vector scalarDiv(Vector v, double t);
double magnitude(Vector v);
Vector normaliseVector(Vector v);
double dotProduct(Vector v1, Vector v2);
Vector crossProduct(Vector v1, Vector v2);
void printVector(Vector v);

#endif