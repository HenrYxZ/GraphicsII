
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
  eyeDirection = eyePosition - gl_Position;
  lightDirection = lightPosition - gl_Position;
  halfAngle = vec3(0);  // XXX fix me
  c0 = vec3(0);  // XXX fix me
  c1 = vec3(0);  // XXX fix me
  c2 = vec3(0);  // XXX fix me
}

