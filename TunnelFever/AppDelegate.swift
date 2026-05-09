//
//  AppDelegate.swift
//  TunnelFever
//
//  Created by admin on 6/26/23.
//

import Foundation
import UIKit
import SwiftUI
import AVFoundation

class AppDelegate: UIResponder, UIApplicationDelegate {
      var window: UIWindow?
     var orientationLock = UIInterfaceOrientationMask.landscape

    func application(_ application: UIApplication,
                     didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        setupAudioSession()
        return true
    }

    private func setupAudioSession() {
        let session = AVAudioSession.sharedInstance()
        do {
            // .ambient: respects the silent switch and mixes with background
            // music (e.g. user's own playlist).  Use .playback if you want
            // sounds to ignore the silent switch.
            try session.setCategory(.ambient, mode: .default)
            try session.setActive(true)
        } catch {
            // Non-fatal — game still works without audio
        }

        // Resume the session after an interruption (phone call, Siri, etc.)
        NotificationCenter.default.addObserver(
            self,
            selector: #selector(handleAudioInterruption(_:)),
            name: AVAudioSession.interruptionNotification,
            object: session
        )
    }

    @objc private func handleAudioInterruption(_ notification: Notification) {
        guard
            let info = notification.userInfo,
            let typeValue = info[AVAudioSessionInterruptionTypeKey] as? UInt,
            let type = AVAudioSession.InterruptionType(rawValue: typeValue)
        else { return }

        if type == .ended {
            let options = info[AVAudioSessionInterruptionOptionKey] as? UInt ?? 0
            if AVAudioSession.InterruptionOptions(rawValue: options).contains(.shouldResume) {
                try? AVAudioSession.sharedInstance().setActive(true)
            }
        }
    }
     
   
    
    
    func application(_ application: UIApplication, supportedInterfaceOrientationsFor window: UIWindow?) -> UIInterfaceOrientationMask {
            return self.orientationLock
           }
    
    struct AppUtility {
        static func lockOrientation(_ orientation: UIInterfaceOrientationMask) {
            if let delegate = UIApplication.shared.delegate as? AppDelegate {
                delegate.orientationLock = orientation
                
            }
        }
            
        static func lockOrientation(_ orientation: UIInterfaceOrientationMask, andRotateTo rotateOrientation:UIInterfaceOrientation) {
            
                self.lockOrientation(orientation)
                UIDevice.current.setValue(rotateOrientation.rawValue, forKey: "orientation")
                UINavigationController.attemptRotationToDeviceOrientation()
            }
           
        
    }

}
