Separation of responsibilities is a key factor to determine maintainability of software systems.
This PR implements basic modularization into static libraries to improve code quality.

During an initial analysis packages and their project internal dependencies are defined.\
Packages in this context are names of subfolders in `src/anbox`.
Only package `common` does not havy any other dependencies and it would make sense to start with it.
The next packages would be `network` and `rpc`. These are still relatively easy to implement, as they are lower level packges with basic functionality. Excerpt from the dependency tree:
```
"common": [],
"network": [ "common" ],
"rpc": [ "common", "network" ],
```

During the implementation ring dependencies will have to be resolved, as for example package `wm` depends on `platform`, but `platform` also requires functionality from `wm`.
