from conans import ConanFile, CMake
import os

class BackwardCpp(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    name = 'backward'
    url = 'https://github.com/bombela/backward-cpp'
    license = 'MIT'
    version = '1.3.0'
    options = {
        'stack_walking_unwind': [True, False],
        'stack_walking_backtrace': [True, False],
        'stack_details_auto_detect': [True, False],
        'stack_details_backtrace_symbol': [True, False],
        'stack_details_dw': [True, False],
        'stack_details_bfd': [True, False],
        'shared': [True, False]
    }
    default_options = (
        'stack_walking_unwind=True',
        'stack_walking_backtrace=False',
        'stack_details_auto_detect=True',
        'stack_details_backtrace_symbol=False',
        'stack_details_dw=False',
        'stack_details_bfd=False',
        'shared=False'
    )
    exports = 'backward.cpp', 'backward.hpp', 'test/*', 'CMakeLists.txt', 'BackwardConfig.cmake'
    generators = 'cmake'

    def build(self):
        cmake = CMake(self)

        cmake.configure(defs={'BACKWARD_' + name.upper(): value for name, value in self.options.values.as_list()})
        cmake.build()

    def package(self):
        self.copy('backward.hpp', os.path.join('include', 'backward'))
        self.copy('*.a', dst='lib')
        self.copy('*.so', dst='lib')
        self.copy('*.lib', dst='lib')
        self.copy('*.dll', dst='lib')

    def package_info(self):
        self.cpp_info.libs = ['backward']
