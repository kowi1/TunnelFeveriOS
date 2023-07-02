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
    var orientationLockIPhone = UIInterfaceOrientationMask.portraitUpsideDown
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
        //if deviceType.lowercaseString.rangeOfString("iphone 4") != nil {
          //     print("iPhone 4 or iphone 4s")
            //}
            //else if deviceType.lowercaseString.rangeOfString("iphone 5") != nil {
              //  print("iPhone 5 or iphone 5s or iphone 5c")
            //}
           //else if deviceType.lowercaseString.rangeOfString("iphone 6") != nil {
             //   print("iPhone 6 Series")
           //  return orientationLockIPhone
           }

}
