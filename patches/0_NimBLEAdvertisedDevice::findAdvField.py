# Instructions:
# if there is a file .pio/libdeps/dictionary/NimBLE-Arduino/src/NimBLEAdvertisedDevice.cpp
# and there is not a file .pio/libdeps/dictionary/NimBLE-Arduino/src/NimBLEAdvertisedDevice.cpp.bak

# copy .pio/libdeps/dictionary/NimBLE-Arduino/src/NimBLEAdvertisedDevice.cpp to .pio/libdeps/dictionary/NimBLE-Arduino/src/NimBLEAdvertisedDevice.cpp.bak

# copy new_files/NimBLEAdvertisedDevice.cpp to .pio/libdeps/dictionary/NimBLE-Arduino/src/NimBLEAdvertisedDevice.cpp

# show message patched.

from os.path import join, isfile
import os
import shutil

project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src_file = join(project_root, ".pio", "libdeps", "dictionary", "NimBLE-Arduino", "src", "NimBLEAdvertisedDevice.cpp")
backup_file = src_file + ".bak"
patched_file = join(project_root, "patches", "new_files", "NimBLEAdvertisedDevice.cpp")

if isfile(backup_file): print("✓ Already patched"); exit(0)
shutil.copy2(src_file, backup_file)
shutil.copy2(patched_file, src_file)
print("[Patch] 0_NimBLEAdvertisedDevice::findAdvField applied!")
