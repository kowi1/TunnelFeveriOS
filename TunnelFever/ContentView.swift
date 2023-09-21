//
//  ContentView.swift
//  TunnelFever
//
//  Created by admin on 1/29/23.
//

import SwiftUI

struct ContentView: View {
    @State private var gestureLocation: CGPoint = .zero
    @State private var tapGestureState: CGPoint = .zero
    @State private var orientation = UIDevice.current.orientation
    
    
    let mtestWrapper = testWrapper()
    @State var isButtonHidden:Bool = true
        let button = InterstialVC()
 
    var body: some View {
       
               
       if (UIScreen.main.bounds.width<UIScreen.main.bounds.height){
            ZStack{
                //BannerVC().
                Group {
                            if isButtonHidden {
                              //  button
                            } else {
                               button
                            }
                }.onChange(of: isButtonHidden){value in
                    
                    if !value
                     {                            InterstialVC()
                        isButtonHidden=true
                    }
                }
               /* .onAppear {
                            DispatchQueue.main.asyncAfter(deadline: .now() + 10.5) {
                                self.isButtonHidden = false
                            }
                        }*/
               
                OpenGLView(gestureLocation: $gestureLocation,isButtonHidden:$isButtonHidden,mtestWrapper: mtestWrapper).frame(width:UIScreen.main.bounds.width, height:UIScreen.main.bounds.height)
                    .onTouch(perform: updateLocation)
                    .rotationEffect(.degrees(-90))
                    .scaleEffect(1.0)
                    .scaledToFit()
                    .offset(y: 5.5)
               
                
            }
            
             //   .onAppear {
              //         AppDelegate.AppUtility.lockOrientation(UIInterfaceOrientationMask.landscape, andRotateTo: UIInterfaceOrientation.landscapeLeft)
            //       }
                
             
        }
        else{
            ZStack{
              
               /* .onAppear {
                            DispatchQueue.main.asyncAfter(deadline: .now() + 10.5) {
                                self.isButtonHidden = false
                            }
                        }*/
         
                Group {
                            if isButtonHidden {
                               // button
                            } else {
                                button
                            }
                        }.onChange(of: isButtonHidden){value in
                            
                           if !value
                            {                         //  InterstialVC()
                               isButtonHidden=true
                           }
                                            
                        }
            }
                OpenGLView(gestureLocation: $gestureLocation,isButtonHidden:$isButtonHidden,mtestWrapper: mtestWrapper).frame(width:UIScreen.main.bounds.height, height:UIScreen.main.bounds.width)
                    .onTouch(perform: updateLocation)
                    .rotationEffect(.degrees(-90))
                    .scaleEffect(1.0)
                    .scaledToFit()
                    .offset(y: 5.5)
            
               
              //  .onAppear {
              //         //AppDelegate.AppUtility.lockOrientation(UIInterfaceOrientationMask.landscape, //andRotateTo: UIInterfaceOrientation.landscapeLeft)
            //       }
        }
           //
     
            
    }
    func updateLocation(_ location: CGPoint) {
        mtestWrapper.inputfunc(Int32(location.x), and: Int32(location.y), and:Int32(UIScreen.main.bounds.width),and:Int32(UIScreen.main.bounds.height))
       }
    
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
