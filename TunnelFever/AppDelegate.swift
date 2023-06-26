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
      var orientationLock = UIInterfaceOrientationMask.portrait
    
   

    func application(_ application: UIApplication, supportedInterfaceOrientationsFor window: UIWindow?) -> UIInterfaceOrientationMask {
            return orientationLock
        }

}
