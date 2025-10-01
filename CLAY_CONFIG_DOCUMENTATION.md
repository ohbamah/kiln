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

### 📁 `includes_path` - Inclusion pathes
```yaml
clay:
  includes_path:
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
      conf: '*/30 * * * * *' # This is an exemple of overriden param of `fclean`
```

**All predefined rules :**
- `base` - Compile, this is the default and the only implicit rule when you execute 'kiln'
- `clean` - Delete all object files
- `oclean` - Delete the generated binary/library
- `fclean` - A combinaison of 'clean' and 'oclean'
- `watch` - Watch all files, if any file is modified, it will be re-compile into an objet file and the binary/library will be recompiled
--> The configuration parameter is a cron time value [DEFAULT]: '* * * * * *'
- `re` - A combinaison of 'fclean' then 'base'

### 🗂️ `objects` - Object files generation
```yaml
clay:
  objects:
    output: ./.objs/
     relative: false
    name: _yo_la_team
    extension: obj
    # For a source file named 'algo', the final name will be 'algo_yo_la_team.o'
```

***Default values:***
There is no objects generation at default.
- `output` - [.] - Where object files will be placed
- `output: relative` - [false] - If 'true', object files will be placed relative to their source files location
- `name:` - [] - Suffixe
- `extension:` - [o] - The file extension

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
- `to_includes` - [true] - Add the dependence config file's includes_path to the current includes_path
- `options` - [] - Options with which to compile the dependence

### 📄 `sources` - Source files configuration
```yaml
clay:
  sources:
    raw_path:                          # Chemins absolus (prefix ignoré)
      - ./main.cpp
      - ./special/file.cpp
    
    prefix: ./FPP/sources/             # Préfixe pour tous les paths
    suffix: .cpp                       # Extension des fichiers
    
    path:                              # Chemins relatifs (avec prefix)
      - ProgramOptions/Flags           # Devient ./FPP/sources/ProgramOptions/Flags.cpp
      - Symtab/Symtab
      - FileHelpers/File
      - Utils/djb2
```

***More informations:***
- `raw_path` - [] - All source files where prefix and suffix are not applied
- `prefix` - [] - All source files prefix
- `suffix` - [] - All source files suffix
- `path` - [] - All source files where prefix and suffix are applied

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

  includes_path:
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
    raw_path:
      - ./main.cpp

    prefix: ./src/
    suffix: .cpp
    path:
      - core/engine
      - utils/helpers
      - parsers/yaml
```

## Important notes

1. **Mandatory namespaces** : Key `clay` at the root.
2. **Predefined rules** : Some rules like `fclean` are predefined, but you can override them, entirly or not
3. **Relative pathes** : All pathes are relative to your YML config file
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
