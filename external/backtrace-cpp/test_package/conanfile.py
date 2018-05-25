from conans import ConanFile, CMake
import os

class TestBackward(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    requires = 'cmake-utils/0.0.0@Manu343726/testing', 'backward/0.0.0@Manu343726/testing'
    generators = 'cmake'

    def build(self):
        cmake = CMake(self.settings)
        self.run('cmake {} {}'.format(self.conanfile_directory, cmake.command_line))
        self.run('cmake --build . {}'.format(cmake.build_config))

    def test(self):
        self.run(os.path.join('.', 'bin', 'example'))
