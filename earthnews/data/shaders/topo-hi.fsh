//
//  topo-hi.fsh
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
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
  vec4 ambient = (u_ambientLight * diffuseMat);
  
  // diffuse
  float d = max (dotp, c_zero);
  float n = c_one - d;

#if 0
  n = smoothstep (0.63, c_one, n); /* nice but expensive */
#else
  //n = smoothstep (0.5, c_one, n); /* next best range */
  //n = clamp ((n - 0.5) / 0.5, c_zero, c_one); /* linear step */
  //n = clamp ((n - 0.5) * 2.0, c_zero, c_one);
  //n = clamp ((n + n) - c_one, c_zero, c_one); /* simply scale range [0,1] to range [-1,1] */
  
  n = (n + n) - c_one; /* n will always be less than one */
  n = max (n, c_zero);
#endif

  vec4 diffuse = ((d * diffuseMat) + (n * nightMat)) * u_diffuseLight;
  
  // phong model
  vec3 r = (c_two * dotp * nVec) - lVec;
#if 0
  float s = pow (max (dot (vVec, r), c_zero), u_shininess);
#else
  u_shininess; // unused
  float mp = max (dot (vVec, r), c_zero);
  float s = mp * mp; // shininess = 2
#endif

  vec4 specular = s * u_specularLight * specularMat * diffuseMat;
    
  gl_FragColor = ambient + diffuse + specular;
}

