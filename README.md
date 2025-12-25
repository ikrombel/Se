# Se

Native framework for develop programs and gamedev

Support platform: Android, Linux, Windows

### Dependences

- basic: `LZ4` *[static libs]*
- win32 `user32 winmm`

### CMake options

| Option        | Default value |Using in lib | Description                                             |
|:--------------|:-------------:|:------------|---------------------------------------------------------|
|SE_THREADING   | ON            | Se          |Enable multi theading                                    |
|SE_SSE         | OFF           | SeMath      |Enabled SSE.                                             |
|SE_FILEWATCHER | ON            | SeVFS       | If `ON` that automatically detects when a file changes. |

### Classes

- **Se::AbstractFile** - A common root class for objects (`MemoryBuffer, File, VectorBuffer, MemoryBuffer`) that implement both Serializer and Deserializer.
- **Se::FileSystem** - Subsystem for file and directory operations and access control.
- **Se::String** - like `std::string` with additional functional
- **Se::PackageFile** - Stores files of a directory tree sequentially for convenient access. File extension: `*.pak`
- ...

<br>

# SeVFS

Dependences: `Se`

### Classes

- **Se::FileIdentifier** - File identifier, similar to Uniform Resource Identifier (URI).
- **Se::FileWatcher** - Watches a directory and its subdirectories for files being modified.
- **Se::MountPoint** - Access to engine file system mount point.
  - **Se::MountedAliasRoot** - Mount point that provides named aliases to other mount points.
  - **Se::MountedDirectory** - Stores files of a directory tree sequentially for convenient access.
  - **Se::MountedExternalMemory** - Lightweight mount point that provides read-only access to the externally managed memory.
  - **Se::MountedPackageFile** - Include PackageFile to virtual file system
  - **Se::MountedRoot** - Lightweight mount point that provides read-only access to the externally managed memory.
- **Se::VirtualFileSystem** - Subsystem for virtual file system.

<br>

# SeMath

Headeronly math library for 2D/3D
Dependences: `Se`

Definition

| Define            | Default | Description                         |
|-------------------|---------|-------------------------------------|
| SE_MATH_STANALONE | undef   | Using for stay alone without Se lib |
| USE_SSE           | undef   | Enable SSE                          |

<br>

# SeArc

Headeronly library for serialize/deserialize structure to another formats
Dependences: `Se`

<br>

# SeResource

### Dependences

- basic: `Se SeArc SeVFS` `ETCPACK nanosvg PugiXml rapidjson rapidyaml-0.7.2 STB` *[static libs]*

### Classes

- **Se::ArchiveBase** - SeArc archive implementation
  - **Se::Base64InputArchive**, **Base64OutputArchive** - I/O archive for Base64 serialization.
  - **Se::BinaryInputArchive**, **BinaryOutputArchive** - I/O archive for binary serialization.
  - **Se::JSONInputArchive**, **JSONOutputArchive** - I/O archive for JSON serialization.
  - **Se::XMLOutputArchive** - I/O archive for XML serialization.
  - `TODO` **Se::YAMLInputArchive**, **Se::YAMLOutputArchive**.
- **Se::Resource** - Base class for resources.
  - **Se::BinaryFile** - Resource for generic binary file.
  - **Se::Image** - Image resource.
    - **Se::ImageSVG** - SVG image resource.
  - **Se::JSONFile** - JSON document resource.
  - **Se::XMLFile** - XML document resource.
  - **Se::YAMLFile** - YAML document resource.
- **Se::ResourceCache** - Resource cache subsystem. Loads resources on demand and stores them for later access.
- **Se::Localization** - Localization subsystem. Stores all the strings in all languages.
- **Se::LocalizationFile** - This is an optional feature for Serialize/Deserialize user's file format.

<br>

# SePlatform `TODO`

### Classes

- **Application**


<br>

# Using

use command for clone to inside your project

```bash
git clone https://github.com/ikrombel/Se.git
```

or, add to cmake:

```cmake
set(SE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/.cache_deps/Se)
if(NOT EXISTS ${SE_SOURCE_DIR} AND NOT TARGET Se)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/.cache_deps)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/.cache_deps)
    endif()
    execute_process(
        COMMAND git clone https://github.com/ikrombel/Se.git ${SE_SOURCE_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
```

Example for linking to :

Do not need link this lib it is wrapped:

- `ETCPACK nanosvg PugiXml rapidjson rapidyaml-0.7.2 STB`

```cmake
add_executable(${YOUR_EXE_NAME} ${YOUR_SOURCE_FILES})

target_link_libraries(${YOUR_EXE_NAME} PUBLIC
    Se
    SeResource
    SeVFS
)
```

<br>

# LICENCE `MIT`
