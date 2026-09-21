# co

`co` is an asynchronous runtime with a versioned C API and C++ wrappers.

## Building the documentation

The documentation is built with [Doxygen](https://www.doxygen.nl/), [Sphinx](https://www.sphinx-doc.org/) and [Breathe](https://breathe.readthedocs.io/).
Doxygen extracts the API reference from the headers in `include/`, Breathe brings it into Sphinx, and Sphinx generates the HTML site.
The sources are in [`docs/`](docs).

### Prerequisites

- **Python 3.11 or newer**
- **Doxygen**
- **Graphviz** (optional, only needed for Doxygen diagrams)

On Windows they can be installed with `winget`:

```powershell
winget install --id Python.Python.3.13 -e
winget install --id DimitriVanHeesch.Doxygen -e
winget install --id Graphviz.Graphviz -e
```

Restart the terminal (or Visual Studio) afterwards so the new tools are on `PATH`, then check with `doxygen --version`.

On Linux use the distribution packages, e.g. `sudo apt-get install python3 python3-venv doxygen graphviz`.

### Python virtual environment

Create the virtual environment in `.venv` in the repository root and install the documentation dependencies into it.
The CMake `docs` target looks for `sphinx-build` in this location.

Windows (PowerShell):

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install --upgrade pip
.\.venv\Scripts\python.exe -m pip install -r docs/requirements.txt
```

Linux / macOS:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install --upgrade pip
.venv/bin/python -m pip install -r docs/requirements.txt
```

`.venv/` is already listed in `.gitignore`.

### Building with CMake

The `docs` target is only added when the `CO_BUILD_DOCS` option is enabled (it is `OFF` by default):

```powershell
cmake -S . -B out/build/docs -G Ninja -DCO_BUILD_DOCS=ON
cmake --build out/build/docs --target docs
```

The HTML output is written to `<build directory>/docs/html`; open `index.html` in a browser.

In Visual Studio, add `CO_BUILD_DOCS=ON` to the CMake cache variables of your configuration, let CMake reconfigure, and build the `docs` target.

The virtual environment does not need to be activated for the CMake target, because it runs `sphinx-build` by its full path.

### Building without CMake

```powershell
.\.venv\Scripts\Activate.ps1
sphinx-build -b html docs docs/_build/html
```

`docs/conf.py` runs Doxygen automatically, so this single command builds everything. The output is in `docs/_build/html`.

### Troubleshooting

- **`Could NOT find Doxygen`**: Doxygen is not on `PATH`. Restart the terminal / Visual Studio after installing it, or pass `-DDOXYGEN_EXECUTABLE=<path to doxygen>` when configuring.
- **`Could not find SPHINX_BUILD_EXECUTABLE`**: the virtual environment is missing or not in `.venv`. Create it as described above, or pass `-DSPHINX_BUILD_EXECUTABLE=<path to sphinx-build>` when configuring.
- **`Activate.ps1 cannot be loaded because running scripts is disabled`**: run `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned` once, or call the tools by their full path (`.\.venv\Scripts\sphinx-build.exe`) instead of activating the environment.

### Hosting

- **Read the Docs** builds the documentation using [`.readthedocs.yaml`](.readthedocs.yaml).
- **GitHub Pages** is deployed from `main` by the [`Docs`](.github/workflows/docs.yml) workflow. In the repository settings under *Pages*, set *Source* to *GitHub Actions*.
