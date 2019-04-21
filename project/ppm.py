import os.path
from os import environ

from ctypes import *
environ['DYLD_LIBRARY_PATH'] = os.path.dirname(os.path.abspath(__file__)) + ';' + environ['DYLD_LIBRARY_PATH']

locations = ["libppm.so", "libppm.dylib"]

lib = None

for location in locations:
    try:
        print("Trying location: " + location)
        lib = cdll.LoadLibrary(location)

        if lib:
            break
    except OSError:
        pass

if lib is None:
    print("Error: Failed to load the ppm library. Please check your library path.")
    quit()

class Ppm(object):
    def __init__(self):
        self.app = lib.PPM_new()
        self.encoding = "utf-8"
        lib.PPM_render.restype = c_char_p

    def render(self, text):
        encoded = text.encode(self.encoding)
        result = lib.PPM_render(self.app, encoded)
        return result.decode(self.encoding)
