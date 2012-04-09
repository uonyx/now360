//
//  Shader.fsh
//  testGraphics
//
//  Created by Ubaka  Onyechi on 26/12/2011.
//  Copyright (c) 2011 SonOfLagos. All rights reserved.
//

varying lowp vec4 colorVarying;
varying mediump vec2 texCoordVarying;

uniform sampler2D sampler;

void main()
{
  //gl_FragColor = colorVarying;
  //gl_FragColor = texture2D (sampler, texCoordVarying) * colorVarying;
  gl_FragColor = texture2D (sampler, texCoordVarying);
}
