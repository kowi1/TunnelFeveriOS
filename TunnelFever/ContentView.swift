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
    var body: some View {
        // Text(_test.runTest())
        //    .padding()
        OpenGLView(gestureLocation: $gestureLocation).frame(width:630, height:630).gesture(
            DragGesture()
                .onChanged { value in
                    gestureLocation = value.location
                }
                .onEnded { _ in
               //    gestureLocation = .zero
                })
        
        
    }
    
}
struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
