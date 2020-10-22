#!/usr/bin/env python3

# SOURCE: https://android.googlesource.com/platform/external/qemu/+/emu-master-dev/android/scripts/gen-entries.py

# Copyright 2015 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# Utility functions used to parse a list of DLL entry points.
# Expected format:
#
#   <empty-line>   -> ignored
#   #<comment>     -> ignored
#   %<verbatim>    -> verbatim output for header files.
#   !<prefix>      -> prefix name for header files.
#   <return-type> <function-name> <signature> ; -> entry point declaration.
#
# Anything else is an error.
import re
import sys
import argparse
re_func = re.compile(r"""^(.*[\* ])([A-Za-z_][A-Za-z0-9_]*)\((.*)\);$""")
re_param = re.compile(r"""^(.*[\* ])([A-Za-z_][A-Za-z0-9_\[\]]*)$""")
class Entry:
    """Small class used to model a single DLL entry point."""
    def __init__(self, func_name, return_type, parameters):
        """Initialize Entry instance. |func_name| is the function name,
           |return_type| its return type, and |parameters| is a list of
           (type,name) tuples from the entry's signature.
        """
        self.__name__ = func_name
        self.return_type = return_type
        self.parameters = ""
        self.vartypes = []
        self.varnames = []
        self.call = ""
        comma = ""
        for param in parameters:
            self.vartypes.append(param[0])
            self.varnames.append(param[1])
            self.parameters += "%s%s %s" % (comma, param[0], param[1])
            self.call += "%s%s" % (comma, param[1])
            comma = ", "
def banner_command(argv):
    """Return sanitized command-line description.
       |argv| must be a list of command-line parameters, e.g. sys.argv.
       Return a string corresponding to the command, with platform-specific
       paths removed."""
    # Remove path from first parameter
    argv = argv[:]
    argv[0] = "android/scripts/gen-entries.py"
    return ' '.join(argv)
def parse_entries_file(lines):
    """Parse an .entries file and return a tuple of:
        entries: list of Entry instances from the file.
        prefix_name: prefix name from the file, or None.
        verbatim: list of verbatim lines from the file.
        errors: list of errors in the file, prefixed by line number.
    """
    entries = []
    verbatim = []
    errors = []
    lineno = 0
    prefix_name = None
    namespaces = []
    for line in lines:
        lineno += 1
        line = line.strip()
        if len(line) == 0:  # Ignore empty lines
            continue
        if line[0] == '#':  # Ignore comments
            continue
        if line[0] == '!':  # Prefix name
            prefix_name = line[1:]
            continue
        if line[0] == '%':  # Verbatim line copy
            verbatim.append(line[1:])
            continue
        if line.startswith("namespaces"): # Namespaces
            namespaces = map(lambda t: t.strip(), line.split("namespaces")[1].strip().split(","))
            continue
        # Must be a function signature.
        m = re_func.match(line)
        if not m:
            errors.append("%d: '%s'" % (lineno, line))
            continue
        return_type, func_name, parameters = m.groups()
        return_type = return_type.strip()
        parameters = parameters.strip()
        params = []
        failure = False
        if parameters != "void":
            for parameter in parameters.split(','):
                parameter = parameter.strip()
                m = re_param.match(parameter)
                if not m:
                    errors.append("%d: parameter '%s'" % (lineno, parameter))
                    failure = True
                    break
                else:
                    param_type, param_name = m.groups()
                    params.append((param_type.strip(), param_name.strip()))
        if not failure:
            entries.append(Entry(func_name, return_type, params))
    return (entries, prefix_name, verbatim, namespaces, errors)
def gen_functions_header(entries, prefix_name, verbatim, filename, with_args):
    """Generate a C header containing a macro listing all entry points.
       |entries| is a list of Entry instances.
       |prefix_name| is a prefix-name, it will be converted to upper-case.
       |verbatim| is a list of verbatim lines that must appear before the
       macro declaration. Useful to insert #include <> statements.
       |filename| is the name of the original file.
    """
    prefix_name = prefix_name.upper()
    print("// Auto-generated with: %s" % banner_command(sys.argv))
    print("// DO NOT EDIT THIS FILE")
    print("")
    print("#ifndef %s_FUNCTIONS_H" % prefix_name)
    print("#define %s_FUNCTIONS_H" % prefix_name)
    print("")
    for line in verbatim:
        print(line)
    print("#define LIST_%s_FUNCTIONS(X) \\" % prefix_name)
    new_header_apis = [
        # AEMU private exts
        "eglGetMaxGLESVersion",
        "eglBlitFromCurrentReadBufferANDROID",
        "eglSetImageFenceANDROID",
        "eglWaitImageFenceANDROID",
        "eglAddLibrarySearchPathANDROID",
        "eglQueryVulkanInteropSupportANDROID",
        "eglQueryVulkanInteropSupportANDROID",
        # For snapshotting
        "eglLoadConfig",
        "eglLoadContext",
        "eglLoadAllImages",
        "eglSaveConfig",
        "eglSaveContext",
        "eglSaveAllImages",
        "eglPreSaveContext",
        "eglPostLoadAllImages",
        "eglPostSaveContext",
        "eglUseOsEglApi",
        "eglFillUsages",
        "eglSetMaxGLESVersion",
    ]
    need_decls = []
    for entry in entries:
        if entry.__name__ in new_header_apis:
            need_decls.append(entry)
        if with_args:
            print("  X(%s, %s, (%s), (%s)) \\" % \
                    (entry.return_type, entry.__name__, entry.parameters,
                     entry.call))
        else:
            print("  X(%s, %s, (%s)) \\" % \
                    (entry.return_type, entry.__name__, entry.parameters))
    print("")
    for entry in need_decls:
        print("EGLAPI %s EGLAPIENTRY %s(%s);" % (entry.return_type, entry.__name__, entry.parameters))
    print("")
    print("#endif  // %s_FUNCTIONS_H" % prefix_name)
def gen_static_translator_namespaced_header(entries, namespaces, prefix_name, verbatim, filename, with_args):
    """Generate a C++ header containing a header for |entries|
       with nesting inside |namespaces|.
       |prefix_name| is a prefix-name, it will be converted to upper-case.
       |verbatim| is a list of verbatim lines that must appear before the
       macro declaration. Useful to insert #include <> statements.
       |filename| is the name of the original file.
    """
    prefix_name = prefix_name.upper()
    print("// Auto-generated with: %s" % banner_command(sys.argv))
    print("// DO NOT EDIT THIS FILE")
    print("")
    print("#pragma once")
    print("")
    for line in verbatim:
        print(line)
    for ns in namespaces:
        print("namespace %s {" % ns)
    for entry in entries:
        if "gles" in filename:
            print("GL_APICALL %s GL_APIENTRY %s(%s);" % (entry.return_type, entry.__name__, entry.parameters))
        else:
            print("EGLAPI %s EGLAPIENTRY %s(%s);" % (entry.return_type, entry.__name__, entry.parameters))
    for ns in namespaces:
        print("} // namespace %s" % ns)
def gen_static_translator_namespaced_stubs(entries, namespaces, prefix_name, verbatim, filename, with_args):
    """Generate a C++ header containing a header for |entries|
       with nesting inside |namespaces|.
       |prefix_name| is a prefix-name, it will be converted to upper-case.
       |verbatim| is a list of verbatim lines that must appear before the
       macro declaration. Useful to insert #include <> statements.
       |filename| is the name of the original file.
    """
    prefix_name = prefix_name.upper()
    print("// Auto-generated with: %s" % banner_command(sys.argv))
    print("// DO NOT EDIT THIS FILE")
    print("")
    for line in verbatim:
        print(line)
    for ns in namespaces:
        print("namespace %s {" % ns)
    for entry in entries:
        if "gles" in filename:
            if "void" == entry.return_type:
                return_part = "return"
            else:
                return_part = "return (%s)0" % entry.return_type
            print("GL_APICALL %s GL_APIENTRY %s(%s) { %s; }" % (entry.return_type, entry.__name__, ", ".join(entry.vartypes), return_part))
        else:
            print("EGLAPI %s EGLAPIENTRY %s(%s);" % (entry.return_type, entry.__name__, entry.parameters))
    for ns in namespaces:
        print("} // namespace %s" % ns)
# The purpose of gen_translator()
# is to quickly generate implementations on the host Translator,
# which processes commands that just got onto the renderthread off goldfish pipe
# and are fed to system OpenGL.
def gen_translator(entries):
    # Definitions for custom implementation bodies go in
    # android/scripts/gles3translatorgen/gles30_custom.py
    # android/scripts/gles3translatorgen/gles31_custom.py
    from gles3translatorgen import gles30_custom
    from gles3translatorgen import gles31_custom
    translator_custom_share_processing = { }
    for (k, v) in list(gles30_custom.custom_share_processing.items()):
        translator_custom_share_processing[k] = v
    for (k, v) in list(gles31_custom.custom_share_processing.items()):
        translator_custom_share_processing[k] = v
    translator_custom_pre = { }
    for (k, v) in list(gles30_custom.custom_preprocesses.items()):
        translator_custom_pre[k] = v
    for (k, v) in list(gles31_custom.custom_preprocesses.items()):
        translator_custom_pre[k] = v
    translator_custom_post = { }
    for (k, v) in list(gles30_custom.custom_postprocesses.items()):
        translator_custom_post[k] = v
    for (k, v) in list(gles31_custom.custom_postprocesses.items()):
        translator_custom_post[k] = v
    translator_no_passthrough = {}
    for (k, v) in list(gles30_custom.no_passthrough.items()):
        translator_no_passthrough[k] = v
    for (k, v) in list(gles31_custom.no_passthrough.items()):
        translator_no_passthrough[k] = v
    translator_needexternc = {
            "glGetStringi": 1,
            "glUniform4ui": 1,
            "glGetUniformIndices": 1,
            "glTransformFeedbackVaryings": 1,
            "glCreateShaderProgramv": 1,
            "glProgramUniform2ui": 1,
            "glProgramUniform3ui": 1,
            "glProgramUniform4ui": 1,
            "glBindVertexBuffer": 1,
    };
    translator_nocontext_fail_codes = {
            "glClientWaitSync" : "GL_WAIT_FAILED",
    };
    def needExternC(entry):
        if entry.__name__ in translator_needexternc:
            return "extern \"C\" "
        else:
            return ""
    def get_fail_code(entry):
        if entry.__name__ in translator_nocontext_fail_codes:
            return translator_nocontext_fail_codes[entry.__name__];
        else:
            return "0"
    def gen_cxt_getter(entry):
        if (entry.return_type == "void"):
            print("    GET_CTX_V2();")
        else:
            print("    GET_CTX_V2_RET(%s);" % get_fail_code(entry))
    def gen_validations_custom_impl(entry):
        isGen = entry.__name__.startswith("glGen")
        isDelete = entry.__name__.startswith("glDelete")
        isBufferOp = "Buffer" in entry.__name__
        hasTargetArg = "target" in entry.varnames
        hasProgramArg = "program" in entry.varnames
        def mySetError(condition, glerr):
            if entry.return_type == "void":
                return "SET_ERROR_IF(%s,%s)" % (condition, glerr);
            else:
                return "RET_AND_SET_ERROR_IF(%s,%s,%s)" % (condition, glerr, get_fail_code(entry));
        if (isGen or isDelete) and ("n" in entry.varnames):
            print("    %s;" % mySetError("n < 0", "GL_INVALID_VALUE"));
        if (isBufferOp and hasTargetArg):
            print("    %s;" % mySetError("!GLESv2Validate::bufferTarget(ctx, target)", "GL_INVALID_ENUM"));
        if entry.__name__ in translator_custom_pre:
            print(translator_custom_pre[entry.__name__])
    def gen_call_ret(entry):
        globalNameTypes = {
                ("GLuint", "program") : "NamedObjectType::SHADER_OR_PROGRAM",
                ("GLuint", "texture") : "NamedObjectType::TEXTURE",
                ("GLuint", "buffer") : "NamedObjectType::VERTEXBUFFER",
                ("GLuint", "sampler") : "NamedObjectType::SAMPLER",
                ("GLuint", "query") : "NamedObjectType::QUERY",
        }
        globalNames = {
                ("GLuint", "program") : "globalProgramName",
                ("GLuint", "texture") : "globalTextureName",
                ("GLuint", "buffer") : "globalBufferName",
                ("GLuint", "sampler") : "globalSampler",
                ("GLuint", "query") : "globalQuery",
        }
        needsShareGroup = False
        for v in zip(entry.vartypes, entry.varnames):
            if v in list(globalNameTypes.keys()):
                needsShareGroup = True
        if needsShareGroup:
            print("    if (ctx->shareGroup().get()) {")
            for key in zip(entry.vartypes, entry.varnames):
                vartype, varname = key
                if key in globalNames:
                    print("        const GLuint %s = ctx->shareGroup()->getGlobalName(%s, %s);" % (globalNames[key], globalNameTypes[key], varname))
        globalCall = ", ".join([globalNames.get(k, k[1]) for k in zip(entry.vartypes, entry.varnames)])
        if needsShareGroup and entry.__name__ in translator_custom_share_processing:
            print(translator_custom_share_processing[entry.__name__])
        if (entry.return_type == "void"):
            if (needsShareGroup):
                print("   ")
            if entry.__name__ not in translator_no_passthrough:
                print("    ctx->dispatcher().%s(%s);" % (entry.__name__, globalCall))
            if needsShareGroup:
                print("    }")
            if entry.__name__ in translator_custom_post:
                print(translator_custom_post[entry.__name__]);
        else:
            if (needsShareGroup):
                print("   ")
            if entry.__name__ not in translator_no_passthrough:
                print("    %s %s = ctx->dispatcher().%s(%s);" % (entry.return_type, entry.__name__ + "RET", entry.__name__, globalCall))
            else:
                print("    %s %s = %s" % (entry.return_type, entry_func_name + "RET", get_fail_code(entry)))
            if entry.__name__ in translator_custom_post:
                print(translator_custom_post[entry.__name__]);
            print("    return %s;" % (entry.__name__ + "RET"));
            if needsShareGroup:
                print("    } else return %s;" % (get_fail_code(entry)))
    print("// Auto-generated with: %s" % banner_command(sys.argv))
    print("// This file is best left unedited.")
    print("// Try to make changes through gen_translator in gen-entries.py,")
    print("// and/or parcel out custom functionality in separate code.")
    for entry in entries:
        print("%sGL_APICALL %s GL_APIENTRY %s(%s) {" % (needExternC(entry), entry.return_type, entry.__name__, entry.parameters))
        gen_cxt_getter(entry);
        gen_validations_custom_impl(entry);
        gen_call_ret(entry);
        print("}\n")
def gen_dll_wrapper(entries, prefix_name, verbatim, filename):
    """Generate a C source file that contains functions that act as wrappers
       for entry points located in another shared library. This allows the
       code that calls these functions to perform lazy-linking to system
       libraries.
       |entries|, |prefix_name|, |verbatim| and |filename| are the same as
       for gen_functions_header() above.
    """
    upper_name = prefix_name.upper()
    ENTRY_PREFIX = "__dll_"
    print("// Auto-generated with: %s" % banner_command(sys.argv))
    print("// DO NOT EDIT THIS FILE")
    print("")
    print("#include <dlfcn.h>")
    for line in verbatim:
        print(line)
    print("")
    print("///")
    print("///  W R A P P E R   P O I N T E R S")
    print("///")
    print("")
    for entry in entries:
        ptr_name = ENTRY_PREFIX + entry.__name__
        print("static %s (*%s)(%s) = 0;" % \
                (entry.return_type, ptr_name, entry.parameters))
    print("")
    print("///")
    print("///  W R A P P E R   F U N C T I O N S")
    print("///")
    print("")
    for entry in entries:
        print("%s %s(%s) {" % \
                (entry.return_type, entry.__name__, entry.parameters))
        ptr_name = ENTRY_PREFIX + entry.__name__
        if entry.return_type != "void":
            print("  return %s(%s);" % (ptr_name, entry.call))
        else:
            print("  %s(%s);" % (ptr_name, entry.call))
        print("}\n")
    print("")
    print("///")
    print("///  I N I T I A L I Z A T I O N   F U N C T I O N")
    print("///")
    print("")
    print("int %s_dynlink_init(void* lib) {" % prefix_name)
    for entry in entries:
        ptr_name = ENTRY_PREFIX + entry.__name__
        print("  %s = (%s(*)(%s))dlsym(lib, \"%s\");" % \
                (ptr_name,
                 entry.return_type,
                 entry.parameters,
                 entry.__name__))
        print("  if (!%s) return -1;" % ptr_name)
    print("  return 0;")
    print("}")
def gen_windows_def_file(entries):
    """Generate a windows DLL .def file. |entries| is a list of Entry instances.
    """
    print("EXPORTS")
    for entry in entries:
        print("    %s" % entry.__name__)
def gen_unix_sym_file(entries):
    """Generate an ELF linker version file. |entries| is a list of Entry
       instances.
    """
    print("VERSION {")
    print("\tglobal:")
    for entry in entries:
        print("\t\t%s;" % entry.__name__)
    print("\tlocal:")
    print("\t\t*;")
    print("};")
def gen_symbols(entries, underscore):
    """Generate a list of symbols from |entries|, a list of Entry instances.
       |underscore| is a boolean. If True, then prepend an underscore to each
       symbol name.
    """
    prefix = ""
    if underscore:
        prefix = "_"
    for entry in entries:
        print("%s%s" % (prefix, entry.__name__))
def parse_file(filename, lines, mode):
    """Generate one of possible outputs from |filename|. |lines| must be a list
       of text lines from the file, and |mode| is one of the --mode option
       values.
    """
    entries, prefix_name, verbatim, namespaces, errors = parse_entries_file(lines)
    if errors:
        for error in errors:
            sys.stderr.write("ERROR: %s:%s" % (filename, error))
        sys.exit(1)
    if not prefix_name:
        prefix_name = "unknown"
    if mode == 'def':
        gen_windows_def_file(entries)
    elif mode == 'sym':
        gen_unix_sym_file(entries)
    elif mode == 'translator_passthrough':
        gen_translator(entries)
    elif mode == 'wrapper':
        gen_dll_wrapper(entries, prefix_name, verbatim, filename)
    elif mode == 'symbols':
        gen_symbols(entries, False)
    elif mode == '_symbols':
        gen_symbols(entries, True)
    elif mode == 'functions':
        gen_functions_header(entries, prefix_name, verbatim, filename, False)
    elif mode == 'funcargs':
        gen_functions_header(entries, prefix_name, verbatim, filename, True)
    elif mode == 'static_translator_namespaced_header':
        gen_static_translator_namespaced_header(entries, namespaces, prefix_name, verbatim, filename, True)
    elif mode == 'static_translator_namespaced_stubs':
        gen_static_translator_namespaced_stubs(entries, namespaces, prefix_name, verbatim, filename, True)
# List of valid --mode option values.
mode_list = [
    'def', 'sym', 'translator_passthrough', 'wrapper', 'symbols', '_symbols', 'functions', 'funcargs', 'static_translator_namespaced_header', 'static_translator_namespaced_stubs'
]
# Argument parsing.
parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="""\
A script used to parse an .entries input file containing a list of function
declarations, and generate various output files depending on the value of
the --mode option, which can be:
  def        Generate a windows DLL .def file.
  sym        Generate a Unix .so linker script.
  wrapper    Generate a C source file containing wrapper functions.
  symbols    Generate a simple list of symbols, one per line.
  _symbols   Generate a simple list of symbols, prefixed with _.
  functions  Generate a C header containing a macro listing all functions.
  funcargs   Like 'functions', but adds function call arguments to listing.
  static_translator_namespaced_header Generate C++ header with namespaced versions of the api declarations.
""")
parser.add_argument("--mode", help="Output mode", choices=mode_list)
parser.add_argument("--output", help="output file")
parser.add_argument("file", help=".entries file path")
args = parser.parse_args()
if not args.mode:
    sys,stderr.write("ERROR: Please use --mode=<name>, see --help.")
    sys.exit(1)
if args.output:
    sys.stdout = open(args.output, "w+")
if args.file == '--':
    parse_file("<stdin>", sys.stdin, args.mode)
else:
    parse_file(args.file, open(args.file), args.mode)