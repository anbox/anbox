from conans import ConanFile, CMake
import os

class BackwardCpp(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    name = 'backward'
    url = 'https:77github.com/Manu343726/backward-cpp'
    license = 'MIT'
    version = '0.0.0'
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

    def cmake_option(self, option, prefix = ''):
        return '-D{}{}={}'.format(prefix, option.upper(), self.options[option])

    def build(self):
        cmake = CMake(self.settings)

        options = ''
        options += self.cmake_option('stack_walking_unwind')
        options += self.cmake_option('stack_walking_backtrace')
        options += self.cmake_option('stack_details_auto_detect')
        options += self.cmake_option('stack_details_backtrace_symbol')
        options += self.cmake_option('stack_details_dw')
        options += self.cmake_option('stack_details_bfd')
        options += self.cmake_option('shared', prefix = 'BACKWARD_')

        self.run('cmake {} {} {} -DBACKWARD_TESTS=OFF'.format(self.conanfile_directory, cmake.command_line, options))
        self.run('cmake --build . {}'.format(cmake.build_config))

    def package(self):
        self.copy('backward.hpp', os.path.join('include', 'backward'))
        self.copy('*.a', dst='lib')
        self.copy('*.so', dst='lib')
        self.copy('*.lib', dst='lib')
        self.copy('*.dll', dst='lib')

    def package_info(self):
        self.cpp_info.libs = ['backward']
