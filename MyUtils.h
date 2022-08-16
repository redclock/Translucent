#pragma once
#include "Point3.h"

inline void coordinateSystem(const Point3& v1, Point3* v2, Point3* v3)
{
    float invLen;
    if (fabs(v1.x) > fabs(v1.y))
    {
        invLen = 1.0f / sqrt(v1.x * v1.x + v1.z * v1.z);
        *v2 = Point3(-v1.z * invLen, 0.0, v1.x * invLen);
    }
    else if (v1.y != 0.0f || v1.x != 0.0f || v1.z != 0.0f)
    {
        invLen = 1.0f / sqrt(v1.y * v1.y + v1.z * v1.z);
        *v2 = Point3(0.0f, v1.z * invLen, -v1.y * invLen);
    }
    else
    {
        *v2 = Point3(0.0f, 1.0f, 0.0f);
    }
    *v3 = v1 % (*v2);
}

const char* getExtName(const char* filename)
{
    size_t len = strlen(filename);
    const char* s = filename + len - 1;
    while (s >= filename)
    {
        if (*s == '.')
            return s + 1;
        s--;
    }
    return filename + len;
}

const char* getFileName(const char* filename)
{
    size_t len = strlen(filename);
    const char* s = filename + len - 1;
    while (s >= filename)
    {
        if (*s == '\\' || *s == '/')
            return s + 1;
        s--;
    }
    return filename;
}
