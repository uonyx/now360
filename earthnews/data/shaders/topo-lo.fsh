//
//  topo-lo.fsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

precision lowp float;

// source light colours
uniform vec4 u_ambientLight;
uniform vec4 u_diffuseLight;

uniform sampler2D u_normalMap;
uniform sampler2D u_diffuseMap;
uniform sampler2D u_glossMap;
uniform sampler2D u_nightMap;

varying vec2 v_texcoord;
varying vec3 v_viewDir;
varying vec3 v_lightDir;

const float c_zero = 0.0;
const float c_one = 1.0;
const float c_two = 2.0;

void main (void)
{
  vec4 nightMat = texture2D (u_nightMap, v_texcoord) * 0.5;
  vec4 diffuseMat = texture2D (u_diffuseMap, v_texcoord);
 
  // get tangent space normal from normal maps
  vec3 nVec = texture2D (u_normalMap, v_texcoord).xyz;
  
  // rgb range [0, 1] to [-1, 1], and normalize
  nVec = normalize ((nVec * c_two) - c_one);
  
  // view vector
  vec3 vVec = normalize (v_viewDir);
  
  // light vector
  vec3 lVec = normalize (v_lightDir);
  
  // n.l
  float dotp = dot (nVec, lVec);
  
  // ambient
  vec4 ambient = (u_ambientLight * diffuseMat);// + (nightMat * vec4 ((0.7 * u_ambientLight).xyz, 1.0));
  
  // diffuse
  float d = max (dotp, 0.1);
  float n = c_one - d;
  vec4 diffuse = ((d * diffuseMat) + (n * nightMat)) * u_diffuseLight;

  u_glossMap; // unused

  gl_FragColor = ambient + diffuse;
}
