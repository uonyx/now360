//
//  cx_texture.m
//
//  Created by Ubaka Onyechi on 19/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "cx_system.h"
#import "cx_string.h"
#import "../graphics/cx_texture.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_get_filepath_from_resource (const char *filename, char *destFilepath, int destFilePathSize);
bool cx_native_load_png (const char *filename, cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_get_filepath_from_resource (const char *filename, char *destFilepath, int destFilePathSize)
{
  NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
  
  NSString *filePath = [resourcePath stringByAppendingPathComponent:[NSString stringWithCString:filename encoding:NSASCIIStringEncoding]];
  
  const char *cFilePath = [filePath cStringUsingEncoding:NSASCIIStringEncoding];
  
  cx_strcpy (destFilepath, destFilePathSize, cFilePath);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_native_load_png (const char *filename, cx_texture *texture)
{
#if 1
  char fullfilePath [512];
  
  cx_native_get_filepath_from_resource (filename, fullfilePath, 512);
  
  NSString *path = [NSString stringWithCString:fullfilePath encoding:NSASCIIStringEncoding];
  
  NSData *pngData = [[NSData alloc] initWithContentsOfFile:path];
  
  UIImage *image = [[UIImage alloc] initWithData:pngData];
  
  CX_ASSERT (image);
  
  texture->width = CGImageGetWidth (image.CGImage);
  texture->height = CGImageGetHeight (image.CGImage);
  texture->dataSize = texture->width * texture->height * 4 * sizeof (cxu8);
  texture->data = cx_malloc (texture->dataSize);
  
  int bpp = CGImageGetBitsPerPixel (image.CGImage);
  
  if (bpp == 24)
  {
    texture->format = CX_TEXTURE_FORMAT_RGB;
  }
  else
  {
    texture->format = CX_TEXTURE_FORMAT_RGBA;
  }
  
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB ();
  
  CGContextRef context = CGBitmapContextCreate (texture->data, texture->width, texture->height, 8, texture->width * 4, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big );
  CGColorSpaceRelease (colorSpace);
  
  CGContextClearRect (context, CGRectMake (0, 0, texture->width, texture->height));
  CGContextTranslateCTM (context, 0, 0);
  
  CGContextDrawImage (context, CGRectMake (0, 0, texture->width, texture->height), image.CGImage);
  CGContextRelease (context);
	
	[image release];
  [pngData release];
  
#else
  
  CGImageRef spriteImage;
	CGContextRef spriteContext;
	//GLubyte *spriteData;
	//size_t	width = 1, height=1 , bpp=0 , bytePerRow = 0;
	
	NSString* name = [[NSString alloc] initWithCString:filename encoding:NSASCIIStringEncoding];

	spriteImage = [UIImage imageNamed:name].CGImage;
	
	CX_ASSERT (spriteImage);

  texture->width = CGImageGetWidth(spriteImage);
  texture->height = CGImageGetHeight(spriteImage);
  texture->dataSize = texture->width * texture->height * 4 * sizeof (cxu8);
  texture->data = (cxu8 *)calloc(texture->width * texture->height * 4,sizeof(cxu8));
  
  int bpp = CGImageGetBitsPerPixel (spriteImage);
  
  if (bpp == 24)
  {
    texture->format = CX_TEXTURE_FORMAT_RGB;
    //NSLog(@"TEXTURE_GL_RGB, bpp=%d ",text->bpp);
    spriteContext = CGBitmapContextCreate(texture->data, texture->width, texture->height, 8, texture->width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaNoneSkipLast);
  }
  else 
  {
    texture->format = CX_TEXTURE_FORMAT_RGBA;		
    //NSLog(@"TEXTURE_GL_RGBA, bpp=%d  ",text->bpp);
    spriteContext = CGBitmapContextCreate(texture->data, texture->width, texture->height, 8, texture->width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);
  }
  
  CGContextDrawImage(spriteContext, CGRectMake(0.0, 0.0, (CGFloat)texture->width, (CGFloat)texture->height), spriteImage);
  CGContextRelease(spriteContext);
	
	[name release];
#endif
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
