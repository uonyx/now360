//
//  earth.vsh
//  testGraphics
//
//  Created by Ubaka Onyechi on 26/12/2011.
//  Copyright (c) 2011 uonyechi.com. All rights reserved.
//

// attributes
attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec3 a_tangent;
attribute vec3 a_bitangent;
attribute vec2 a_texcoord;

// uniforms
uniform mat4 u_mvpMatrix;

uniform mediump vec4 u_lightPosition;
uniform mediump vec4 u_lightColour;
uniform mediump vec4 u_eyePosition;

uniform lowp vec4 u_emissive;
uniform lowp vec4 u_ambient;
uniform lowp vec4 u_diffuse;
uniform lowp vec4 u_specular;

uniform lowp vec4 u_globalAmbient;
uniform float u_shininess;

// varyings
varying lowp vec4 v_colour;
varying mediump vec2 v_texcoord;

const float c_zero = 0.0;
const float c_one = 1.0;

void main (void)
{
  vec3 eyeNormal = normalize (u_normalMatrix * a_normal);
  vec3 lightPosition = vec3 (c_zero, c_zero, c_one);
  vec4 diffuseColor = vec4 (c_one, c_one, c_one, c_one);
    
  float nDotVP = max(c_zero, dot(eyeNormal, normalize (lightPosition)));
                 
  v_colour = diffuseColor * nDotVP;
  
  
  mediump vec4 position = u_mvpMatrix * a_position;
  mediump vec3 nVec = normalize (a_normal.xyz);
  mediump vec3 vVec = normalize (u_eyePosition - a_position);
  mediump vec3 lVec = normalize (u_lightPosition - a_position);
  
  
  mediump float dotp = dot (nVec, lVec);
  
  // emmisive
  
  mediump vec4 ke = u_emissive;
  
  // ambient
  
  mediump vec4 ka = u_ambient * u_globalAmbient;
  
  // diffuse
  
  mediump float d = max (dotp, c_zero);
  mediump vec4 kd = d * u_lightColour * u_diffuse;
  
  // specular
  
  mediump vec4 ks = vec4 (0.0);
  
  if (dop > c_zero)
  {
#if 1
    //mediump vec3 r = reflect (lVec, nVec);
    mediump vec3 r = (2.0 * dotp * nVec) - lVec;
    mediump float s = pow (max (dot (vVec, r), c_zero), u_shininess);
    ks = s * u_lightColour * u_specular;
#else
    // blinn model
    mediump vec3 h = normalize (vVec + lVec);
    mediump float s = pow (max (dot (nVec, h), c_zero), u_shininess);
    ks = s * u_lightColour * u_specular;
#endif
  }
  
  // attenuation (for point lights
  //mediump vec4 att = vec4 (1.0);
  //v_colour = ke + ka + att * (kd + ks);
  
  v_colour = ke + (ka + kd + ks);
  v_texcoord = a_texcoord;
  
  gl_Position = position;
}
