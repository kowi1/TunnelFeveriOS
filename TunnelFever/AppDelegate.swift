//
//  AppDelegate.swift
//  TunnelFever
//
//  Created by admin on 6/26/23.
//

import Foundation
import UIKit
import SwiftUI

class AppDelegate: UIResponder, UIApplicationDelegate {
      var window: UIWindow?
    var orientationLockIPhone = UIInterfaceOrientationMask.all
     var orientationLockIPad = UIInterfaceOrientationMask.all
   
    
    
    func application(_ application: UIApplication, supportedInterfaceOrientationsFor window: UIWindow?) -> UIInterfaceOrientationMask {
        
            
        let deviceType = UIDevice.modelName
        if deviceType.contains("iPhone"){
            return orientationLockIPhone
        }else if deviceType.contains("iPad"){
            return orientationLockIPad
        }else{
            return orientationLockIPhone
        }
           }

}
