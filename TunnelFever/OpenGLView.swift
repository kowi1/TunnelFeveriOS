import UIKit
import OpenGLES
import SwiftUI


protocol GestureDelegate: AnyObject {
    func handleSwipe(deltaX: CGFloat, deltaY: CGFloat)
}


struct OpenGLView: UIViewRepresentable {
    typealias UIViewType = OpenGLUIView

    @Binding var gestureLocation: CGPoint
    @Binding var isButtonHidden:Bool
    let mtestWrapper: testWrapper
    
    func makeUIView(context: Context) -> OpenGLUIView {
        if(UIScreen.main.bounds.height<UIScreen.main.bounds.width){
            let view = OpenGLUIView(frame: CGRect(x:0,y:0,width:UIScreen.main.bounds.width,height:UIScreen.main.bounds.height), gestureLocation: $gestureLocation,mtestWrapper:mtestWrapper, isButtonHidden:$isButtonHidden)
            
            return view
        }
        else{
            let view = OpenGLUIView(frame: CGRect(x:0,y:0,width:UIScreen.main.bounds.height,height:UIScreen.main.bounds.width), gestureLocation: $gestureLocation,mtestWrapper:mtestWrapper, isButtonHidden:$isButtonHidden)
            
            return view
        }
        // Create and configure your OpenGL view here
        //view.gestureLocation = gestureLocation
        
      
    }
    func updateUIView(_ uiView: OpenGLUIView, context: Context) {
          //  uiView.gestureLocation = gestureLocation
        }
    
   
}



@objc(OpenGLUIView)
class OpenGLUIView: UIView{
    private var context: EAGLContext?
    private var renderBuffer: GLuint = 0
    private var frameBuffer: GLuint = 0
    private var displayLink: CADisplayLink?
    private var BundlePath: String?
    private var DocumentDataPath: String?
    private var con: PurchaseView
    
    private var _test:testWrapper
   

    @Binding var gestureLocation: CGPoint
    @Binding var isButtonHidden:Bool

    override class var layerClass: AnyClass {
        return CAEAGLLayer.self
    }
    
   
    
    init(frame: CGRect,gestureLocation: Binding<CGPoint>,mtestWrapper:testWrapper,isButtonHidden:Binding<Bool>){
         self._gestureLocation = gestureLocation
        self._test=mtestWrapper
        self.con = PurchaseView(mtestWrapper:mtestWrapper)
        self._isButtonHidden = isButtonHidden
        super.init(frame: frame)
        NotificationCenter.default.addObserver(self, selector: #selector(orientationDidChange), name: UIDevice.orientationDidChangeNotification, object: nil)
        BundlePath=""
        /*Bundle.main.url(forResource: "right", withExtension: "tga")?.absoluteString.replacingOccurrences(of: "right.tga", with: "").replacingOccurrences(of: "file://", with: "")*/
        
        DocumentDataPath=FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first?.absoluteString.replacingOccurrences(of: "file://", with: "")
        
        let fileURL = (FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first? .appendingPathComponent("tunnelll.dat"))!
        var text = "0"
        do {
                text = try String(contentsOf: fileURL, encoding: .utf16LittleEndian)
            }
        catch
        {
            text = "1"
        }
               
        do {
         try   text.write(to: fileURL, atomically: false, encoding: .utf16LittleEndian)
        }
        catch
        {
            
        }
        _test.initializePurchase(con)
        _test.initializeUIView(self)
    
        
        if setupOpenGLContext() {
            setupRenderBuffer()
            setupFrameBuffer()
            _test.setupGraphics()
            startRenderLoop()
        }
        //if(UIScreen.main.bounds.height>UIScreen.main.bounds.width){
         //   transform = CGAffineTransform(rotationAngle: .pi / 2)
            
       // }

    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
        NotificationCenter.default.addObserver(self, selector: #selector(orientationDidChange), name: UIDevice.orientationDidChangeNotification, object: nil)
    }
    
   
    deinit {
           // Unregister from orientation change notifications
           NotificationCenter.default.removeObserver(self, name: UIDevice.orientationDidChangeNotification, object: nil)
       }

    private func setupOpenGLContext() -> Bool {
        guard let eaglLayer = self.layer as? CAEAGLLayer else {
            print("Failed to get CAEAGLLayer")
            return false
        }
        
        eaglLayer.isOpaque = true
        eaglLayer.drawableProperties = [
            kEAGLDrawablePropertyRetainedBacking: false,
            kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8
        ]
        
        guard let context = EAGLContext(api: .openGLES3) else {
            print("Failed to create OpenGL context")
            return false
        }
        
        if !EAGLContext.setCurrent(context) {
            print("Failed to set current OpenGL context")
            return false
        }
        
        self.context = context
        return true
    }
    
    private func setupRenderBuffer() {
        if renderBuffer != 0 {
                    glDeleteRenderbuffers(1, &renderBuffer)
                    renderBuffer = 0
                }
        glGenRenderbuffers(1, &renderBuffer)
        glBindRenderbuffer(GLenum(GL_RENDERBUFFER), renderBuffer)
        context?.renderbufferStorage(Int(GL_RENDERBUFFER), from: layer as? CAEAGLLayer)
    }
    
    private func setupFrameBuffer() {
        if frameBuffer != 0 {
                    glDeleteFramebuffers(1, &frameBuffer)
                    frameBuffer = 0
                }
        glGenFramebuffers(1, &frameBuffer)
        glBindFramebuffer(GLenum(GL_FRAMEBUFFER), frameBuffer)
        glFramebufferRenderbuffer(GLenum(GL_FRAMEBUFFER), GLenum(GL_COLOR_ATTACHMENT0), GLenum(GL_RENDERBUFFER), renderBuffer)
    }
    
    private func render() {
        
    
      glClearColor(0.0, 0.0, 0.0, 1.0)
            
      glClear(GLbitfield(GL_COLOR_BUFFER_BIT))
         
        
        _test.nativeEngine(Int32(UIScreen.main.bounds.height),and: Int32(UIScreen.main.bounds.width),and:BundlePath, and:DocumentDataPath
        )

        
       

        // Add your OpenGL rendering code here
        
        context?.presentRenderbuffer(Int(GL_RENDERBUFFER))
    }
    override func layoutSubviews() {
            super.layoutSubviews()
                        // Update the viewport and other OpenGL settings
        glViewport(0, 0, GLsizei(frame.width), GLsizei(frame.height))
            
            // Call the render method to trigger OpenGL rendering
            render()
        
    }
    private func startRenderLoop() {
        displayLink = CADisplayLink(target: self, selector: #selector(renderLoop))
        displayLink?.add(to: .current, forMode: .default)
    }
    
    override func didMoveToWindow() {
           super.didMoveToWindow()

           // Get the initial device orientation
           let currentOrientation = UIDevice.current.orientation
           print("Initial device orientation: \(currentOrientation)")
       }
    @objc func showInterstitial() {
        self.isButtonHidden=false
    }
       @objc func orientationDidChange() {
           let currentOrientation = UIDevice.current.orientation
           print("Device orientation changed: \(currentOrientation)")
          // if (UIDevice.current.orientation.isPortrait){
           //    transform = CGAffineTransform(scaleX: 0.2, y: 0.2)
         //  }
           
           transform = CGAffineTransform(rotationAngle: .pi )
           transform = CGAffineTransform(rotationAngle: .pi )
           transform = CGAffineTransform(rotationAngle: .pi/2 )
         //  }
           // Perform actions in response to orientation changes
           // Update your UI, layout, or apply any necessary changes
       }
    
@objc private func renderLoop() {
    guard let context = context else {
        print("OpenGL context is not available")
        return
    }
    
    EAGLContext.setCurrent(context)
    glViewport(0, 0, GLsizei(bounds.size.width), GLsizei(bounds.size.height))
    
    render()
}
}





