# This top-level build file is included by all modules that implement
# the hardware OpenGL ES emulation for Android.
#
# We use it to ensure that all sub-Makefiles are included in the right
# order for various variable definitions and usage to happen in the correct
# order.
#

# The following macros are used to start a new GLES emulation module.
#
# This will define LOCAL_MODULE as $1, plus a few other variables
# needed by the build system (e.g. LOCAL_MODULE_TAGS, LOCAL_MODULE_CLASS...)
#
# NOTE: You still need to define LOCAL_PATH before this
#
# Usage example:
#
#   $(call emugl-begin-static-library,<name>)
#       LOCAL_SRC_FILES := ....
#       LOCAL_C_INCLUDES += ....
#   $(call emugl-end-module)
#
emugl-begin-static-library = $(call emugl-begin-module,$1,STATIC_LIBRARY)
emugl-begin-shared-library = $(call emugl-begin-module,$1,SHARED_LIBRARY)

# Internal list of all declared modules (used for sanity checking)
_emugl_modules :=
_emugl_HOST_modules :=

# do not use directly, see functions above instead
emugl-begin-module = \
    $(eval include $(CLEAR_VARS)) \
    $(eval LOCAL_MODULE := $1) \
    $(eval LOCAL_MODULE_CLASS := $(patsubst HOST_%,%,$(patsubst %EXECUTABLE,%EXECUTABLES,$(patsubst %LIBRARY,%LIBRARIES,$2)))) \
    $(eval LOCAL_IS_HOST_MODULE := $(if $3,true,))\
    $(eval LOCAL_C_INCLUDES := $(EMUGL_COMMON_INCLUDES)) \
    $(eval LOCAL_CFLAGS := $(EMUGL_COMMON_CFLAGS)) \
    $(eval _EMUGL_INCLUDE_TYPE := $(BUILD_$2)) \
    $(call _emugl-init-module,$1,$2,$3)

# Used to end a module definition, see function definitions above
emugl-end-module = \
    $(eval include $(_EMUGL_INCLUDE_TYPE))\
    $(eval _EMUGL_INCLUDE_TYPE :=) \
    $(eval _emugl_$(_emugl_HOST)modules += $(_emugl_MODULE))\
    $(if $(EMUGL_DEBUG),$(call emugl-dump-module))

# Managing module exports and imports.
#
# A module can 'import' another module, by calling emugl-import. This will
# make the current LOCAL_MODULE inherit various definitions exported from
# the imported module.
#
# Module exports are defined by calling emugl-export. Here is an example:
#
#      $(call emugl-begin-static-library,foo)
#      LOCAL_SRC_FILES := foo.c
#      $(call emugl-export,C_INCLUDES,$(LOCAL_PATH))
#      $(call emugl-export,SHARED_LIBRARIES,libcutils)
#      $(call emugl-end-module)
#
#      $(call emugl-begin-shared-library,bar)
#      LOCAL_SRC_FILES := bar.cpp
#      $(call emugl-import,foo)
#      $(call emugl-end-module)
#
# Here, we define a static library named 'foo' which exports an include
# path and a shared library requirement, and a shared library 'bar' which
# imports it.
#
# What this means is that:
#
#    - 'bar' will automatically inherit foo's LOCAL_PATH in its LOCAL_C_INCLUDES
#    - 'bar' will automatically inherit libcutils in its own LOCAL_SHARED_LIBRARIES
#
# Note that order of declaration matters. If 'foo' is defined after 'bar' in
# the example above, nothing will work correctly because dependencies are
# computed at import time.
#
#
# IMPORTANT: Imports are transitive, i.e. when module A imports B,
#            it automatically imports anything imported by B too.

# This is the list of recognized export types we support for now.
EMUGL_EXPORT_TYPES := \
    CFLAGS \
    LDLIBS \
    LDFLAGS \
    C_INCLUDES \
    SHARED_LIBRARIES \
    STATIC_LIBRARIES \
    ADDITIONAL_DEPENDENCIES

# Initialize a module in our database
# $1: Module name
# $2: Module type
# $3: "HOST" for a host module, empty for a target one.
_emugl-init-module = \
    $(eval _emugl_HOST := $(if $3,HOST_,))\
    $(eval _emugl_MODULE := $(_emugl_HOST)$1)\
    $(if $(filter $(_emugl_$(_emugl_HOST)modules),$(_emugl_MODULE)),\
        $(error There is already a $(if $3,host,) module named $1!)\
    )\
    $(eval _mod = $(_emugl_MODULE)) \
    $(eval _emugl.$(_mod).type := $(patsubst HOST_%,%,$2))\
    $(eval _emugl.$(_mod).imports :=) \
    $(eval _emugl,$(_mod).moved :=) \
    $(foreach _type,$(EMUGL_EXPORT_TYPES),\
        $(eval _emugl.$(_mod).export.$(_type) :=)\
    )

# Called to indicate that a module exports a given local variable for its
# users. This also adds this to LOCAL_$1
# $1: Local variable type (e.g. CFLAGS, LDLIBS, etc...)
# $2: Value(s) to append to the export
emugl-export = \
    $(eval _emugl.$(_emugl_MODULE).export.$1 += $2)\
    $(eval LOCAL_$1 := $2 $(LOCAL_$1))

emugl-export-outer = \
    $(eval _emugl.$(_emugl_MODULE).export.$1 += $2)

# Called to indicate that a module imports the exports of another module
# $1: list of modules to import
#
emugl-import = \
    $(foreach _imod,$1,\
        $(call _emugl-module-import,$(_emugl_HOST)$(_imod))\
    )

_emugl-module-import = \
    $(eval _mod := $(_emugl_MODULE))\
    $(if $(filter-out $(_emugl_$(_emugl_HOST)modules),$1),\
        $(info Unknown imported emugles module: $1)\
        $(if $(_emugl_HOST),\
            $(eval _names := $(patsubst HOST_%,%,$(_emugl_HOST_modules))),\
            $(eval _names := $(_emugl_modules))\
        )\
        $(info Please one of the following names: $(_names))\
        $(error Aborting)\
    )\
    $(if $(filter-out $(_emugl.$(_mod).imports),$1),\
        $(eval _emugl.$(_mod).imports += $1)\
        $(foreach _sub,$(_emugl.$1.imports),\
            $(call _emugl-module-import,$(_sub))\
        )\
        $(foreach _type,$(EMUGL_EXPORT_TYPES),\
            $(eval LOCAL_$(_type) := $(_emugl.$1.export.$(_type)) $(LOCAL_$(_type)))\
        )\
        $(if $(filter EXECUTABLE SHARED_LIBRARY,$(_emugl.$(_emugl_MODULE).type)),\
            $(if $(filter STATIC_LIBRARY,$(_emugl.$1.type)),\
                $(eval LOCAL_STATIC_LIBRARIES := $(1:HOST_%=%) $(LOCAL_STATIC_LIBRARIES))\
            )\
            $(if $(filter SHARED_LIBRARY,$(_emugl.$1.type)),\
                $(if $(_emugl.$1.moved),,\
                  $(eval LOCAL_SHARED_LIBRARIES := $(1:HOST_%=%) $(LOCAL_SHARED_LIBRARIES))\
                )\
            )\
        )\
    )

_emugl-dump-list = \
    $(foreach _list_item,$(strip $1),$(info .    $(_list_item)))

emugl-dump-module = \
    $(info MODULE=$(_emugl_MODULE))\
    $(info .  HOST=$(_emugl_HOST))\
    $(info .  TYPE=$(_emugl.$(_emugl_MODULE).type))\
    $(info .  IMPORTS=$(_emugl.$(_emugl_MODULE).imports))\
    $(foreach _type,$(EMUGL_EXPORT_TYPES),\
        $(if $(filter C_INCLUDES ADDITIONAL_DEPENDENCIES,$(_type)),\
            $(info .  EXPORT.$(_type) :=)\
            $(call _emugl-dump-list,$(_emugl.$(_emugl_MODULE).export.$(_type)))\
            $(info .  LOCAL_$(_type)  :=)\
            $(call _emugl-dump-list,$(LOCAL_$(_type)))\
        ,\
            $(info .  EXPORT.$(_type) := $(strip $(_emugl.$(_emugl_MODULE).export.$(_type))))\
            $(info .  LOCAL_$(_type)  := $(strip $(LOCAL_$(_type))))\
        )\
    )\
    $(info .  LOCAL_SRC_FILES := $(LOCAL_SRC_FILES))\

# This function can be called to generate the wrapper source files.
# LOCAL_MODULE and LOCAL_MODULE_CLASS must be defined or the build will abort.
# Source files will be stored in the local intermediates directory that will
# be automatically added to your LOCAL_C_INCLUDES.
# Usage:
#    $(call emugl-gen-wrapper,<input-dir>,<basename>)
#
emugl-gen-wrapper = \
    $(eval _emugl_out := $(call local-intermediates-dir)) \
    $(call emugl-gen-wrapper-generic,$(_emugl_out),$1,$2) \
    $(call emugl-export,C_INCLUDES,$(_emugl_out))

# DO NOT CALL DIRECTLY, USE emugl-gen-wrapper instead.
#
# The following function can be called to generate GL library wrapper
# Usage is:
#
#  $(call emugl-gen-wrapper-generic,<dst-dir>,<src-dir>,<basename>)
#
#  <dst-dir> is the destination directory where the generated sources are stored
#  <src-dir> is the source directory where to find <basename>.attrib, etc..
#  <basename> is the emugen basename (see host/tools/emugen/README)
#
emugl-gen-wrapper-generic = $(eval $(emugl-gen-wrapper-generic-ev))

define emugl-gen-wrapper-generic-ev
_emugl_wrap := $$1/$$3
_emugl_src  := $$2/$$3
GEN := $$(_emugl_wrap)_wrapper_entry.cpp \
       $$(_emugl_wrap)_wrapper_context.cpp \
       $$(_emugl_wrap)_wrapper_context.h \
       $$(_emugl_wrap)_wrapper_proc.h

$$(GEN): PRIVATE_PATH := $$(LOCAL_PATH)
$$(GEN): PRIVATE_CUSTOM_TOOL := $$(EMUGL_EMUGEN) -W $$1 -i $$2 $$3
$$(GEN): $$(EMUGL_EMUGEN) $$(_emugl_src).attrib $$(_emugl_src).in $$(_emugl_src).types
	$$(transform-generated-source)

$$(call emugl-export,ADDITIONAL_DEPENDENCIES,$$(GEN))
LOCAL_GENERATED_SOURCES += $$(GEN)
LOCAL_C_INCLUDES += $$1

#ifneq ($$(HOST_OS),windows)
$$(call emugl-export,LDFLAGS,-ldl)
#endif

endef

# Call this function when your shared library must be placed in a non-standard
# library path (i.e. not under /system/lib
# $1: library sub-path,relative to /system/lib
# For example: $(call emugl-set-shared-library-subpath,egl)
emugl-set-shared-library-subpath = \
    $(eval LOCAL_MODULE_RELATIVE_PATH := $1)\
    $(eval _emugl.$(LOCAL_MODULE).moved := true)
