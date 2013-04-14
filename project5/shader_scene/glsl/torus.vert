
attribute vec2 parametric;

uniform vec3 lightPosition;  // Object-space
uniform vec3 eyePosition;    // Object-space
uniform vec2 torusInfo;

varying vec2 normalMapTexCoord;

varying vec3 lightDirection;
varying vec3 halfAngle;
varying vec3 eyeDirection;
varying vec3 c0, c1, c2;

void main()
{
  const float pi2 = 6.28318530;  // 2 times Pi

  float outer_R = torusInfo[0];
  float inner_R = torusInfo[1];

  normalMapTexCoord = vec2(-6.0*parametric[0], 2.0*parametric[1]);

  float theta = parametric[0];
  float phi = parametric[1];

  float x = (outer_R+(inner_R*cos(phi*pi2)))*cos(theta*pi2);
  float y = (outer_R+(inner_R*cos(phi*pi2)))*sin(theta*pi2);
  float z = inner_R*sin(phi*pi2);
  gl_Position = gl_ModelViewProjectionMatrix * vec4(x, y, z, 1);
  eyeDirection = normalize(vec3(eyePosition[0] - x,
                                eyePosition[1] - y,
                                eyePosition[2] - z));
  lightDirection = vec3(lightPosition[0] - x,
                        lightPosition[1] - y,
                        lightPosition[2] - z);  // object-space
  halfAngle = (lightDirection + eyeDirection) / length(lightDirection + eyeDirection);  // XXX fix me
  c0 = vec3(0);  // XXX fix me
  c1 = vec3(0);  // XXX fix me
  c2 = vec3(0);  // XXX fix me

  // Diffuse calculations

  // d/du F(u, v)
  float dux = -pi2*sin(pi2*theta)*(outer_R+(inner_R*cos(phi*pi2)));
  float duy = pi2*cos(pi2*theta)*(outer_R+(inner_R*cos(phi*pi2)));
  float duz = 0.0;

  // d/dv F(u, v)
  float dvx = -pi2*inner_R*cos(pi2*theta)*sin(pi2*phi);
  float dvy = -pi2*inner_R*sin(pi2*theta)*sin(pi2*phi);
  float dvz = pi2*inner_R*cos(phi*pi2);

  vec3 tangent = normalize(vec3(dux, duy, duz));
  vec3 normal = cross(tangent, normalize(vec3(dvx, dvy, dvz)));
  vec3 binormal = cross(normal, tangent);

  mat3 obj_to_surf = inverse(mat3(tangent, binormal, normal));

  lightDirection = obj_to_surf * lightDirection;  // surface-space
  halfAngle = obj_to_surf * halfAngle;
}

