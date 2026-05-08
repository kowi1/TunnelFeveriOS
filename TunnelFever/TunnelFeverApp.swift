 //
//  TunnelFeverApp.swift
//  TunnelFever
//
//  Created by admin on 1/29/23.
//

import SwiftUI
import Firebase
@main
struct TunnelFeverApp: App {
    
@UIApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
   init() {
          FirebaseApp.configure() // Initialize Firebase
     }
var body: some Scene {
        WindowGroup {
            ContentView()
        }
    }
}

