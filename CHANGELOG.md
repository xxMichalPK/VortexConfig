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