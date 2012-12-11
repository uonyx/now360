//
//  Shader.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

#if 0
uniform sampler2D u_diffuseMap;

varying lowp vec4 v_colour;
varying mediump vec2 v_texcoord;

void main (void)
{
  mediump vec4 fix = vec4 (0.8, 0.8, 0.8, 0.55);
  
  mediump vec4 colour = texture2D (u_diffuseMap, v_texcoord) * v_colour;

  colour *= fix;
  
  gl_FragColor = colour;
}

#else

uniform mediump vec4 u_ambientLight;
uniform mediump vec4 u_diffuseLight;
uniform mediump vec4 u_specularLight;
uniform mediump float u_shininess;

uniform sampler2D u_normalMap;
uniform sampler2D u_diffuseMap;

varying mediump vec2 v_texcoord;
varying mediump vec3 v_viewDir;
varying mediump vec3 v_lightDir;

const mediump float c_zero = 0.0;

void main (void)
{
  mediump vec4 diffuseMat = texture2D (u_diffuseMap, v_texcoord);
  
  // get tangent space normal from normal maps
  mediump vec3 nVec = texture2D (u_normalMap, v_texcoord).xyz;
  
  // rgb range [0, 1] to [-1, 1], and normalize
  nVec = normalize ((nVec * 2.0) - 1.0);
  
  // view vector
  mediump vec3 vVec = normalize (v_viewDir);
  
  // light vector
  mediump vec3 lVec = normalize (v_lightDir);
  
  // n.l
  mediump float dotp = dot (nVec, lVec);
  
  // ambient
  mediump vec4 ambient = (u_ambientLight * diffuseMat);
  
  // diffuse
  mediump float d = max (dotp, c_zero);
  mediump vec4 diffuse = d * u_diffuseLight * diffuseMat;
  
  // specular
#if 1
  mediump vec3 r = (2.0 * dotp * nVec) - lVec;
  mediump float s = pow (max (dot (vVec, r), c_zero), u_shininess);
  mediump vec4 specular = s * u_specularLight * diffuseMat;
#else
  // blinn model
  mediump vec3 h = normalize (vVec + lVec);
  mediump float s = pow (max (dot (nVec, h), c_zero), u_shininess);
  mediump vec4 specular = s * u_specularLight * diffuseMat;
#endif

  mediump vec4 colour = ambient + (diffuse + specular);
  
  colour = diffuse;
  colour.a *= 0.5;
  
  gl_FragColor = colour;
}

#endif