from ctypes import *

try:
    # try linux and cygwin
    lib = cdll.LoadLibrary("libppm.so")
except OSError:
    # try darwin
    try:
        lib = cdll.LoadLibrary("libppm.dylib")
    except OSError:
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
