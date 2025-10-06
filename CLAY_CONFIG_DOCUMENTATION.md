# Documentation on clay configuration files

## General structure

File configuration must contains a key named `clay:` at her root. It's the main key>

```yaml
clay:
  # All configuration keys here
```

## Configuration keys available

### 👔​ `compiler` - The used compiler
```yaml
clay:
  compiler: gcc
```

- `compiler` - [clang] - The used compiler for source files

### 🌍 `env` - Environment variables
```yaml
clay:
  env:
    # Environment variables to define, two exemples
    ITERM_PROFILE: Default
    SHLVL: 3
```

### ⚙️ `options` - Compilation options
```yaml
clay:
  options:
    - -j                # Parallele compilation
    - --verbose         # Verbose mode
    - -l pthread        # Link with `pthread` library
    - -O3               # Full optmization
    - -Wall             # All warnings
    - -Wextra           # Extra warnings
    # ...
```

***May be declared linearly, like this:***
```yaml
clay:
  options: -j --verbose -l pthread -O3 -Wall -Wextra
```

### 📁 `includes_paths` - Inclusion paths
```yaml
clay:
  includes_paths:
    - ./includes/
    - ./thirdparty/lib/include
```

***A linearly declaration is also possible***

### 🔧 `rules` - Custom rules, like a Makefile
```yaml
clay:
  rules:
    rule1:
      description: Rule n1
      cmd: echo 'This is text'
    rule2:
      description: Rule n2
      cmd: |
        echo -n 'This is rule number 2'
        echo 'Executing...'
    watch: # it's an predefined rule, but can be overriden if any `description` or `cmd` is set
```

**All predefined rules :**
- `base` - Compile, this is the default and the only implicit rule when you execute 'kiln'
- `clean` - Delete all object files
- `oclean` - Delete the generated binary/library
- `fclean` - A combinaison of 'clean' and 'oclean'
- `watch` - Watch all files, if any file is modified, it will be re-compile into an objet file and the binary/library will be recompiled
- `re` - A combinaison of 'fclean' then 'base'

### 🗂️ `objects` - Object files generation
```yaml
clay:
  objects:
    path: .objs
    relative: false
    name: _yo_la_team
    extension: obj
    # For a source file named 'algo', the final name will be 'algo_yo_la_team.o'
```

***Default values:***
There is no objects generation at default.
- `path` - [.] - Where object files will be placed
- `relative` - [true] - If 'true', object files will be placed relative to their source files location
- `name:` - [] - Suffixe
- `extension:` - [o] - The file extension

### 🗂️ `headers` - Header files dependence generation
```yaml
clay:
  objects:
    headers:
      path: .objs
      relative: false
      name: _header_dependance
      extension: dep
```

***Default values:***
There is no objects generation at default.
- `path` - [.] - Where object files will be placed
- `relative` - [true] - If 'true', object files will be placed relative to their source files location
- `name:` - [] - Suffixe
- `extension:` - [d] - The file extension

### 📦 `output` - Output configuration
```yaml
clay:
  output:
    kind: bin
    path: .
    name: hasher
```

***Signification:***
- `kind` - [bin] - Can be 'bin' for a binary, 'lib' for a static library, 'shared' for a shared library
- `path` - [.] - The final output path
- `name` - [a.out] - The name of the generated shellcode

### 🔗 `link` - Libraries to link
```yaml
clay:
  link:
    - /lib/libc.a
    - -lpthread
    - ./custom/libmylib.a
```

### 📚 `depends_on` - External dependencies to handle
```yaml
clay:
  depends_on:
    dependence1: # custom dependence name, no importance
      path: ./thirdparty/MEgide
      to_includes: true
      options: -j --verbose
    dependence2:
      path: ./thirdparty/VVenus
      to_includes: true
      options: -j --verbose
```

***More informations:***
~DEPENDENCE MUST HAVE YML config file~
- `path` - !MUST BE PRESENT! - The path where is located the dependence to handle them
- `to_includes` - [true] - Add the dependence config file's includes_paths to the current includes_paths
- `options` - [] - Options with which to compile the dependence

### 📄 `sources` - Source files configuration
```yaml
clay:
  sources:
    raw_paths:                          # Chemins absolus (prefix ignoré)
      - ./main.cpp
      - ./special/file.cpp
    
    prefix: ./FPP/sources/             # Préfixe pour tous les paths
    suffix: .cpp                       # Extension des fichiers
    
    paths:                              # Chemins relatifs (avec prefix)
      - ProgramOptions/Flags           # Devient ./FPP/sources/ProgramOptions/Flags.cpp
      - Symtab/Symtab
      - FileHelpers/File
      - Utils/djb2
```

***More informations:***
- `raw_paths` - [] - All source files where prefix and suffix are not applied
- `prefix` - [] - All source files prefix
- `suffix` - [] - All source files suffix
- `paths` - [] - All source files where prefix and suffix are applied

### 📄 `output_format` - Formating standard output
```yaml
clay:
  output_format:
    colors: true
    messages:
      on_start: 'Starting compilation!'
      on_end: 'Compilation finished!'
      for_each_files: '🔨 Compiling %s...'
      on_error: '❌ Build failed: %s'
      on_success: '✅ Build successful of %s'
    show_commands: false
      # style: classic
    show_stats: true # Compilation time + files number + ...
    script:
      for_each_files: scripts/progress_bar.sh {filename} {total_file_count} {percent} {status}
    where: ./logs
```

***More informations:***
- `color` - [true] - Apply colors on text
- `messages` - [] - Messages printing at time point
- `messages/*` - [] - A custom message if a certain condition
- `show_commands` - [true] - Print each command executed
- `show_stats` - [false] - Print statistics about the compilation, time, files count, ...
- `script` - [] - Execute a script for each file compiled by default
- `script/*` - [] - Execute a script if a certain condition
- `where` - [STDOUT] - In which file print output

***All conditions possibles:***
- `on_error`
- `on_success`
- `for_each_files`
- `on_start`
- `on_end`

<!--
Special symboles :
 '%s' is the error description for `on_error` and it's the filename for `for_each_files`

Variables disponibles dans les scripts :
 {filename}     - main.cpp
 {current}      - 3 (fichier actuel)
 {total}        - 10 (total fichiers)
 {percent}      - 30 (pourcentage)
 {status}       - compiling, linking, done, error
 {elapsed}      - 00:02:15 (temps écoulé)
 {remaining}    - 00:05:30 (temps estimé)
 {output_file}  - ./bin/myapp (fichier de sortie final)
-->

## Real exemple

```yaml
clay:
  env:
    DEBUG: true

  options:
    - -std=c++20
    - -Wall
    - -Wextra
    - -O3

  includes_paths:
    - ./includes/
    - ./thirdparty/include/

  rules:
    test:
      description: "Lance les tests unitaires"
      cmd: "./run_tests.sh"

    install:
      description: "Installe le binaire"
      cmd:
        - "mkdir -p /usr/local/bin"
        - "cp kiln /usr/local/bin/"

  objects:
    output: ./.objs/
    relative: false
    name: .o

  output:
    kind: bin
    path: ./bin/
    name: kiln

  link:
    - -lpthread
    - ./libs/libutils.a

  depends_on:
    hopt:
      path: ./hopt/
      to_includes: true
      options:
        - -j

  sources:
    raw_paths:
      - ./main.cpp

    prefix: ./src/
    suffix: .cpp
    paths:
      - core/engine
      - utils/helpers
      - parsers/yaml
```

## Important notes

1. **Mandatory namespaces** : Key `clay` at the root.
2. **Predefined rules** : Some rules like `fclean` are predefined, but you can override them, entirly or not
3. **Relative paths** : All paths are relative to your YML config file
4. **Execution order** : If different commands in `cmd`, commands will be executed asynchronously

## Usage

```bash
# `clay.yml` is used by default
./kiln

# For more information about options (-h, --help)
./kiln -h

# Will execute the rule1
./kiln rule1

# Will execute the rule1 and then the the rule2
./kiln rule1 rule2
```
