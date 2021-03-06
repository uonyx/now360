//
//  cx_native_ios.m
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#import "cx_native_ios.h"
#import "cx_string.h"
#import <Foundation/Foundation.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_file_get_resource_path (char *dstPath, cxu32 dstSize)
{
  CX_ASSERT (dstPath);
  
  @autoreleasepool 
  {
    NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
    
    const char *cFilePath = [resourcePath cStringUsingEncoding:NSASCIIStringEncoding];
    
    cx_strcpy (dstPath, dstSize, cFilePath);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_file_get_cache_path (char *dstPath, cxu32 dstSize)
{
  CX_ASSERT (dstPath);
  
  @autoreleasepool 
  {
    NSArray *paths = NSSearchPathForDirectoriesInDomains (NSCachesDirectory, NSUserDomainMask, YES);
    
    NSString *cachePath = [paths objectAtIndex:0];
    
    const char *cFilePath = [cachePath cStringUsingEncoding:NSASCIIStringEncoding];
    
    cx_strcpy (dstPath, dstSize, cFilePath);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_file_get_documents_path (char *dstPath, cxu32 dstSize)
{
  CX_ASSERT (dstPath);
  
  @autoreleasepool 
  {
    NSArray *paths = NSSearchPathForDirectoriesInDomains (NSDocumentDirectory, NSUserDomainMask, YES);
    
    NSString *documentsPath = [paths objectAtIndex:0];
    
    const char *cFilePath = [documentsPath cStringUsingEncoding:NSASCIIStringEncoding];
    
    cx_strcpy (dstPath, dstSize, cFilePath);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// gl context
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static EAGLContext *g_eaglContext = nil;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_eagl_context_init (void *eaglContext)
{
  CX_ASSERT (eaglContext);
  
  g_eaglContext = eaglContext;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_eagl_context_deinit (void)
{
  g_eaglContext = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_eagl_context_add (void)
{
  CX_ASSERT (g_eaglContext);
  
  EAGLContext *newContext = [[[EAGLContext alloc] initWithAPI:[g_eaglContext API]
                                                   sharegroup:[g_eaglContext sharegroup]] autorelease];
#if CX_DEBUG
  EAGLContext *curContext = [EAGLContext currentContext];
  CX_REF_UNUSED (curContext);
#endif
  
  [EAGLContext setCurrentContext:newContext];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_eagl_context_remove (void)
{
#if CX_DEBUG
  EAGLContext *curContext = [EAGLContext currentContext];
  CX_REF_UNUSED (curContext);
#endif
 
  [EAGLContext setCurrentContext:nil];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
#import "../graphics/cx_texture.h"
static bool cx_native_load_png (const char *filename, cx_texture *texture)
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
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
