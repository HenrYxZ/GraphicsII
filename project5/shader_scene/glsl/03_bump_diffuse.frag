
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
  vec3 l = normalize(lightDirection);
  vec3 normap = (texture2D(normalMap, normalMapTexCoord) - 0.5)*2.0;
  vec3 heightmap = texture2D(heightField, normalMapTexCoord);
  float lnorm = dot(normap, l);
  vec4 diffuse_color = mod(LMd*max(lnorm, 0.0), lnorm);
  gl_FragColor = diffuse_color + LMa;
}
