//
//  GameBridge.h
//  TunnelFever
//
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "swift_call.h"

#ifndef GameBridge_h
#define GameBridge_h

@protocol SKPaymentTransactionObserver;
@protocol SKProductsRequestDelegate;
@protocol GADFullScreenContentDelegate;

@interface GameBridge: NSObject

-(void)initializePurchase:(NSObject *)obj;
-(void)initializeUIView:(NSObject *)uiobj;
-(void)updateLife:(int)a;
-(void)nativeEngine:(int)width and:(int)height and:(NSString *)BundlePath and:(NSString *)DocumentDataPath;
-(void)inputfunc:(int)posY and:(int)posX and:(int)width and:(int)height;

@end

#endif /* GameBridge_h */
