
uniform vec4 LMa; // Light-Material ambient
uniform vec4 LMd; // Light-Material diffuse
uniform vec4 LMs; // Light-Material specular
uniform float shininess;

uniform sampler2D normalMap;
uniform sampler2D decal;
uniform sampler2D heightField;
uniform samplerCube envmap;

uniform mat3 objectToWorld;

varying vec2 normalMapTexCoord;
varying vec3 lightDirection;
varying vec3 eyeDirection;
varying vec3 halfAngle;
varying vec3 c0, c1, c2;

void main()
{
  vec3 normap = (texture2D(normalMap, normalMapTexCoord) - 0.5)*2.0;
  mat3 M = mat3(c0,c1,c2);
  vec3 e = eyeDirection*M;  // view vector in surface space
  vec3 r = reflect(e, normap);
  r = M*r;
  r = objectToWorld*r;
  gl_FragColor = textureCube(envmap, r*-1.0);
}
