
# create a package dependency tree
# each source files is captured with the include directives
# then the source package and included package are connected
# the result is a tree with three levels, but can be propagated
#   NOTE: due to ring dependencies, the propagation is limited to
#         four additional levels.
#         That should give adequate insight for now.


import os
pwd = os.path.dirname(__file__)
# walk up directories until git repository is found
while not os.path.exists(f"{pwd}/.git"):
    pwd = os.path.dirname(pwd)
    print(pwd)

src_dir = f"{pwd}/src/anbox"

src_files = []
for root, _, files in os.walk(src_dir):
    for file in files:
        if file.endswith(".cpp") or file.endswith(".h"):
            src_files.append(f"{root}/{file}")

# parse files into dict:
#   file_includes = { 
#     "src_file.h/cpp" : [ 
#       "incl_file.h", ...
#     ], ...
#   }
file_includes = {}
for file in src_files:
    file_includes[file] = []
    with open(file) as fp:
        for line in fp.readlines():
            if line.startswith("#include \""):
                # cut line to include content
                # #include "anbox/daemon.h"
                #           ^^^^^^^^^^^^^^
                file_includes[file].append(line[10:-2])


def top_level_dir(file_path: str) -> str:
    if file_path.startswith("anbox/"):
        file_path= file_path[len("anbox/"):]
    
    
    if "/" not in file_path:
        return ""
    index_of_slash = file_path.index("/")
    return file_path[:index_of_slash]


# create single level dependency tree:
#   source module: [ dest module, * ]
dependency_tree = { "external": []}
for fi in file_includes:
    src_mod = top_level_dir(fi[len(src_dir) + 1:])
    if len(src_mod) == 0:
        src_mod = "empty" # probably root level?

    if src_mod not in dependency_tree:
        dependency_tree[src_mod] = []
    
    for incl in file_includes[fi]:
        
        dest_mod = top_level_dir(incl)
        
        if len(dest_mod) == 0:
            continue
        elif dest_mod in dependency_tree[src_mod]:
            continue        
        elif dest_mod == src_mod:
            continue
        else:
            dependency_tree[src_mod].append(dest_mod)    
            
print(dependency_tree)

