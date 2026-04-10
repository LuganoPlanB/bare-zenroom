# bare-addon-zenroom

Bare native addon for embedding Zenroom.

This repository starts from the standard Bare native addon template and
is being adapted gradually rather than reworked all at once.

The current v0 target is intentionally small:

- expose one execution-oriented API
- map it onto Zenroom's public C API
- keep the addon thin and explicit

The first public JS boundary is:

```js
const zenroom = require('.')

const result = zenroom.exec(script, {
  conf,
  keys,
  data,
  extra,
  context
})
```

The planned result shape is:

```js
{
  exitCode: Number,
  stdout: String,
  stderr: String
}
```

The local development path links the addon against a Zenroom shared
library built from a plain local checkout in [`./zenroom`](./zenroom).

`bare-make` itself is left untouched. The practical local workflow is
integrated in this repository through npm scripts that build Zenroom
first and then run the normal Bare addon steps.

## Non-goals for v0

- no direct hash or sign helper APIs yet
- no async worker model yet
- no streaming stdout/stderr yet
- no full Zenroom source vendoring yet

## Building

<https://github.com/holepunchto/bare-make> is used for compiling the native bindings in [`binding.c`](binding.c). Start by installing the tool globally:

```console
npm i -g bare-make
```

Next, generate the build system for compiling the bindings, optionally setting the `--debug` flag to enable debug symbols and assertions:

```console
bare-make generate [--debug]
```

This only has to be run once per repository checkout. When updating `bare-make` or your compiler toolchain it might also be necessary to regenerate the build system. To do so, run the command again with the `--no-cache` flag set to disregard the existing build system cache:

```console
bare-make generate [--debug] --no-cache
```

With a build system generated, the bindings can be compiled:

```console
bare-make build
```

This will compile the bindings and output the resulting shared library module to the `build/` directory. To install it into the `prebuilds/` directory where the Bare addon resolution algorithm expects to find it, do:

```console
bare-make install
```

To make iteration faster during development, the shared library module can also be linked into the `prebuilds/` directory rather than copied. To do so, set the `--link` flag:

```console
bare-make install --link
```

Prior to publishing the module, make sure that no links exist within the `prebuilds/` directory as these will not be included in the resulting package archive.

## Local status

The addon is intended to call `zencode_exec_tobuf(...)` from Zenroom.
The local Zenroom source is expected to live inside this repository,
not as a submodule.

Clone it with:

```console
git clone git@github.com:dyne/zenroom.git zenroom
```

For local development the expected flow is:

```console
cd zenroom
make linux-lib

bare-make generate
bare-make build
bare-make install --link
```

Equivalent repo-local wrappers are available:

```console
npm run zenroom:build
npm run addon:generate
npm run addon:build
npm run addon:install
```

And the practical end-to-end commands are:

```console
npm run dev:prepare
npm run dev:test
```

What they do:

- `zenroom:build`: detects the host platform and selects the matching Zenroom library target
- `addon:generate`: uses the normal Bare toolchain on Unix targets and selects the MinGW toolchain on Windows
- `dev:prepare`: builds Zenroom, generates the Bare build tree, builds the addon, and installs the prebuild link
- `dev:test`: runs the full local preparation flow and then executes `npm test`

Current platform mapping for `npm run zenroom:build`:

- Linux: `linux-lib`
- macOS: `osx-lib`
- Windows: `win-dll`

Architecture is detected and reported as part of the build context. For
these three targets the Zenroom Make target is platform-specific rather
than arch-specific, so the current mapping keys off the OS and lets the
native toolchain handle the host architecture.

If you need to force a different Zenroom Make target, set
`ZENROOM_BUILD_TARGET`:

```console
ZENROOM_BUILD_TARGET=posix-lib npm run zenroom:build
```

This is a local-first setup. Packaging and portable prebuild generation
will be tightened later once the addon/native boundary is stable.

For Windows, the current CI and local addon generation path targets
`win32-x64` using the MinGW toolchain. This matches Zenroom's own
`win-dll` build target, which is currently x64-specific.

## Runtime dependency placement

The addon now arranges for the platform-specific Zenroom shared library
to sit in the runtime
directory already searched by the addon loader:

- Linux: `libzenroom.so`
- macOS: `libzenroom.dylib`
- Windows: `zenroom.dll` with import library `libzenroom_dll.lib`

Examples:

- after `bare-make build` on Linux:
  `build/bare-addon-zenroom/libzenroom.so`
- after `bare-make build` on macOS:
  `build/bare-addon-zenroom/libzenroom.dylib`
- after `bare-make install --link` on Linux:
  `prebuilds/linux-x64/bare-addon-zenroom/libzenroom.so`

This removes the need for `LD_LIBRARY_PATH` during the normal local
workflow.

## Publishing

To publish an addon, make sure to first compile bindings for the targets you wish to support. The prebuild workflow defined in [`.github/workflows/prebuild.yml`](.github/workflows/prebuild.yml) automates this process for all [tier 1 targets](https://github.com/holepunchto/bare#platform-support) supported by Bare. The whole process can be handily orchestrated by the [GitHub CLI](https://cli.github.com). As the package version is part of the compiled bindings, make sure to first commit and push a version update:

```console
npm version <increment>
git push
git push --tags
```

To start the prebuild workflow for the newly pushed version, do:

```console
gh workflow run prebuild --ref <version>
```

To watch the status of the workflow run until it finishes, do:

```console
gh run watch
```

When finished, the resulting prebuilds can be downloaded to the `prebuilds/` directory by doing:

```console
gh run download --name prebuilds --dir prebuilds
```

> [!IMPORTANT]
> You still need to manually run `npm pub` to publish the package to npm.

## Native dependencies

Addons are rarely self-contained and most often need to pull in
external native libraries. For this, <https://github.com/holepunchto/cmake-fetch>
can be used when the addon starts importing third-party native build
inputs from CMake.

Install it as a development dependency:

```console
npm i -D cmake-fetch
```

Next, import the package in the [`CMakeLists.txt`](CMakeLists.txt) build definition:

```cmake
find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
```

This will make the `fetch_package()` function available. To fetch an
external native library, such as <https://github.com/holepunchto/liburl>,
add the following line _after_ the `project()` declaration in the build
definition:

```cmake
fetch_package("github:holepunchto/liburl")
```

Finally, link the imported native library to the addon:

```cmake
target_link_libraries(
  ${bare_addon}
  PUBLIC
    url
)
```

## Troubleshooting

### Local changes not being reflected after installation

The `bare` CLI statically links built-in native addons using `link_bare_module()` in [`bare/bin/CMakeLists.txt`](https://github.com/holepunchto/bare/blob/main/bin/CMakeLists.txt). If you are working on one of those, `bare` may be loading a cached version of it.

To check the current cache state, do:

```sh
bare --print 'Bare.Addon.cache'
```

To bypass this issue during development, manually bump the `version` field in the `package.json` to invalidate the cache.

## License

Apache-2.0
