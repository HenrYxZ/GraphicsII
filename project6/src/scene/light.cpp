#include <cmath>

#include "light.h"



using namespace std;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
    Vec3d shadow = Vec3d(1.0, 1.0, 1.0);
  Vec3d d = getDirection(P);
  isect i;
  ray shadow_ray (P, d, ray::SHADOW);
  while (scene->intersect(shadow_ray, i)) {
    const Material& m = i.getMaterial();
    shadow = prod(shadow, m.kt(i));
    shadow_ray = ray(shadow_ray.at(i.t), d, ray::SHADOW);
  }

  return shadow;

}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
  // Color doesn't depend on P 
  return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
  return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{

  // YOUR CODE HERE

  // You'll need to modify this method to attenuate the intensity 
  // of the light based on the distance between the source and the 
  // point P.  For now, we assume no attenuation and just return 1.0
  double dist = (position - P).length();
  dist = constantTerm + linearTerm*dist + quadraticTerm*dist*dist;
  return 1.0 / dist;

}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
  // Color doesn't depend on P 
  return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
  Vec3d ret = position - P;
  ret.normalize();
  return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  Vec3d shadow = Vec3d(1.0, 1.0, 1.0);
  Vec3d d = getDirection(P);
  isect i;
  ray shadow_ray (P, d, ray::SHADOW);
  // make sure the intersection point is between point P and the light
  while ((scene->intersect(shadow_ray, i)) & ((d*getDirection(shadow_ray.at(i.t))) >= 0)) {
    const Material& m = i.getMaterial();
    shadow = prod(shadow, m.kt(i));
    shadow_ray = ray(shadow_ray.at(i.t), d, ray::SHADOW);
  }

  return shadow;

}
