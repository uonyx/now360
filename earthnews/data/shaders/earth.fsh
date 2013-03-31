//
//  earth.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

precision lowp float;

// source light colours
uniform vec4 u_ambientLight;
uniform vec4 u_diffuseLight;
uniform vec4 u_specularLight;
uniform float u_shininess;

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

/*
float linearstep (float e0, float e1, float x)
{
  float t = (x - e0) / (e1 - e0);
  
  return t;
  //return clamp (t, c_zero, c_one);
}
*/

void main (void)
{
  vec4 nightMat = texture2D (u_nightMap, v_texcoord) * 0.5;
  vec4 diffuseMat = texture2D (u_diffuseMap, v_texcoord);
  vec4 specularMat = texture2D (u_glossMap, v_texcoord);
  
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
  float d = max (dotp, c_zero);
  float n = c_one - d;
  //n = smoothstep (0.7, 0.9, n); /* nice but expensive */
  //n = clamp ((n - 0.7) / 0.2, c_zero, c_one);
  //n = n * n * (3.0 - c_two * n);
  
  //vec4 diffuse = (d * u_diffuseLight * diffuseMat) + (n * u_diffuseLight * nightMat);
  vec4 diffuse = ((d * diffuseMat) + (n * nightMat)) * u_diffuseLight;
  
  
  // specular
#if 1
  // phong model
  vec3 r = (c_two * dotp * nVec) - lVec;
  float s = pow (max (dot (vVec, r), c_zero), u_shininess);
  vec4 specular = s * u_specularLight * specularMat * diffuseMat;
  
#else
  // blinn model
  vec3 h = normalize (vVec + lVec);
  float s = pow (max (dot (nVec, h), c_zero), u_shininess);
  vec4 specular = s * u_specularLight * specularMat;
#endif
  
  /*
  lowp vec4 diffuseColour = texture2D (u_diffuseMap, v_texcoord) * v_colour;
  lowp vec4 nightColour = texture2D (u_nightMap, v_texcoord) * (white - v_colour);  
  gl_FragColor = diffuseColour + nightColour;
  */
    
  gl_FragColor = ambient + (diffuse + specular);
}