//
//  earth.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

#if 1
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
  //vec4 specularMat = texture2D (u_glossMap, v_texcoord);
  
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
  
#if 0 // specular
  // phong model
  vec3 r = (c_two * dotp * nVec) - lVec;
  float s = pow (max (dot (vVec, r), c_zero), u_shininess);
  vec4 specular = s * u_specularLight * specularMat * diffuseMat;
#else
  u_shininess;
  u_specularLight;
  u_glossMap;
  
#endif
  /*
  lowp vec4 diffuseColour = texture2D (u_diffuseMap, v_texcoord) * v_colour;
  lowp vec4 nightColour = texture2D (u_nightMap, v_texcoord) * (white - v_colour);  
  gl_FragColor = diffuseColour + nightColour;
  */
    
  gl_FragColor = ambient + (diffuse);// + specular);
}

#else

varying lowp vec3 v_lightDir;
varying lowp vec3 v_viewDir;
varying lowp vec2 v_texcoord;
uniform sampler2D u_nightMap;
uniform sampler2D u_glossMap;
uniform sampler2D u_diffuseMap;
uniform sampler2D u_normalMap;
uniform lowp float u_shininess;
uniform lowp vec4 u_specularLight;
uniform lowp vec4 u_diffuseLight;
uniform lowp vec4 u_ambientLight;
void main ()
{
  lowp vec3 nVec_1;
  lowp vec4 specularMat_2;
  lowp vec4 diffuseMat_3;
  lowp vec4 nightMat_4;
  lowp vec4 tmpvar_5;
  
  tmpvar_5 = (texture2D (u_nightMap, v_texcoord) * 0.5);
  nightMat_4 = tmpvar_5;
  
  lowp vec4 tmpvar_6;
  tmpvar_6 = texture2D (u_diffuseMap, v_texcoord);
  diffuseMat_3 = tmpvar_6;
  lowp vec4 tmpvar_7;
  tmpvar_7 = texture2D (u_glossMap, v_texcoord);
  specularMat_2 = tmpvar_7;
  lowp vec3 tmpvar_8;
  tmpvar_8 = texture2D (u_normalMap, v_texcoord).xyz;
  nVec_1 = tmpvar_8;
  lowp vec3 tmpvar_9;
  tmpvar_9 = normalize(((nVec_1 * 2.0) - 1.0));
  nVec_1 = tmpvar_9;
  lowp vec3 tmpvar_10;
  tmpvar_10 = normalize(v_lightDir);
  lowp float tmpvar_11;
  tmpvar_11 = dot (tmpvar_9, tmpvar_10);
  lowp float tmpvar_12;
  tmpvar_12 = max (tmpvar_11, 0.0);
  lowp float t_13;
  t_13 = max (min ((((1.0 - tmpvar_12) - 0.7) / 0.2), 1.0), 0.0);
  gl_FragColor = ((u_ambientLight * diffuseMat_3) + ((((tmpvar_12 * u_diffuseLight) * diffuseMat_3) + (((t_13 * (t_13 * (3.0 - (2.0 * t_13)))) * u_diffuseLight) * nightMat_4)) + (((pow (max (dot (normalize(v_viewDir), (((2.0 * tmpvar_11) * tmpvar_9) - tmpvar_10)), 0.0), u_shininess) * u_specularLight) * specularMat_2) * diffuseMat_3)));
}

#endif
