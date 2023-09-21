//
//  test-wrapper.h
//  TunnelFever
//
//  Created by admin on 2/12/23.
//
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "swift_call.h"


#ifndef test_wrapper_h
#define test_wrapper_h

@protocol SKPaymentTransactionObserver;
@protocol SKProductsRequestDelegate;
@protocol GADFullScreenContentDelegate;

@interface testWrapper: NSObject



-(NSString*) runTest;
-(void)initializePurchase:(NSObject *)obj;
-(void)initializeUIView:(NSObject *)uiobj;
-(void)renderFrame:(int)a and:(int)b;
-(void)updateLife:(int)a;
-(void)setupGraphics;
-(void)nativeEngine:(int)width and:(int)height and:(NSString *)BundlePath and:(NSString *)DocumentDataPath;
-(void)inputfunc:(int)posY and:(int)posX and:(int)width and:(int)height;
@end

#endif /* test_wrapper_h */





