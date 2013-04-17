
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
  vec4 diffuse_color = mod(LMd*max(l[2], 0.0), l[2]);
  vec3 hA = normalize(halfAngle);
  vec4 spec_color = LMs*max(pow(hA[2], shininess), 0);
  gl_FragColor = diffuse_color + spec_color + LMa;
}
