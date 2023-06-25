import UIKit
import OpenGLES
import SwiftUI

protocol GestureDelegate: AnyObject {
    func handleSwipe(deltaX: CGFloat, deltaY: CGFloat)
}

struct OpenGLView: UIViewRepresentable {
    typealias UIViewType = OpenGLUIView

    @Binding var gestureLocation: CGPoint
    let mtestWrapper: testWrapper
    
    func makeUIView(context: Context) -> OpenGLUIView {
        let view = OpenGLUIView(frame: CGRect(x:0,y:0,width:UIScreen.main.bounds.height,height:UIScreen.main.bounds.width), gestureLocation: $gestureLocation,mtestWrapper:mtestWrapper)
        // Create and configure your OpenGL view here
//view.gestureLocation = gestureLocation
        return view
    }
    func updateUIView(_ uiView: OpenGLUIView, context: Context) {
          //  uiView.gestureLocation = gestureLocation
           
        }
    static func lockOrientation(_ orientation: UIInterfaceOrientationMask) {
            if let delegate = UIApplication.shared.delegate as? AppDelegate {
                delegate.orientationLock = orientation
            }
        }
   
}

class AppDelegate: UIResponder, UIApplicationDelegate {
      var window: UIWindow?
       var orientationLock = UIInterfaceOrientationMask.all

    func application(_ application: UIApplication, supportedInterfaceOrientationsFor window: UIWindow?) -> UIInterfaceOrientationMask {
           return orientationLock
       }
}

class OpenGLUIView: UIView{
    
    private var context: EAGLContext?
    private var renderBuffer: GLuint = 0
    private var frameBuffer: GLuint = 0
    private var displayLink: CADisplayLink?
    
    private var _test:testWrapper
 

    @Binding var gestureLocation: CGPoint
 
    
   

    override class var layerClass: AnyClass {
        return CAEAGLLayer.self
    }
    
    init(frame: CGRect,gestureLocation: Binding<CGPoint>,mtestWrapper:testWrapper) {
         self._gestureLocation = gestureLocation
        self._test=mtestWrapper
        super.init(frame: frame)
         
        if setupOpenGLContext() {
            setupRenderBuffer()
            setupFrameBuffer()
            _test.setupGraphics()
            startRenderLoop()
        }
         transform = CGAffineTransform(rotationAngle: .pi / 2)
    }
    
    required init?(coder aDecoder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
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
         
       
  
        _test.nativeEngine(Int32(UIScreen.main.bounds.height),and: Int32(UIScreen.main.bounds.width))
       
    
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





