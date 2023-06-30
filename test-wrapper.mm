//
//  test-wrapper.m
//  TunnelFever
//
//  Created by admin on 2/12/23.
//

#import <Foundation/Foundation.h>
#import "test-wrapper.h"
#include "Test.hpp"
#include "native_engine.hpp"
//#include "input_util.hpp"


@implementation testWrapper

NativeEngine *_nativeEngine = new NativeEngine();

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
-(void)  nativeEngine:(int)height and:(int)width{
    
   
  //  PlayScene _scene;
    
    
   _nativeEngine->GameLoop(height,width);
    
    
}

-(void)  inputfunc:(int)a and:(int)b and:(int)width and:(int)height;{
    
    AInputEvent event;
    event.motionX=(b);
    event.motionY=(a);
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
