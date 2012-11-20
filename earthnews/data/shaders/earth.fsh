//
//  earth.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

// source light colours
uniform mediump vec4 u_ambientLight;
uniform mediump vec4 u_diffuseLight;
uniform mediump vec4 u_specularLight;
uniform mediump float u_shininess;

uniform sampler2D u_normalMap;
uniform sampler2D u_diffuseMap;
uniform sampler2D u_glossMap;
uniform sampler2D u_nightMap;

varying mediump vec2 v_texcoord;
varying mediump vec3 v_viewDir;
varying mediump vec3 v_lightDir;

const mediump float c_zero = 0.0;

void main (void)
{
  mediump vec4 nightMat = texture2D (u_nightMap, v_texcoord);
  mediump vec4 diffuseMat = texture2D (u_diffuseMap, v_texcoord);
  mediump vec4 specularMat = texture2D (u_glossMap, v_texcoord);
  
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
  mediump vec4 ambient = (u_ambientLight * diffuseMat);// + (nightMat * u_ambientLight);
  
  // diffuse
  mediump float d = max (dotp, c_zero);
  //mediump vec4 diffuse = d * u_diffuseLight * diffuseMat;
  mediump vec4 diffuse = (d * u_diffuseLight * diffuseMat) + (nightMat * (u_diffuseLight - (d * u_diffuseLight)));
  
  // specular
  mediump vec4 specular = vec4 (c_zero);
  
  if (dotp > c_zero)
  {
    #if 0
    //mediump vec3 r = reflect (lVec, nVec);
    mediump vec3 r = (2.0 * dotp * nVec) - lVec;
    mediump float s = pow (max (dot (vVec, r), c_zero), u_shininess);
    specular = s * u_specularLight * specularMat;
    #else
    // blinn model
    mediump vec3 h = normalize (vVec + lVec);
    mediump float s = pow (max (dot (nVec, h), c_zero), u_shininess);
    specular = s * u_specularLight * (specularMat * diffuseMat);
  
    #endif
  }
  
  /*
  lowp vec4 diffuseColour = texture2D (u_diffuseMap, v_texcoord) * v_colour;
  lowp vec4 nightColour = texture2D (u_nightMap, v_texcoord) * (white - v_colour);  
  gl_FragColor = diffuseColour + nightColour;
  */
  
  gl_FragColor = (ambient + diffuse + specular);
}