#ifndef PLANE_H
#define PLANE_H
#include "Vec3.h"
#include "Line.h"
class Plane
{
private:
    Vec3 m_center, m_normal;

public:
    Plane() {}
    Plane(Vec3 const &c, Vec3 const &n)
    {
        m_center = c;
        m_normal = n;
        m_normal.normalize();
    }
    void setCenter(Vec3 const &c) { m_center = c; }
    void setNormal(Vec3 const &n)
    {
        m_normal = n;
        m_normal.normalize();
    }
    Vec3 const &center() const { return m_center; }
    Vec3 const &normal() const { return m_normal; }
    Vec3 project(Vec3 const &p) const
    {
        Vec3 result = p - m_normal;
        float t = Vec3::dot((p - m_center), m_normal);
        return result;
    }
    float squareDistance(Vec3 const &p) const { return (project(p) - p).squareLength(); }
    float distance(Vec3 const &p) const { return sqrt(squareDistance(p)); }
    bool isParallelTo(Line const &L) const
    {
        return Vec3::dot(L.direction(), m_normal) == 0;
    }
    Vec3 getIntersectionPoint(Line const &L) const
    {
        if (this->isParallelTo(L))
        {
            return Vec3(NAN, NAN, NAN);
        }

        // N.(O + tD - C) = 0
        // N.O + t(N.D) - N.C = 0
        // t(N.D) = N.C - N.O
        // t = (N.C - N.O)/N.D
        // t = N.(C - O)/N.D

        // X = O + tD

        double t = (Vec3::dot(m_normal, m_center - L.origin())) / Vec3::dot(m_normal, L.direction());

        Vec3 result = L.origin() + t * L.direction();

        return result;
    }
};
#endif
