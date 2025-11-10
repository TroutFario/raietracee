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
        if (isParallelTo(L))
        {
            return Vec3(NAN);
        }
        Vec3 offsetFromCenter = L.origin() - m_center;
        float t = Vec3::dot(offsetFromCenter, m_normal) / Vec3::dot(L.direction(), m_normal);
        return L.origin() + L.direction() * t;
    }
};
#endif
