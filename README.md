# android-hpe-library
Head pose estimation library written in C++ for android-hpe, to use with Android NDK. This repo contains all code nedded to perform ndk-build and therefore to get a suitable shared library to be included into android-hpe.
Please see android-hpe for other details.

### Get the source
    $ git clone --recursive https://github.com/beraldofilippo/android-hpe-library.git

### Prerequisites
* Download Android-NDK from [Android website](https://developer.android.com/ndk/downloads/index.html)

* It's crucial to know how the build process works. Please see [reference docs](https://developer.android.com/ndk/index.html) and also refer to [android.mk](http://android.mk/).

* Install Android Debug Bride (ADB). You can download it via [Android SDK Manager](https://developer.android.com/sdk/installing/index.html)

* Prepare an Android device for test

### Credits
This repository heavily relies and replicates works in [dlib-android](https://github.com/tzutalin/dlib-android) and in [gazr](https://github.com/severin-lemaignan/gazr).
