//
//  ContentView.swift
//  Test
//
//  Created by admin on 6/3/23.
//

import SwiftUI

struct ContentView: View {
    
    @State private var gestureLocation: CGPoint = .zero
    var body: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundColor(.accentColor)
                .gesture(
                    DragGesture()
                        .onChanged { value in
                            gestureLocation = value.location
                        }
                        .onEnded { _ in
                            gestureLocation = .zero
                        }
                )
            
            Text("Hello, world!")
                .gesture(
                    TapGesture()
                        .onEnded {
                            gestureLocation = .zero
                        }
                )
        }
        .padding()
        .overlay(
            Circle()
                .fill(Color.red)
                .frame(width: 30, height: 30)
                .position(gestureLocation)
        )
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
