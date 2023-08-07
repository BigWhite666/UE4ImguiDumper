
adb push outputs\arm64-v8a\NativeSurface /data/local/tmp
adb shell su -c chmod +x /data/local/tmp/NativeSurface
adb shell su -c /data/local/tmp/NativeSurface
