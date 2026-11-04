# Installed component discovery

Base installs `FindStormByte.cmake`, `StormByteCommon.cmake` and its generated
`StormByteCoreDependencies.cmake` in `${CMAKE_INSTALL_DATAROOTDIR}/cmake/Modules`.
Add this directory to `CMAKE_MODULE_PATH` and the installation prefixes to
`CMAKE_PREFIX_PATH`. The consumer interface remains:

```cmake
find_package(StormByte REQUIRED COMPONENTS Base Network)
target_link_libraries(app PRIVATE StormByte::Base StormByte::Network)
```

Base discovers only its own library and provides shared discovery helpers.
Every other suite library must **create and install its own finder**, in the
same module directory, and include it in every binary release ZIP:

| Repository | Installed finder | Direct component dependencies |
| --- | --- | --- |
| Logger | `FindStormByteLogger.cmake` | none |
| System | `FindStormByteSystem.cmake` | none |
| Config | `FindStormByteConfig.cmake` | none |
| Database | `FindStormByteDatabase.cmake` | Logger |
| Buffer | `FindStormByteBuffer.cmake` | Logger, System |
| Crypto | `FindStormByteCrypto.cmake` | Buffer, Logger, System |
| Network | `FindStormByteNetwork.cmake` | Buffer, Crypto, Logger, System |
| Multimedia | `FindStormByteMultimedia.cmake` | Buffer, Logger, System |

All components link Base automatically. Each component owns its dependencies;
verify them against its public link interface when maintaining its finder.
For example, based on its current direct public links, Network's finder is:

```cmake
stormbyte_find_component(Network DEPENDS Buffer Crypto Logger System)
```

A component without additional dependencies uses
`stormbyte_find_component(Logger)`. The helper requires both the component
headers and library, recursively loads dependency finders, creates
`StormByte-<Component>` and `StormByte::<Component>`, and applies the shared-library
visibility definition. Missing component finders, headers, libraries or
dependencies make that component unavailable. Older component releases without
their own finder are not supported.

On Windows, the finder distinguishes MSVC `.lib` import libraries from static
archives using the COFF linker index, and recognizes MinGW `.dll.a` import
libraries. An import descriptor matching the `.lib` basename identifies a
shared library even when the DLL is installed separately; a static archive may
contain import descriptors for other DLLs without becoming shared. Archives
without a recognizable linker index are rejected. For renamed or otherwise
ambiguous archives, set `STORMBYTE_LIBRARY_IS_SHARED` for Base or
`STORMBYTE_<Component>_LIBRARY_IS_SHARED` for a component to `ON` or `OFF`.
The DLLs must still be available when the application runs.

The public result variables remain `StormByte_FOUND`, `STORMBYTE_FOUND`,
`STORMBYTE_INCLUDE_DIR`, `STORMBYTE_LIBRARY`, `StormByte_<Component>_FOUND` for
requested components, and `STORMBYTE_<Component>_LIBRARY`.

Before releasing each component, test `find_package` against its installed prefix
and an extracted release ZIP, including transitive dependencies. Alchemist only
consumes these files; it must not duplicate component discovery.
