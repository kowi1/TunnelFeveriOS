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
            .onTouch(perform: updateLocation)
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
