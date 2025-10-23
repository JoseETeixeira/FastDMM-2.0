# Product Overview

This repository contains two related BYOND development tools:

## FastDMM (Java)

A flexible alternative map editor for BYOND with advanced features:

- Specific display filters beyond basic object/mob/area/turf
- Directional object placement
- Support for icons larger than 32x32
- Plane support
- Minimizes diffs and prevents merge conflicts through key randomization
- Random tile placement and adjacent tile tools
- Prefab system for reusable map pieces

## DMCompiler (C++)

A C++ port of the OpenDream DM compiler and disassembler:

- Compiles BYOND Dream Maker files (.dme/.dm) to JSON bytecode
- Disassembles compiled JSON back to readable format
- Full preprocessor support (#define, #include, #if, etc.)
- Complete lexer, parser, and bytecode generation
- Map file (.dmm) support

Both tools are designed to improve the BYOND development workflow - FastDMM for visual map editing and DMCompiler for code compilation and analysis.
