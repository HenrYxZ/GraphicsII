
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
  // ambient diffuse+bump
  vec3 l = normalize(lightDirection);
  vec3 normap = (texture2D(normalMap, normalMapTexCoord) - 0.5)*2.0;
  float lnorm = dot(normap, l);
  vec4 diffuse_color = mod(LMd*max(lnorm, 0.0), lnorm) + LMa;

  // specular
  vec3 hA = normalize(halfAngle);
  float hAnorm = dot(normap, hA);
  vec4 spec_color = LMs*max(pow(hAnorm, shininess), 0);

  // bumpy environment
  mat3 M = mat3(c0,c1,c2);
  vec3 e = M*eyeDirection;  // view vector in surface space
  vec3 r = reflect(e, normap);
  r = r*M;
  r = objectToWorld*r;  // problem converting to world space
  gl_FragColor = (diffuse_color*0.5) + (spec_color*0.5) + (textureCube(envmap, r)*0.6);
}
