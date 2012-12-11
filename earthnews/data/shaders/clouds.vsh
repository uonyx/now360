//
//  clouds.vsh
//

#if 0
attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 u_mvpMatrix;
uniform mat3 u_normalMatrix;
//uniform vec4 u_eyePosition;
uniform vec4 u_lightPosition;

varying vec4 v_colour;
varying vec2 v_texcoord;

const float c_zero = 0.0;
const float c_one = 1.0;

void main (void)
{
  vec3 eyeNormal = normalize (u_normalMatrix * a_normal);
  //vec3 lightDirection = vec3 (c_zero, c_zero, -2.0); // target - position
  vec3 lightDirection = (u_lightPosition - a_position).xyz;
  vec4 diffuseColor = vec4 (c_one, c_one, c_one, c_one);
  
  float nDotVP = max(c_zero, dot(eyeNormal, normalize (lightDirection)));
                 
  v_colour = diffuseColor * nDotVP;
  
  v_texcoord = a_texcoord;
  
  gl_Position = u_mvpMatrix * a_position;
}

#else

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec3 a_tangent;
attribute vec3 a_bitangent;
attribute vec2 a_texcoord;

uniform mat4 u_mvpMatrix;
uniform vec4 u_eyePosition;
uniform vec4 u_lightPosition;

varying vec2 v_texcoord;
varying vec3 v_viewDir;
varying vec3 v_lightDir;

void main (void)
{
  // view and light direction vectors
  vec3 vVec = (u_eyePosition - a_position).xyz;
  vec3 lVec = (u_lightPosition - a_position).xyz;
  
  // tangent matrix
  mat3 tangentMat = mat3 (a_tangent, a_bitangent, a_normal);
  
  // transform view and light vectors to tangent space
  
  v_viewDir = vVec * tangentMat;
  v_lightDir = lVec * tangentMat;
  
  // tex coords
  v_texcoord = a_texcoord;
  
  // vertex position
  gl_Position = u_mvpMatrix * a_position;
}

#endif