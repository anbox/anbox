/*
 * Copyright (C) 2017 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "anbox/cmds/system_info.h"
#include "anbox/graphics/emugl/RenderApi.h"
#include "anbox/graphics/emugl/DispatchTables.h"
#include "anbox/utils/environment_file.h"
#include "anbox/logger.h"

#include "anbox/build/config.h"

#include <sstream>
#include <fstream>

#include <boost/filesystem.hpp>

#include "OpenGLESDispatch/EGLDispatch.h"

#include "cpu_features_macros.h"
#if defined(CPU_FEATURES_ARCH_X86)
#include "cpuinfo_x86.h"
#endif

namespace fs = boost::filesystem;

namespace {
constexpr const char *os_release_path{"/etc/os-release"};
constexpr const char *host_os_release_path{"/var/lib/snapd/hostfs/etc/os-release"};
constexpr const char *proc_version_path{"/proc/version"};
constexpr const char *binder_path{"/dev/binder"};
constexpr const char *ashmem_path{"/dev/ashmem"};
constexpr const char *os_release_name{"NAME"};
constexpr const char *os_release_version{"VERSION"};

class SystemInformation {
 public:
  SystemInformation() {
    collect_cpu_info();
    collect_os_info();
    collect_kernel_info();
    collect_graphics_info();
  }

  std::string to_text() const {
    std::stringstream s;

    s << "version: "
      << anbox::build::version
      << std::endl;

    if (anbox::utils::is_env_set("SNAP_REVISION")) {
      s << "snap-revision: "
        << anbox::utils::get_env_value("SNAP_REVISION")
        << std::endl;
    }

    s << "cpu:" << std::endl
      << "  arch:  " << cpu_info_.arch << std::endl
      << "  brand: " << cpu_info_.brand << std::endl
      << "  features: " << std::endl;
    for (const auto& feature : cpu_info_.features)
      s << "    - " << feature << std::endl;

    s << "os:" << std::endl
      << "  name: " << os_info_.name << std::endl
      << "  version: " << os_info_.version << std::endl
      << "  snap-based: " << std::boolalpha << os_info_.snap_based << std::endl;

    s << "kernel:" << std::endl
      << "  version: " << kernel_info_.version << std::endl
      << "  binder: " << std::boolalpha << kernel_info_.binder << std::endl
      << "  ashmem: " << std::boolalpha << kernel_info_.ashmem << std::endl;

    auto print_extensions = [](const std::vector<std::string> &extensions) {
      std::stringstream s;
      if (extensions.size() > 0) {
        s << std::endl;
        for (const auto &ext : extensions) {
          if (ext.length() == 0)
            continue;
          s << "      - " << ext << std::endl;
        }
      } else {
        s << " []" << std::endl;
      }
      return s.str();
    };

    s << "graphics:" << std::endl
      << "  egl:" << std::endl
      << "    vendor: " << graphics_info_.egl_vendor << std::endl
      << "    version: " << graphics_info_.egl_version << std::endl
      << "    extensions:" << print_extensions(graphics_info_.egl_extensions)
      << "  gles2:" << std::endl
      << "    vendor: " << graphics_info_.gles2_vendor << std::endl
      << "    vendor: " << graphics_info_.gles2_version << std::endl
      << "    extensions:" << print_extensions(graphics_info_.gles2_extensions);

    return s.str();
  }

 private:
  void collect_cpu_info() {
#if defined(CPU_FEATURES_ARCH_X86)
  cpu_info_.arch = "x86";

  const auto info = cpu_features::GetX86Info();
  if (info.features.aes)
    cpu_info_.features.push_back("aes");
  if (info.features.sse4_1)
    cpu_info_.features.push_back("sse4_1");
  if (info.features.sse4_2)
    cpu_info_.features.push_back("sse4_2");
  if (info.features.avx)
    cpu_info_.features.push_back("avx");
  if (info.features.avx2)
    cpu_info_.features.push_back("avx2");

  char brand_string[49];
  cpu_features::FillX86BrandString(brand_string);
  cpu_info_.brand = brand_string;
#endif
  }

  void collect_os_info() {
    os_info_.snap_based = !anbox::utils::get_env_value("SNAP").empty();
    fs::path path = os_release_path;
    // If we're running from within a snap the best we can do is to
    // access the hostfs and read the os-release file from there.
    if (os_info_.snap_based && fs::exists(host_os_release_path))
        path = host_os_release_path;

    // Double check that there aren't any permission errors when trying
    // to access the file (e.g. because of snap confinement)
    if (fs::exists(path)) {
      anbox::utils::EnvironmentFile os_release(path);
      os_info_.name = os_release.value(os_release_name);
      os_info_.version = os_release.value(os_release_version);
    }
  }

  void collect_kernel_info() {
    if (fs::exists(proc_version_path)) {
      std::ifstream in(proc_version_path);
      std::getline(in, kernel_info_.version);
    }

    kernel_info_.binder = fs::exists(binder_path);
    kernel_info_.ashmem = fs::exists(ashmem_path);
  }

  void collect_graphics_info() {
    auto gl_libs = anbox::graphics::emugl::default_gl_libraries();
    if (!anbox::graphics::emugl::initialize(gl_libs, nullptr, nullptr)) {
      return;
    }

    auto display = s_egl.eglGetDisplay(0);
    if (display != EGL_NO_DISPLAY) {
      s_egl.eglInitialize(display, nullptr, nullptr);

      auto egl_safe_get_string = [display](EGLint item) {
        auto str = s_egl.eglQueryString(display, item);
        if (!str)
          return std::string("n/a");
        return std::string(reinterpret_cast<const char*>(str));
      };

      graphics_info_.egl_vendor = egl_safe_get_string(EGL_VENDOR);
      graphics_info_.egl_version = egl_safe_get_string(EGL_VERSION);
      const auto egl_extensions = egl_safe_get_string(EGL_EXTENSIONS);
      graphics_info_.egl_extensions = anbox::utils::string_split(egl_extensions, ' ');

      GLint config_attribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
                                EGL_NONE};

      EGLConfig config;
      int n;
      if (s_egl.eglChooseConfig(display, config_attribs, &config, 1, &n) && n > 0) {
        GLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        auto context = s_egl.eglCreateContext(display, config, nullptr, attribs);
        if (context != EGL_NO_CONTEXT) {
          // We require surfaceless-context support here for now. If eglMakeCurrent fails
          // glGetString will return null below which we handle correctly.
          s_egl.eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context);

          auto gl_safe_get_string = [](GLint item) {
            auto str = s_gles2.glGetString(item);
            if (!str)
              return std::string("n/a");
            return std::string(reinterpret_cast<const char*>(str));
          };

          graphics_info_.gles2_vendor = gl_safe_get_string(GL_VENDOR);
          graphics_info_.gles2_version = gl_safe_get_string(GL_VERSION);
          const auto gl_extensions = gl_safe_get_string(GL_EXTENSIONS);
          graphics_info_.gles2_extensions = anbox::utils::string_split(gl_extensions, ' ');

          s_egl.eglMakeCurrent(display, nullptr, nullptr, nullptr);
          s_egl.eglDestroyContext(display, context);
        }
      }
    }
  }

  struct {
    std::string arch;
    std::string brand;
    std::vector<std::string> features;
  } cpu_info_;

  struct {
    bool snap_based = false;
    std::string name = "n/a";
    std::string version = "n/a";
  } os_info_;

  struct {
    std::string version = "n/a";
    bool binder = false;
    bool ashmem = false;
  } kernel_info_;

  struct {
    std::string egl_vendor = "n/a";
    std::string egl_version = "n/a";
    std::vector<std::string> egl_extensions;
    std::string gles2_vendor = "n/a";
    std::string gles2_version = "n/a";
    std::vector<std::string> gles2_extensions;
  } graphics_info_;
};

std::ostream &operator<<(std::ostream &out, const SystemInformation &info) {
  out << info.to_text();
  return out;
}
}

anbox::cmds::SystemInfo::SystemInfo()
    : CommandWithFlagsAndAction{
          cli::Name{"system-info"}, cli::Usage{"system-info"},
          cli::Description{"Print various information about the system we're running on"}} {
  action([](const cli::Command::Context&) {
    SystemInformation si;
    std::cout << si;
    return EXIT_SUCCESS;
  });
}
