#ifndef __MATH_H__
#define __MATH_H__

#include <D3DX10.h>

const float float_epsilon = 0.00001f;

//struct Box
//{
//	XMVECTOR min_point;
//	XMVECTOR max_point;
//
//	XMVECTOR points[8];	// All the 8 corner points of the box
//
//	Box(){}; 
//
//	Box(XMVECTOR min_point, XMVECTOR max_point)
//	{
//		this->min_point = min_point;
//		this->max_point = max_point;
//
//		// Init the 8 points
//		// Front face
//		points[0] = XMVECTOR(min_point.x, max_point.y, min_point.z);
//		points[1] = XMVECTOR(max_point.x, max_point.y, min_point.z);
//		points[2] = XMVECTOR(max_point.x, min_point.y, min_point.z);
//		points[3] = min_point;
//
//		// Back face
//		points[4] = XMVECTOR(min_point.x, max_point.y, max_point.z);
//		points[5] = max_point;
//		points[6] = XMVECTOR(max_point.x, min_point.y, max_point.z);
//		points[7] = XMVECTOR(min_point.x, min_point.y, max_point.z);
//	}
//};

// Triangle
struct Triangle
{
	XMVECTOR v1 ;
	XMVECTOR v2 ;
	XMVECTOR v3 ;

	Triangle(XMVECTOR v1, XMVECTOR v2, XMVECTOR v3)
	{
		this->v1 = v1 ;
		this->v2 = v2 ;
		this->v3 = v3 ;
	}
} ;

// Rectangle
struct Rect 
{
	XMVECTOR v1 ; // top-left 
	XMVECTOR v2 ; // top-right
	XMVECTOR v3 ; // left-bottom
	XMVECTOR v4 ; // right-bottom

	Rect(){};

	Rect(XMVECTOR& v1, XMVECTOR& v2, XMVECTOR& v3, XMVECTOR& v4)
	{
		this->v1 = v1 ;
		this->v2 = v2 ;
		this->v3 = v3 ;
		this->v4 = v4 ;
	}
};

// Picking ray
struct Ray
{
	XMVECTOR origin;
	XMVECTOR direction;

	Ray(){}

	Ray(XMVECTOR origin, XMVECTOR direction)
	{
		this->origin    = origin;
		this->direction = direction;
	}
};

// Calculate the square distance of two points
inline float SquareDistance(XMVECTOR v1, XMVECTOR v2)
{
	return (XMVectorGetX(v1) - XMVectorGetX(v2)) * (XMVectorGetX(v1) - XMVectorGetX(v2)) +
		   (XMVectorGetY(v1) - XMVectorGetY(v2)) * (XMVectorGetY(v1) - XMVectorGetY(v2)) +
		   (XMVectorGetZ(v1) - XMVectorGetZ(v2)) * (XMVectorGetZ(v1) - XMVectorGetZ(v2));
}

// Determine whether a ray intersect with a triangle
// Parameters
// orig: origin of the ray
// dir: direction of the ray
// v0, v1, v2: vertices of triangle
// t(out): weight of the intersection for the ray
// u(out), v(out): barycentric coordinate of intersection
inline bool RayTriangleIntersection(Ray* ray, Triangle* triangle, XMVECTOR* hit_point)
{
	XMVECTOR orig = ray->origin;
	XMVECTOR dir  = ray->direction;

	XMVECTOR v0 = triangle->v1;
	XMVECTOR v1 = triangle->v2;
	XMVECTOR v2 = triangle->v3;

	// E1
	XMVECTOR E1 = v1 - v0;

	// E2
	XMVECTOR E2 = v2 - v0;

	// P
	XMVECTOR P = XMVector3Cross(dir, E2);

	// determinant
	//float det = E1.Dot(P);
	float det = XMVectorGetX(XMVector3Dot(E1, P));

	// keep det > 0, modify T accordingly
	XMVECTOR T;
	if( det >0 )
	{
		T = orig - v0;
	}
	else
	{
		T = v0 - orig;
		det = -det;
	}

	// If determinant is near zero, ray lies in plane of triangle
	if( det < 0.0001f )
		return false;

	// Calculate u and make sure u <= 1
	float u = XMVectorGetX(XMVector3Dot(T, P));
	if( u < 0.0f || u > det )
		return false;

	// Q
	XMVECTOR Q = XMVector3Cross(T, E1);

	// Calculate v and make sure u + v <= 1
	float v = XMVectorGetX(XMVector3Dot(dir, Q));
	if( v < 0.0f || u + v > det )
		return false;

	// Calculate t, scale parameters, ray intersects triangle
	float t = XMVectorGetX(XMVector3Dot(E2, Q));

	float fInvDet = 1.0f / det;
	t *= fInvDet;
	u *= fInvDet;
	v *= fInvDet;

	// Intersection point can be calculate by the following two ways
	// 1. O + Dt
	// 2. (1 - u - v) * v0 + u * v1 + v * v2
	// I use the first way.
	*hit_point = orig + (t * dir);

	return true;
}

// Determine Whether a ray intersect with a rectangle
// Divide the rectangle into two triangles and if the ray intersect with any of them
inline bool RayRectIntersection(Ray& ray, Rect& rect, XMVECTOR& hit_point)
{
	// Divide the rectangle into two triangles
	Triangle t1(rect.v1, rect.v2, rect.v3);
	Triangle t2(rect.v1, rect.v3, rect.v4) ;

	return RayTriangleIntersection(&ray, &t1, &hit_point) || RayTriangleIntersection(&ray, &t2, &hit_point);
}

// Determine whether a plane was intersect with a box
// If all the 8 points of the box exists on the same side of the plane, they are not intersect.
// else they are intersect.
//inline bool PlaneBoxIntersection(const D3DXPLANE& plane, const Box& box)
//{
//	// Normalize the plane
//	D3DXPLANE normalized_plane;
//	D3DXPlaneNormalize(&normalized_plane, &plane);
//
//	// Use a variable side_count to count the points
//	// If the point on the positive side, increase the side_count, else decrease the side_count
//	// After all 8 points were tested, if side_count == 8, then all points on the positive side
//	// on the other hand, if side_count == -8, all points exists on the negative side, only return true for 
//	// these two conditions.
//	int side_count = 0;
//	for (int i = 0; i < 8; ++i)
//	{
//		if (D3DXPlaneDotCoord(&normalized_plane, &box.points[i]) > 0.0f)
//		{
//			++side_count;
//		}
//		else
//		{
//			--side_count;
//		}
//	}
//
//	//return (side_count != 8 || side_count != -8);
//	return (side_count > -8 && side_count < 8);
//}

#endif //end __MATH_H__