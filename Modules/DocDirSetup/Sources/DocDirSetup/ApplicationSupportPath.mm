#include "FileSystem/FileSystem.h"

#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)

#import <Foundation/NSString.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSPathUtilities.h>

namespace DAVA
{
FilePath GetApplicationSupportPath()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString* bundlePath = [paths objectAtIndex:0];
    NSString* filePath = [bundlePath stringByAppendingString:@"/DAVAEngine/"];
    return FilePath(String([filePath cStringUsingEncoding:NSUTF8StringEncoding]));
}
}

#endif //#if defined(__DAVAENGINE_MACOS__) || defined(__DAVAENGINE_IPHONE__)
