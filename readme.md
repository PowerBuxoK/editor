# Text editor

A basic TUI text editor

## How to build

### Dependencies

#### Windows

- MSYS2
- mingw-w64-ucrt-x86_64-gcc
- mingw-w64-ucrt-x86_64-ncurses
- mingw-w64-ucrt-x86_64-icu
- mingw-w64-ucrt-x86_64-cmake

#### Linux

- gcc
- ncurses
- icu
- cmake

### Building

#### Windows

In source directory:
```
cmake -B ./build/ -S .
cmake --build ./build/
```

#### Linux

In source directory:
```
make
```

##### Output files will be located in ./build/

## TODO

### Urgent

- [x] Secetion-copy-pase
	- [x] Visual (selection mode)
	- [x] Copy
	- [x] Paste
- [x] Buffer juggling
- [x] Fix non-ASCII chars on Windows
- [x] Fix bug with stuck render
- [ ] Optimize it
- [ ] Rewrite how buffers and windows are stored to allow deletion (Maybe sth like an area allocator)
- [ ] Reimplement undo


### Less-than-urgent

- [x] Add automatic layout adjustment
- [x] Add buffer opening with command line arguments

### Optional

- [x] Add I keybind for inserting in start of text
- [ ] Add file explorer
