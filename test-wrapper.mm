//
//  test-wrapper.m
//  TunnelFever
//
//  Created by admin on 2/12/23.
//

#import <Foundation/Foundation.h>
#import "test-wrapper.h"
#import <UIKit/UIKit.h>
#import "TunnelFever-Swift.h"
#include "Test.hpp"
#include "native_engine.hpp"
#include "swift_call.h"
//#include "input_util.hpp"


@implementation testWrapper

NativeEngine *_nativeEngine = new NativeEngine();
//PurchaseView *_puchaseView;
PurchaseView *_purchaseView;
OpenGLUIView *_opengluiView;

void ShowAd ()
{
  // testWrapper *testWrapperInstance = [[testWrapper alloc] init];
   // ViewController *myOb = [[ViewController alloc] init];
    
   // [_purchaseView buyConsumable];
    [_opengluiView showInterstitial];
    // Call the Objective-C method using Objective-C syntax
    //[(id) CFBridgingRelease(self) buyConsumable];
}
void BuyConsumableC ()
{
  // testWrapper *testWrapperInstance = [[testWrapper alloc] init];
   // ViewController *myOb = [[ViewController alloc] init];
    
    [_purchaseView buyConsumable];
   // [_opengluiView showInterstitial];
    // Call the Objective-C method using Objective-C syntax
    //[(id) CFBridgingRelease(self) buyConsumable];
}
-(void)  initializePurchase:(NSObject *)obj{
    
    _purchaseView=obj;
    
    
}
-(void)  initializeUIView:(NSObject *)uiobj{
    
    _opengluiView = uiobj;
    _nativeEngine->InitializeOpengLUIObject((__bridge objc_object *)uiobj);

}


-(NSString*) runTest {
    
    Test _test;
  //  PlayScene _scene;
    
    
    return  [NSString stringWithUTF8String: _test.run()];
    
    
}
-(void) renderFrame:(int)a and:(int)b {
    
    Test _test;
  //  PlayScene _scene;
    
    
     _test.renderFrame(a,b);
    
    
}
-(void)  setupGraphics{
    
    Test _test;
  //  PlayScene _scene;
    
    
     _test.setupGraphics();
    
    
}
-(void)  updateLife:(int)a{
    
    _nativeEngine->UpdateLife(a);
    
    
}
-(void)  nativeEngine:(int)width and:(int)height and:(NSString *)BundlePath  and:(NSString *)DocumentDataPath{
    
   
  //  PlayScene _scene;
    
   // myOb = [PurchaseView init];
    _nativeEngine->GameLoop(width,height,[BundlePath UTF8String],[DocumentDataPath UTF8String]);
    
    
}

-(void)  inputfunc:(int)posY and:(int)posX and:(int)width and:(int)height;{
    
    AInputEvent event;
    event.motionX=(posX);
    event.motionY=(posY);
    event.motionMinX=0.0f;
    event.motionMaxX=1.0f;
    event.motionMaxY=1.0f;
    event.motionMinY=0.0f;
    event.type=6;//"COOKED_EVENT_TYPE_JOY";
    event.motionIsOnScreen=true;
    // PlayScene _scene;
   // _nativeEngine->
    _nativeEngine->HandleInputProxy(&event);
        
}

@end


