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
     var orientationLockIPhone = UIInterfaceOrientationMask.landscape
     var orientationLockIPad = UIInterfaceOrientationMask.landscape
   
    
    
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
    struct AppUtility {
        static func lockOrientation(_ orientation: UIInterfaceOrientationMask) {
            if let delegate = UIApplication.shared.delegate as? AppDelegate {
                delegate.orientationLockIPad = orientation
                delegate.orientationLockIPhone = orientation
            }
        }
            
        static func lockOrientation(_ orientation: UIInterfaceOrientationMask, andRotateTo rotateOrientation:UIInterfaceOrientation) {
            let deviceType = UIDevice.modelName
            if deviceType.contains("iPhone"){
                self.lockOrientation(UIInterfaceOrientationMask.landscape)
                UIDevice.current.setValue(UIInterfaceOrientationMask.landscape, forKey: "orientation")
            }else if deviceType.contains("iPad"){
                self.lockOrientation(UIInterfaceOrientationMask.landscape)
                UIDevice.current.setValue(UIInterfaceOrientationMask.landscape, forKey: "orientation")
            }else{
                self.lockOrientation(UIInterfaceOrientationMask.landscape)
                UIDevice.current.setValue(UIInterfaceOrientationMask.landscape, forKey: "orientation")
            }
           
        }
    }

}
