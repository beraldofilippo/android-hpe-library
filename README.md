# android-hpe-library
Head pose estimation library written in C++ for android-hpe, to use with Android NDK. This repo contains all code nedded to perform ndk-build and therefore to get a suitable shared library to be included into android-hpe.
Please see android-hpe for other details.

### Get the source
    $ git clone --recursive https://github.com/beraldofilippo/android-hpe-library.git

### Prerequisites
* It's beneficial to know how the build process works. Please see [reference docs](https://developer.android.com/ndk/index.html) and also refer to [android.mk](http://android.mk/)

* Download Android-NDK ndk-r13b - [Android NDK downloads page](https://developer.android.com/ndk/downloads/index.html#older-downloads) - it is a rather old version 

* Download OpenCV Android SDK 3.1.0 - [sourceforge](https://sourceforge.net/projects/opencvlibrary/files/opencv-android/3.1.0/OpenCV-3.1.0-android-sdk.zip/download) - place it into `/third_party` folder, (the other needed dependency miniglog is already there).
This way, the `/third_party` folder will have `/opencv-android-sdk` and `/miniglog` as subfolders.

### Build JNI code and shared library for Android application
* cd to the project root folder `[android-hpe-library_directory]`
* Perform NDK build using `ndk-build -j 2` this process builds .so files
* Grab the folders located into `[android-hpe-library_directory]/libs`, each folder contains a specific .so based on architecture
* Put the folders into the [android-hpe](https://github.com/beraldofilippo/android-hpe) Android project into the path `[android-hpe_directory]/dlib/src/main/jniLibs`

### Credits
This repository heavily relies and replicates works in [dlib-android](https://github.com/tzutalin/dlib-android) and in [gazr](https://github.com/severin-lemaignan/gazr).
