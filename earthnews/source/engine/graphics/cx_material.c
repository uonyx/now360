//
//  cx_material.c
//
//  Created by Ubaka Onyechi on 26/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_material.h"
#include "cx_gdi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct cx_material_mapping
{
  cx_shader_uniform uniform;
  cx_material_property property;
  GLenum openglTextureUnit;
};

static struct cx_material_mapping s_mappings [CX_NUM_MATERIAL_TEXTURES] = 
{
  { CX_SHADER_UNIFORM_SAMPLER2D_0, CX_MATERIAL_PROPERTY_AMBIENT,  GL_TEXTURE0 },
  { CX_SHADER_UNIFORM_SAMPLER2D_1, CX_MATERIAL_PROPERTY_DIFFUSE,  GL_TEXTURE1 },
  { CX_SHADER_UNIFORM_SAMPLER2D_2, CX_MATERIAL_PROPERTY_BUMP,     GL_TEXTURE2 },
  { CX_SHADER_UNIFORM_SAMPLER2D_3, CX_MATERIAL_PROPERTY_SPECULAR, GL_TEXTURE3 }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_material *cx_material_create (const char *name)
{
  cx_material *material = (cx_material *) cx_malloc (sizeof (cx_material));
  
#if CX_MATERIAL_DEBUG
  material->name        = cx_strdup (name, strlen (name));
#endif
  
  material->ambient     = *cx_colour_white ();
  material->diffuse     = *cx_colour_white ();
  material->specular    = *cx_colour_white ();
  material->emmissive   = *cx_colour_white ();
  material->shininess   = 1.0f;
  material->twoSided    = false;
  material->alpha       = false;
  material->properties  = CX_MATERIAL_PROPERTY_NONE;
  
  for (int i = 0; i < CX_NUM_MATERIAL_TEXTURES; i++)
  {
    material->textures [i] = NULL;
  }
  
  return material;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_destroy (cx_material *material)
{
  CX_ASSERT (material);
  
#if CX_MATERIAL_DEBUG
  cx_free (material->name);
#endif
  
  cx_free (material);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_set_properties (cx_material *material, cx_material_property properties)
{
  material->properties = properties;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_render (const cx_material *material, const cx_shader *shader)
{
  CX_ASSERT (material);
  CX_ASSERT (shader);
  
  GLenum textureUnit;
  cx_shader_uniform uniform;
  cx_material_property property;
  const cx_texture *texture = NULL;
  
  for (int i = 0; i < CX_NUM_MATERIAL_TEXTURES; i++)
  {
    property    = s_mappings [i].property;
    uniform     = s_mappings [i].uniform;
    textureUnit = s_mappings [i].openglTextureUnit;
    
    if ((material->properties & property) == property)
    {
      texture = material->textures [i];
      CX_ASSERT (texture);
      
      glActiveTexture (textureUnit);
      cx_gdi_assert_no_errors ();
      
      glBindTexture (GL_TEXTURE_2D, texture->id);
      cx_gdi_assert_no_errors ();
      
      glUniform1i (shader->uniforms [uniform], i);
      cx_gdi_assert_no_errors ();
    } 
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_render_texture (const cx_texture *texture, cx_material_texture type, const cx_shader *shader)
{
  CX_ASSERT (texture);
  CX_ASSERT (shader);

  GLenum textureUnit         = s_mappings [type].openglTextureUnit;
  cx_shader_uniform uniform  = s_mappings [type].uniform;
  
  glActiveTexture (textureUnit);
  cx_gdi_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_gdi_assert_no_errors ();
  
  glUniform1i (shader->uniforms [uniform], type);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_attach_texture (cx_material *material, const cx_texture *texture, cx_material_texture type)
{
  CX_ASSERT (material);
  CX_ASSERT (texture);
  CX_ASSERT ((type >= 0) && (type < CX_NUM_MATERIAL_TEXTURES));
  
  material->textures [type] = texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_detach_texture (cx_material *material, cx_material_texture type)
{
  //const cx_texture *texture = material->textures [type];
  //CX_ASSERT (texture);
  
  material->textures [type] = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////