# mmg: make my glap

Preprossessor program to generate to a glap header file from a yaml file which describe arguments for a program.

## Installation

Using [xmake project manager](https://github.com/xmake-io/xmake), add the following xmake lua file :

```lua
add_repositories("tapzcrew-xmake-repo https://github.com/TapzCrew/xmake-repo.git")
add_requires("mmg")
add_requires("glap")

target("your_target")
    -- [...]
    add_packages("glap", "mmg")
    add_rules("@mmg/mmg")
    add_files("my_arguments.glap.yml") -- file stem name can be anything
```

Once your project is configured, you can include `<my_arguments.h>` (name based on the stem of the yaml filename).
Note that everytime you change your yaml, you can xmake config or build your project to update the header.

## Usage

TODO: I'm currently working on an major update for glap. Since the update will impact mmg, I prefer to describe the 
usage once glap will be updated.

