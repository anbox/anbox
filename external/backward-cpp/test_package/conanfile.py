from conans import ConanFile, CMake
import os

class TestBackward(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'

    def build(self):
        cmake = CMake(self)
        cmake.configure(defs={'CMAKE_VERBOSE_MAKEFILE': 'ON'})
        cmake.build()

    def test(self):
        self.run(os.path.join('.', 'bin', 'example'))
