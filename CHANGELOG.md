# VortexConfig Change Log

All notable changes to this project will be documented in this file.

## [Unreleased]

### Added

- Standard key-value suport
- Array support
- Object support
- Basic comment support
- String and Node retrieval
- All the basic functionality of a parser
 
### Changed
 
### Fixed


## [0.1] - 2024-06-18

### Added

- All the features of the unreleased version
 
### Changed
 
### Fixed

- Improper array parsing code where the comma next to a value that wasn't in quotes was included in the returned value


## [0.2] - 2024-06-18

### Added

- Integer values support
- Floating point values support
- Boolean values support
 
### Changed

- The way configuration files are opened (from a+b mode to rb mode [basically from read and write to read only mode])

### Fixed


## [0.3] - 2024-06-26

### Added

- Full C++ support (C++ wrappers)
- C++ polymorphic functions. ```GetString()``` and other functions work with both node pointer and string arguments
 
### Changed

- Function prefixes changed from ```cfv_``` to ```vcfg_``` + minor changes in structure names
- Calling convention in C has changed. Instead of using ```vcfg_parse();``` we have to create the parser object and call ```vcfg_parse(objectPointer);```. This applies to all the library calls
- Library structure. Instead of one huge header file the source now consists of smaller header files that make it easier to navigate between the lines of code

### Fixed