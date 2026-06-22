#include <stdio.h>
#include <math.h>
#include "vectors.h"

Vector initVector(double x, double y, double z) {
    Vector v = {x, y, z};
    return v;
}

Vector addVector(Vector v1, Vector v2) {
    Vector v3 = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
    return v3;
}

Vector subVector(Vector v1, Vector v2) {
    Vector v3 = {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
    return v3;
}

Vector scalarMultipl(Vector v, double t) {
    Vector result = {v.x * t, v.y * t, v.z * t};
    return result;
}

Vector scalarDiv(Vector v, double t) {
    if (t == 0) {
        printf("Division by zero!\n");
        return v;
    }
    Vector result = {v.x / t, v.y / t, v.z / t};
    return result;
}

double magnitude(Vector v) {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector normaliseVector(Vector v) {
    double r = magnitude(v);
    if (r == 0) {
        printf("Cannot normalise a zero vector!\n");
        return v;
    }
    return scalarDiv(v, r);
}

double dotProduct(Vector v1, Vector v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector crossProduct(Vector v1, Vector v2) {
    Vector v3 = {
        v1.y * v2.z - v1.z * v2.y,
        v1.z * v2.x - v1.x * v2.z,
        v1.x * v2.y - v1.y * v2.x
    };
    return v3;
}
