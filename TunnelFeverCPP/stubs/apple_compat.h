#pragma once
// Forward-declare ObjC runtime types so C++ headers that reference them
// compile cleanly without -x objective-c++.
#ifndef __OBJC__
struct objc_object;
typedef struct objc_object objc_object;
#endif
