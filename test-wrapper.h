//
//  test-wrapper.h
//  TunnelFever
//
//  Created by admin on 2/12/23.
//
#import <Foundation/Foundation.h>

#ifndef test_wrapper_h
#define test_wrapper_h

@interface testWrapper: NSObject
-(NSString*) runTest;
-(void)renderFrame:(int)a and:(int)b;
-(void)setupGraphics;
-(void)nativeEngine:(int)a and:(int)b and:(NSString *)c;
-(void)inputfunc:(int)a and:(int)b and:(int)width and:(int)height;
@end

#endif /* test_wrapper_h */
