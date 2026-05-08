# Uncomment the next line to define a global platform for your project
# platform :ios, '9.0'

target 'TunnelFever' do
  # Comment the next line if you don't want to use dynamic frameworks
  use_frameworks!

  # Pods for TunnelFever
# Add the Firebase pod for Google Analytics
pod 'Firebase'
pod 'Firebase/Analytics'
pod 'FirebaseAppCheck'
pod 'Google-Mobile-Ads-SDK'
pod 'FirebaseMessaging'
pod 'FirebaseStorage'
pod 'FirebaseRemoteConfig'
pod 'FirebaseFirestore'

# Add the pods for any other Firebase products you want to use in your app
# For example, to use Firebase Authentication and Firebase Realtime Database
pod 'FirebaseAuth'
pod 'FirebaseDatabase'
end

post_install do |installer|
  installer.pods_project.targets.each do |target|
    target.build_configurations.each do |config|
      # Fix DT_TOOLCHAIN_DIR deprecation (Xcode 15+)
      xcconfig_path = config.base_configuration_reference&.real_path
      next unless xcconfig_path&.exist?
      xcconfig = File.read(xcconfig_path)
      if xcconfig.include?("DT_TOOLCHAIN_DIR")
        xcconfig.gsub!("DT_TOOLCHAIN_DIR", "TOOLCHAIN_DIR")
        File.write(xcconfig_path, xcconfig)
      end
    end

    # Fix BoringSSL-GRPC: Apple Clang 21 (Xcode 26) rejects -GCC_WARN_INHIBIT_ALL_WARNINGS
    # as an unsupported -G flag for arm64-apple-ios targets.
    if target.name == 'BoringSSL-GRPC'
      target.source_build_phase.files.each do |file|
        if file.settings && file.settings['COMPILER_FLAGS']
          flags = file.settings['COMPILER_FLAGS'].split
          flags.reject! { |f| f == '-GCC_WARN_INHIBIT_ALL_WARNINGS' }
          file.settings['COMPILER_FLAGS'] = flags.join(' ')
        end
      end
    end

    # Fix gRPC-Core: Apple Clang 21 (Xcode 26) enforces strict C++ conformance for
    # 'template' keyword usage, breaking gRPC's basic_seq.h at line 499.
    if target.name == 'gRPC-Core'
      target.build_configurations.each do |config|
        config.build_settings['OTHER_CPLUSPLUSFLAGS'] =
          '$(inherited) -Wno-missing-template-arg-list-after-template-kw'
      end
    end
  end

  # Fix FirebaseAuth 10.x using deprecated 'xros' platform name.
  # Xcode 15+ SDK renamed xrOS → visionOS; strip the block to keep
  # the existing macos/tvos/watchos unavailability declaration.
  auth_header = File.join(
    installer.sandbox.root,
    "FirebaseAuth/FirebaseAuth/Sources/Public/FirebaseAuth/FIRFederatedAuthProvider.h"
  )
  if File.exist?(auth_header)
    content = File.read(auth_header)
    patched = content.gsub(
      /\n#if defined\(TARGET_OS_XR\).*?#endif.*?TARGET_OS_XR/m,
      ""
    ).gsub(
      /,\s*\n\s*xros\s*\n/,
      "\n"
    )
    File.write(auth_header, patched) if patched != content
  end
end
