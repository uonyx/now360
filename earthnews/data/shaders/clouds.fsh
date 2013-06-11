//
//  Shader.fsh
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

#if 1
uniform sampler2D u_diffuseMap;

varying lowp vec4 v_colour;
varying lowp vec2 v_texcoord;

void main (void)
{
  //lowp vec4 colour = texture2D (u_diffuseMap, v_texcoord) * v_colour;
  //lowp vec4 fix = vec4 (0.8, 0.8, 0.8, 0.55);
  //colour *= fix;
  
  lowp vec4 colour = texture2D (u_diffuseMap, v_texcoord);

  lowp float alpha = colour.r;
  
  colour.rgb *= v_colour.rgb;
  
  colour.a *= alpha;
  
  gl_FragColor = colour;
}

#else

uniform lowp vec4 u_ambientLight;
uniform lowp vec4 u_diffuseLight;
uniform lowp vec4 u_specularLight;
uniform lowp float u_shininess;

uniform sampler2D u_normalMap;
uniform sampler2D u_diffuseMap;

varying lowp vec2 v_texcoord;
varying lowp vec3 v_viewDir;
varying lowp vec3 v_lightDir;

const lowp float c_zero = 0.0;

void main (void)
{
  lowp vec4 diffuseMat = texture2D (u_diffuseMap, v_texcoord);
  
  // get tangent space normal from normal maps
  lowp vec3 nVec = texture2D (u_normalMap, v_texcoord).xyz;
  
  // rgb range [0, 1] to [-1, 1], and normalize
  nVec = normalize ((nVec * 2.0) - 1.0);
  
  // view vector
  lowp vec3 vVec = normalize (v_viewDir);
  
  // light vector
  lowp vec3 lVec = normalize (v_lightDir);
  
  // n.l
  lowp float dotp = dot (nVec, lVec);
  
  // ambient
  lowp vec4 ambient = (u_ambientLight * diffuseMat);
  
  // diffuse
  lowp float d = max (dotp, c_zero);
  lowp vec4 diffuse = d * u_diffuseLight * diffuseMat;
  
  // specular
#if 1
  lowp vec3 r = (2.0 * dotp * nVec) - lVec;
  lowp float s = pow (max (dot (vVec, r), c_zero), u_shininess);
  lowp vec4 specular = s * u_specularLight * diffuseMat;
#else
  // blinn model
  lowp vec3 h = normalize (vVec + lVec);
  lowp float s = pow (max (dot (nVec, h), c_zero), u_shininess);
  lowp vec4 specular = s * u_specularLight * diffuseMat;
#endif

  lowp vec4 colour = ambient + (diffuse + specular);
  
  colour = diffuse;
  colour.a *= 0.5;
  
  gl_FragColor = colour;
}

#endif