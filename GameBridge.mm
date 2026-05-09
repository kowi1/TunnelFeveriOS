//
//  GameBridge.mm
//  TunnelFever
//

#import <Foundation/Foundation.h>
#import "GameBridge.h"
#import <UIKit/UIKit.h>
#import "TunnelFever-Swift.h"
#include "native_engine.hpp"
#include "swift_call.h"

@implementation GameBridge

NativeEngine *_nativeEngine = new NativeEngine();
PurchaseView *_purchaseView;
OpenGLUIView *_opengluiView;

void ShowAd()
{
    [_opengluiView showInterstitial];
}

void BuyConsumableC()
{
    [_purchaseView buyConsumable];
}

-(void)initializePurchase:(NSObject *)obj {
    _purchaseView = obj;
}

-(void)initializeUIView:(NSObject *)uiobj {
    _opengluiView = uiobj;
    _nativeEngine->InitializeOpengLUIObject((__bridge objc_object *)uiobj);
}

-(void)updateLife:(int)a {
    _nativeEngine->UpdateLife(a);
}

-(void)nativeEngine:(int)width and:(int)height and:(NSString *)BundlePath and:(NSString *)DocumentDataPath {
    _nativeEngine->GameLoop(width, height, [BundlePath UTF8String], [DocumentDataPath UTF8String]);
}

-(void)inputfunc:(int)posY and:(int)posX and:(int)width and:(int)height {
    AInputEvent event;
    event.motionX = (posX);
    event.motionY = (posY);
    event.motionMinX = 0.0f;
    event.motionMaxX = 1.0f;
    event.motionMaxY = 1.0f;
    event.motionMinY = 0.0f;
    event.type = 6;
    event.motionIsOnScreen = true;
    _nativeEngine->HandleInputProxy(&event);
}

@end
