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
 
    let mtestWrapper = testWrapper()
    var body: some View {
        
        OpenGLView(gestureLocation: $gestureLocation,mtestWrapper: mtestWrapper).frame(width:UIScreen.main.bounds.width, height:UIScreen.main.bounds.height)
            .onAppear {
                                OpenGLView.lockOrientation(.portrait) // Lock to portrait mode
                            }
                            .onDisappear {
                                OpenGLView.lockOrientation(.all) // Reset orientation lock
                            }
            .gesture(
            DragGesture()
                .onChanged { value in
                    gestureLocation = value.location
                    mtestWrapper.inputfunc(Int32(gestureLocation.x), and: Int32(gestureLocation.y), and:Int32(UIScreen.main.bounds.width),and:Int32(UIScreen.main.bounds.height))
                }
                .onEnded { _ in
        //           gestureLocation = .zero
                })
        
        
    }
    
}
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
