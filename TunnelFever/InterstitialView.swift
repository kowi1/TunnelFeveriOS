//
//  InterstitialView.swift
//  TunnelFever
//
//  Created by admin on 7/28/23.
//

import Foundation
import SwiftUI
import GoogleMobileAds
import UIKit
    

struct InterstialVC: UIViewControllerRepresentable  {
    
    typealias UIViewControllerType = UIViewController
    
    func makeUIViewController(context: Context) -> UIViewController {
         let view = ViewController()
        return view
    }

    func updateUIViewController(_ uiViewController: UIViewController, context: Context) {}
}


class ViewController:UIViewController, GADFullScreenContentDelegate {

   private var interstitial: GAMInterstitialAd?
    
    
  /*  convenience init() {
        self.init(nibName:nil, bundle:nil)
    }

    // This extends the superclass.
    override init(nibName nibNameOrNil: String?, bundle nibBundleOrNil: Bundle?) {
        super.init(nibName: nibNameOrNil, bundle: nibBundleOrNil)
        let request = GAMRequest()
        GAMInterstitialAd.load(withAdManagerAdUnitID: "ca-app-pub-4370884607853019/3198503939",
                                             request: request)
          { ad, error in
                      if error != nil { return }
                      self.interstitial = ad
              self.interstitial?.fullScreenContentDelegate = self
              self.interstitial?.present(fromRootViewController: self)
                  }
      //  tap = UITapGestureRecognizer(target: self, action: Selector("handleTap:"))
    }

    // This is also necessary when extending the superclass.
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented") // or see Roman Sausarnes's answer
    }
    
   /* override init() {
        let request = GAMRequest()
        GAMInterstitialAd.load(withAdManagerAdUnitID: "ca-app-pub-4370884607853019/3198503939",
                                             request: request)
          { ad, error in
                      if error != nil { return }
                      self.interstitial = ad
              self.interstitial?.fullScreenContentDelegate = self
              self.interstitial?.present(
                  }*/
                         
   // }
    
   */
    
  override func viewDidLoad() {
    super.viewDidLoad()
    let request = GAMRequest()
    GAMInterstitialAd.load(withAdManagerAdUnitID: "ca-app-pub-4370884607853019/3198503939",
                                         request: request)
      { ad, error in
                  if error != nil { return }
                  self.interstitial = ad
          self.interstitial?.fullScreenContentDelegate = self
          let root = UIApplication.shared.windows.first?.rootViewController
          self.interstitial?.present(fromRootViewController: root!)
         // self.interstitial?.present(fromRootViewController: self)
              }
                       
  }
          
  /// Tells the delegate that the ad failed to present full screen content.
  func ad(_ ad: GADFullScreenPresentingAd, didFailToPresentFullScreenContentWithError error: Error) {
    print("Ad did fail to present full screen content.")
  }

  /// Tells the delegate that the ad will present full screen content.
  func adWillPresentFullScreenContent(_ ad: GADFullScreenPresentingAd) {
    print("Ad will present full screen content.")
  }

  /// Tells the delegate that the ad dismissed full screen content.
  func adDidDismissFullScreenContent(_ ad: GADFullScreenPresentingAd) {
    print("Ad did dismiss full screen content.")
  }
}

