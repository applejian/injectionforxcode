//
//  $Id: //depot/InjectionPluginLite/Classes/BundleSweeper.h#14 $
//  Injection
//
//  Created by John Holdsworth on 12/11/2014.
//  Copyright (c) 2012 John Holdsworth. All rights reserved.
//
//  Client application interface to Code Injection system.
//  Added to program's main.(m|mm) to connect to the Injection app.
//
//  This file is copyright and may not be re-distributed, whole or in part.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
//  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//

#import <objc/runtime.h>
#import "IvarAccess.h"

#ifdef __IPHONE_OS_VERSION_MIN_REQUIRED
#ifndef XPROBE_BUNDLE
#import <UIKit/UIKit.h>

@interface CCDirector
+ (CCDirector *)sharedDirector;
@end
#endif
@implementation BundleInjection(BundleSeeds)

+ (NSArray *)bprobeSeeds {
    UIApplication *app = [UIApplication sharedApplication];
    NSMutableArray *seeds = [[app windows] mutableCopy];
    [seeds insertObject:app atIndex:0];

    // support for cocos2d
    Class ccDirectorClass = NSClassFromString(@"CCDirector");
    CCDirector *ccDirector = [ccDirectorClass sharedDirector];
    if ( ccDirector )
        [seeds addObject:ccDirector];
    return seeds;
}

@end
#else
#import <Cocoa/Cocoa.h>

@implementation BundleInjection(BundleSeeds)
+ (NSArray *)bprobeSeeds {
    NSApplication *app = [NSApplication sharedApplication];
    NSMutableArray *seeds = [[app windows] mutableCopy];
    if ( app.delegate )
        [seeds addObject:app.delegate];
    return seeds;
}
@end
#endif

@interface NSObject(BundleReferences)

// external references
- (NSArray *)getNSArray;
- (NSArray *)subviews;
- (id)contentView;
- (id)document;
- (id)delegate;
- (SEL)action;
- (id)target;

@end

@interface BundleInjection(Sweeper)
+ (NSMutableDictionary *)instancesSeen;
+ (NSMutableArray *)liveInstances;
@end

@implementation NSObject(BundleSweeper)

/*****************************************************
 ********* sweep and object display methods **********
 *****************************************************/

+ (void)bsweep {
}

- (void)bsweep {
//    bsweep( self );
//}
//
//static void bsweep( NSObject *self ) {
    id key = [NSValue valueWithPointer:(__bridge void *)self];
    Class bundleInjection = objc_getClass("BundleInjection");
    if ( [bundleInjection instancesSeen][key] )
        return;

    [bundleInjection instancesSeen][key] = @"1";

    Class aClass = object_getClass(self);
    NSString *className = NSStringFromClass(aClass);
    if ( [className characterAtIndex:1] == '_' || [className hasPrefix:@"UITransition"] )
        return;
    else
        [[bundleInjection liveInstances] addObject:self];

    //printf("BundleSweeper sweep <%s %p>\n", [className UTF8String], self);

    for ( ; aClass && aClass != [NSObject class] ; aClass = class_getSuperclass(aClass) ) {
        unsigned ic;
        Ivar *ivars = class_copyIvarList(aClass, &ic);
        const char *currentClassName = class_getName(aClass), firstChar = currentClassName[0];

        if ( firstChar != '_' && !(firstChar == 'N' && currentClassName[1] == 'S') )
            for ( unsigned i=0 ; i<ic ; i++ ) {
                __unused const char *currentIvarName = ivar_getName(ivars[i]);
                const char *type = ivar_getTypeEncodingSwift(ivars[i],aClass);
                if ( type && type[0] == '@' ) {
                    id subObject = xvalueForIvarType( self, ivars[i], type, aClass );
                    if ( [subObject respondsToSelector:@selector(bsweep)] )
                        [subObject bsweep];////( subObject );
                }
            }

        free( ivars );
    }

    if ( [self respondsToSelector:@selector(target)] )
        [[self target] bsweep];
    if ( [self respondsToSelector:@selector(delegate)] )
        [[self delegate] bsweep];
    if ( [self respondsToSelector:@selector(document)] )
        [[self document] bsweep];

    if ( [self respondsToSelector:@selector(contentView)] )
        [[[self contentView] superview] bsweep];
    if ( [self respondsToSelector:@selector(subviews)] )
        [[self subviews] bsweep];
    if ( [self respondsToSelector:@selector(getNSArray)] )
        [[self getNSArray] bsweep];
}

@end

@implementation NSArray(BundleSweeper)

- (void)bsweep {
    for ( id obj in self )
        if ( [obj respondsToSelector:@selector(bsweep)] )
            [obj bsweep];////( subObject );
}

@end

@implementation NSSet(BundleSweeper)

- (void)bsweep {
    [[self allObjects] bsweep];
}

@end

@implementation NSDictionary(BundleSweeper)

- (void)bsweep {
    [[self allValues] bsweep];
}

@end

@implementation NSMapTable(BundleSweeper)

- (void)bsweep {
    [[[self objectEnumerator] allObjects] bsweep];
}

@end

@implementation NSHashTable(BundleSweeper)

- (void)bsweep {
    [[self allObjects] bsweep];
}

@end

@implementation NSString(BundleSweeper)

- (void)bsweep {
}

@end

@implementation NSValue(BundleSweeper)

- (void)bsweep {
}

@end

@implementation NSData(BundleSweeper)

- (void)bsweep {
}

@end

@interface NSBlock : NSObject
@end

@implementation NSBlock(BundleSweeper)

- (void)bsweep {
}

@end

static NSMutableDictionary *instancesSeen;
static NSMutableArray *liveInstances;

@implementation BundleInjection(Sweeper)

+ (NSMutableDictionary *)instancesSeen {
    return instancesSeen;
}

+ (void)setInstancesSeen:(NSMutableDictionary *)dictionary {
    instancesSeen = dictionary;
}

+ (NSMutableArray *)liveInstances {
    return liveInstances;
}

+ (void)setLiveInstances:(NSMutableArray *)array {
    liveInstances = array;
}

+ (NSArray *)sweepForLiveObjects {
    Class bundleInjection = objc_getClass("BundleInjection");
    bundleInjection.instancesSeen = [NSMutableDictionary new];
    bundleInjection.liveInstances = [NSMutableArray new];

    //NSTimeInterval start = [NSDate timeIntervalSinceReferenceDate];
    [[self bprobeSeeds] bsweep];
    //NSLog( @"%f", [NSDate timeIntervalSinceReferenceDate]-start );

    NSArray *liveInstances = bundleInjection.liveInstances;
    bundleInjection.instancesSeen = nil;
    bundleInjection.liveInstances = nil;
    return liveInstances;
}

@end
