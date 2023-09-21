//
//  FVRInAppPurchaseVC.swift
//  favroutes
//
//  Created by wisaruthk on 10/8/2558 BE.
//  Copyright © 2558 kojo. All rights reserved.
//
import UIKit
import StoreKit

//@objc(PurchaseView)
class PurchaseView:NSObject,SKProductsRequestDelegate, SKPaymentTransactionObserver {
   // @IBOutlet weak var purchaseButton: UIButton!
    //@IBOutlet weak var restorePurchaseButton:UIButton!

    var product_id:String?;
    var appusername:String?;
    
   private var _test:testWrapper
    
    @objc(initPurchaseViewWithWrapper:)
    init(mtestWrapper:testWrapper){
        
       self.product_id = "life_heart_572023";
       self.appusername = "dolukowi@gmail.com";
       self._test=mtestWrapper;
        super.init();
        SKPaymentQueue.default().add(self);
    }
    deinit {
           SKPaymentQueue.default().remove(self)
       }
 /*   override func viewDidLoad() {
        product_id = "life_heart_572023";
        //appusername = "favid@test.com";
        super.viewDidLoad()
        SKPaymentQueue.default().add(self)
        self.title = "In App Purchase"
        //
    }
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        //
    }
    
    override func viewDidDisappear(_ animated: Bool) {
        super.viewDidDisappear(animated)
        SKPaymentQueue.default().remove(self)
    }
    */
    @objc
    /*@IBAction*/ func buyConsumable() {
        print("About to fetch the products");
        self.product_id = "life_heart_572023";
        // We check that we are allow to make the purchase.
        if (SKPaymentQueue.canMakePayments())
        {
            let productID:Set<String> = Set<String>(arrayLiteral: self.product_id!)
            let productsRequest:SKProductsRequest = SKProductsRequest(productIdentifiers: productID);
            productsRequest.delegate = self;
            productsRequest.start();
            print("Fetching Products");
        }else{
            print("can't make purchases");
        }
    }
    @objc
   /*@IBAction*/ func restorePurchase(){
        print("About to restore purchase");
        // For IAP with appusername
       // SKPaymentQueue.default().restoreCompletedTransactions(withApplicationUsername: self.appusername)
        
        // For default
         SKPaymentQueue.default().restoreCompletedTransactions();
    }
    
    
    // Helper Methods
    func buyProduct(product: SKProduct){
        print("Sending the Payment Request to Apple");
      //  self.appusername = "dolukowi@gmail.com";
        // For IAP with appusername
       // let payment:SKMutablePayment = SKMutablePayment(product: product)
      //  payment.applicationUsername = self.appusername!;
      //  print("payment request with appusername = \(payment.applicationUsername)");
        
        // For default
        var payment = SKPayment(product: product)
        
        SKPaymentQueue.default().add(payment);
        
    }
    
    // Delegate Methods for IAP
    
    func productsRequest (_ request: SKProductsRequest, didReceive response: SKProductsResponse) {
        print("got the request from Apple")
        
        let count : Int = response.products.count
        if (count>0) {
            let validProducts = response.products
            let validProduct: SKProduct = response.products[0] as SKProduct
            if (validProduct.productIdentifier == self.product_id) {
                print(validProduct.localizedTitle)
                print(validProduct.localizedDescription)
                print(validProduct.price)
                buyProduct(product: validProduct);
            } else {
                print(validProduct.productIdentifier)
            }
        } else {
            print("nothing")
        }
    }
    
    
    func request(request: SKRequest!, didFailWithError error: NSError!) {
        print("Request fail with error \(error.localizedDescription)");
    }
    
    // Delegate for SKPaymentTransactionObserver
    func paymentQueue(_ queue: SKPaymentQueue, updatedTransactions transactions: [SKPaymentTransaction])
    {
       // var trans:SKPaymentTransaction
        print("Received Payment Transaction Response from Apple");
        for transaction:SKPaymentTransaction in transactions {
            // trans = transaction {
                switch transaction.transactionState {
                case .purchased:
                    print("Product Purchased");
                    //For IAP With appusername
                    print("tran with appuser = \(transaction.payment.applicationUsername)")
                    _test.updateLife(Int32(5));
                    SKPaymentQueue.default().finishTransaction(transaction)
                    break;
                case .failed:
                    print("Purchased Failed with \(transaction.error)");
                    SKPaymentQueue.default().finishTransaction(transaction)
                    break;
                    // case .Restored:
                    //[self restoreTransaction:transaction];
                default:
                    break;
                }
            }
        }
        
    }
    
    // Sent when an error is encountered while adding transactions from the user's purchase history back to the queue.
    func paymentQueue(queue: SKPaymentQueue, restoreCompletedTransactionsFailedWithError error: NSError){
        print("restore completed but failed with error \(error.localizedDescription)");
    }
    
    // Sent when all transactions from the user's purchase history have successfully been added back to the queue.
   
    func paymentQueueRestoreCompletedTransactionsFinished(queue: SKPaymentQueue){
        print("restore completed with queue \(queue.transactions[0].payment.productIdentifier)");
    }

