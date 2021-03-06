
#ifndef SDL_ANDROID_INCLUDE_H_
#define SDL_ANDROID_INCLUDE_H_

using namespace SDL::Android;

#ifndef nullptr
const                        // this is a const object...
class {
public:
  template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};
#endif

#endif // SDL_ANDROID_INCLUDE_H_
