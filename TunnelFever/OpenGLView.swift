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
        
        UIDevice.current.setValue(UIInterfaceOrientation.landscapeRight.rawValue, forKey: "orientation")
        UIViewController.attemptRotationToDeviceOrientation()
        }
}


struct TouchLocatingView: UIViewRepresentable {
    // The types of touches users want to be notified about
    struct TouchType: OptionSet {
        let rawValue: Int

        static let started = TouchType(rawValue: 1 << 0)
        static let moved = TouchType(rawValue: 1 << 1)
        static let ended = TouchType(rawValue: 1 << 2)
        static let all: TouchType = [.started, .moved, .ended]
    }

    // A closure to call when touch data has arrived
    var onUpdate: (CGPoint) -> Void

    // The list of touch types to be notified of
    var types = TouchType.all

    // Whether touch information should continue after the user's finger has left the view
    var limitToBounds = true

    func makeUIView(context: Context) -> TouchLocatingUIView {
        // Create the underlying UIView, passing in our configuration
        let view = TouchLocatingUIView()
        view.onUpdate = onUpdate
        view.touchTypes = types
        view.limitToBounds = limitToBounds
        return view
    }

    func updateUIView(_ uiView: TouchLocatingUIView, context: Context) {
    }

    // The internal UIView responsible for catching taps
    class TouchLocatingUIView: UIView {
        // Internal copies of our settings
        var onUpdate: ((CGPoint) -> Void)?
        var touchTypes: TouchLocatingView.TouchType = .all
        var limitToBounds = true

        // Our main initializer, making sure interaction is enabled.
        override init(frame: CGRect) {
            super.init(frame: frame)
            isUserInteractionEnabled = true
        }

        // Just in case you're using storyboards!
        required init?(coder: NSCoder) {
            super.init(coder: coder)
            isUserInteractionEnabled = true
        }

        // Triggered when a touch starts.
        override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
            guard let touch = touches.first else { return }
            let location = touch.location(in: self)
            send(location, forEvent: .started)
        }

        // Triggered when an existing touch moves.
        override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
            guard let touch = touches.first else { return }
            let location = touch.location(in: self)
            send(location, forEvent: .moved)
        }

        // Triggered when the user lifts a finger.
        override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
            guard let touch = touches.first else { return }
            let location = touch.location(in: self)
            send(location, forEvent: .ended)
        }

        // Triggered when the user's touch is interrupted, e.g. by a low battery alert.
        override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
            guard let touch = touches.first else { return }
            let location = touch.location(in: self)
            send(location, forEvent: .ended)
        }

        // Send a touch location only if the user asked for it
        func send(_ location: CGPoint, forEvent event: TouchLocatingView.TouchType) {
            guard touchTypes.contains(event) else {
                return
            }

            if limitToBounds == false || bounds.contains(location) {
                onUpdate?(CGPoint(x: round(location.x), y: round(location.y)))
            }
        }
    }
}

// A custom SwiftUI view modifier that overlays a view with our UIView subclass.
struct TouchLocater: ViewModifier {
    var type: TouchLocatingView.TouchType = .all
    var limitToBounds = true
    let perform: (CGPoint) -> Void

    func body(content: Content) -> some View {
        content
            .overlay(
                TouchLocatingView(onUpdate: perform, types: type, limitToBounds: limitToBounds)
            )
    }
}

// A new method on View that makes it easier to apply our touch locater view.
extension View {
    func onTouch(type: TouchLocatingView.TouchType = .all, limitToBounds: Bool = true, perform: @escaping (CGPoint) -> Void) -> some View {
        self.modifier(TouchLocater(type: type, limitToBounds: limitToBounds, perform: perform))
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
      //  AppDelegate.lockOrientation(.portrait)
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





