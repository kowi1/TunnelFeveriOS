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
    var body: some View {
        if (UIScreen.main.bounds.width<UIScreen.main.bounds.height){
            OpenGLView(gestureLocation: $gestureLocation,mtestWrapper: mtestWrapper).frame(width:UIScreen.main.bounds.width, height:UIScreen.main.bounds.height)
                .onTouch(perform: updateLocation)
                .rotationEffect(.degrees(-90))
                .scaleEffect(1.0)
                .onAppear {
                       AppDelegate.AppUtility.lockOrientation(UIInterfaceOrientationMask.landscape, andRotateTo: UIInterfaceOrientation.landscapeLeft)
                   }
                
             
        }
        else{
            OpenGLView(gestureLocation: $gestureLocation,mtestWrapper: mtestWrapper).frame(width:UIScreen.main.bounds.height, height:UIScreen.main.bounds.width)
                .onTouch(perform: updateLocation)
                .rotationEffect(.degrees(-90))
                .scaleEffect(1.0)
                .onAppear {
                       AppDelegate.AppUtility.lockOrientation(UIInterfaceOrientationMask.landscape, andRotateTo: UIInterfaceOrientation.landscapeLeft)
                   }
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
