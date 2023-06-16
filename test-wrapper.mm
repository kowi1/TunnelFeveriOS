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
-(void)  nativeEngine{
    
   
  //  PlayScene _scene;
    
    
   _nativeEngine->GameLoop();
    
    
}

-(void)  inputfunc:(int)a and:(int)b {
    
    AInputEvent event;
    event.motionX=a/100;
    event.motionY=b/100;
   // event.type="COOKED_EVENT_TYPE_JOY";
    // PlayScene _scene;
   // _nativeEngine->
    _nativeEngine->HandleInputProxy(&event);
}
@end
