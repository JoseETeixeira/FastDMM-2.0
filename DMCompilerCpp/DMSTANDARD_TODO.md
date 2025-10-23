# DMStandard Library Completion Plan

## Overview

The DMStandard library in DMCompilerCpp is incomplete. Many built-in procs, type definitions, and constants are missing or have incorrect signatures. This document outlines the work needed to bring DMStandard to parity with the BYOND DM language specification.

## Reference Documentation

The complete DM language reference is located in `dm_open_ref/ref/`. This should be used as the authoritative source for:

- Proc signatures and return types
- Type definitions and their members
- Constants and defines
- Default parameter values

## Current State Analysis

### What Exists

**Global Procs** (in `_Standard.dm`):

- Basic math: ceil, floor, round, fract, sign, clamp, lerp
- Text manipulation: copytext, findtext, replacetext, lowertext, splittext, etc.
- Type checking: isarea, isfile, isicon, islist, ismob, isobj, isturf, isnum, istext, isnull
- File operations: file, file2text, fexists, fcopy, fdel, flist, ftime
- Spatial: block, bounds_dist, get_step_to, get_steps_to, range, orange, oview
- View: hearers, ohearers, oviewers
- List: list2params, params2list
- Encoding: html_encode, html_decode, md5, sha1, rgb, rgb2num
- JSON: json_encode, json_decode
- System: alert, animate, CRASH, shutdown, sleep, stat, statpanel
- Random: rand, rand_seed, roll, generator, gradient
- Misc: ckey, ckeyEx, ref, refcount, sound, image, filter, icon_states

**Type Definitions** (in `Types/`):

- /datum, /atom, /turf, /area, /mob, /obj (in Atoms/)
- /client, /world, /list, /savefile
- /icon, /image, /sound, /matrix, /regex
- /database, /exception, /mutable_appearance
- /filter, /generator, /particles, /pixloc, /vector

**Constants** (in `Defines.dm`):

- Direction constants (NORTH, SOUTH, EAST, WEST, etc.)
- Various system constants

### What's Missing

#### Critical Missing Procs (High Priority)

**Math Functions**:

- abs(x) - absolute value
- sqrt(x) - square root
- sin(x), cos(x), tan(x) - trigonometric functions
- arcsin(x), arccos(x), arctan(x) - inverse trig
- log(x, base) - logarithm
- trunc(x) - truncate to integer

**Text Functions**:

- addtext(...) - concatenate text
- uppertext(T) - convert to uppercase
- trimtext(T) - trim whitespace
- text2num(T) - convert text to number
- text2path(T) - convert text to path
- text2file(File, Text) - write text to file
- time2text(timestamp, format) - format timestamp
- REGEX_QUOTE(text) - escape regex special chars
- nameof(x) - get name of variable/proc

**List Functions**:

- pick(...) - randomly select from arguments
- prob(P) - probability check
- typesof(path) - get all subtypes
- newlist(...) - create new list with values
- jointext(List, Glue) - join list elements (exists but may need verification)
- lentext(E) - get text length (exists but may need verification)

**Movement/Spatial**:

- get_dir(Loc1, Loc2) - get direction between locations
- get_dist(Loc1, Loc2) - get distance between locations
- get_step(Ref, Dir, Step) - get adjacent turf
- get_step_towards(Ref, Trg) - step towards target
- get_step_away(Ref, Trg, Max) - step away from target
- get_step_rand(Ref) - random adjacent turf
- step(Ref, Dir, Speed) - move in direction
- step_to(Ref, Trg, Min) - move towards target
- step_towards(Ref, Trg, Min) - move towards target
- step_away(Ref, Trg, Max) - move away from target
- step_rand(Ref) - move randomly
- walk(Ref, Dir, Lag, Speed) - continuous movement
- walk_to(Ref, Trg, Min, Lag, Speed) - walk to target
- walk_towards(Ref, Trg, Lag, Speed) - walk towards
- walk_away(Ref, Trg, Max, Lag, Speed) - walk away
- walk_rand(Ref, Lag, Speed) - random walk

**View/Range**:

- view(Dist, Center) - get visible atoms
- viewers(Depth, Center) - get visible mobs
- bounds(atom/Ref, dist) - atoms in bounds
- obounds(atom/Ref, dist) - atoms outside bounds
- bound_pixloc(atom/Ref, dx, dy) - pixel location in bounds

**Type Checking**:

- istype(Object, Type) - check if object is type (CRITICAL - used everywhere)
- ispointer(x) - check if value is pointer
- issaved(x) - check if var is saved
- initial(x) - get initial value of variable

**I/O and Network**:

- browse(Body, Options) - display HTML to client
- browse_rsc(File, FileName) - send resource to client
- output(Msg, Control) - output to interface control
- ftp(File, Name) - send file via FTP
- link(url) - create clickable link
- load_resource(File) - load external resource
- call_ext(library, function) - call external DLL
- load_ext(library) - load external DLL

**System/Control**:

- startup() - called when world starts
- run(file) - execute DM file
- shell(command) - execute shell command
- call(proc, ...) - dynamically call proc
- arglist(args) - convert args to list
- ASSERT(condition) - assertion check
- EXCEPTION(message) - throw exception

**Icon/Image**:

- icon(icon, icon_state, dir, frame, moving) - create icon
- pixloc(x, y, z) - create pixel location
- vector(x, y, z) - create vector
- matrix(...) - create transformation matrix
- regex(pattern, flags) - create regex object

**URL/Encoding**:

- url_encode(text) - URL encode
- url_decode(text) - URL decode

**Window/Interface**:

- winshow(player, window, show) - show/hide window
- winset(player, control, params) - set control properties
- winget(player, control, params) - get control properties
- winexists(player, control) - check if control exists
- winclone(player, window, clone_name) - clone window
- set_background(mode) - set background mode

### Signature Issues

Many existing procs may have incorrect:

- Parameter names (should match BYOND documentation)
- Return types (should use "as type" syntax correctly)
- Default values (should match BYOND defaults)
- Optional parameters (some may be missing)

**Example Issues to Check**:

- Does `rand()` have the correct signature?
- Are all `is*()` procs returning `as num`?
- Do text manipulation procs have correct return types?
- Are optional parameters marked correctly?

## Implementation Strategy

### Phase 1: Critical Procs (Tasks 18-19)

Add the most commonly used missing procs:

- Math functions (abs, sqrt, trig functions)
- Text manipulation (addtext, uppertext, text2num, text2path)
- Type checking (istype - CRITICAL)

### Phase 2: Movement and Spatial (Tasks 20-22)

Add movement and spatial awareness procs:

- List manipulation (pick, prob, typesof)
- Movement procs (step, walk, get_step, etc.)
- View/range procs (view, viewers, bounds)

### Phase 3: I/O and System (Tasks 23-25)

Add I/O, network, and system control:

- Type checking completion
- I/O and network procs
- System/control procs

### Phase 4: Specialized (Tasks 26-28)

Add specialized functionality:

- Icon/image creation
- URL encoding
- Window/interface control

### Phase 5: Verification (Tasks 29-32)

Verify and fix existing implementations:

- Fix incorrect signatures
- Complete type definitions
- Add missing constants
- Comprehensive testing

## Testing Approach

For each added proc:

1. Add the proc signature to `_Standard.dm`
2. Mark with `set opendream_unimplemented = 1` if not implemented in runtime
3. Add appropriate return type using "as type" syntax
4. Document any deviations from BYOND behavior
5. Create test cases in test files

## Priority Ranking

**P0 (Critical - Blocks Compilation)**:

- istype() - used in almost every DM codebase
- get_dist() - used in spatial calculations
- get_dir() - used in movement code
- step() family - basic movement
- view() family - visibility checks

**P1 (High - Common Usage)**:

- Math functions (abs, sqrt, trig)
- Text manipulation (addtext, uppertext, text2num)
- pick() and prob() - random selection
- walk() family - AI movement

**P2 (Medium - Moderate Usage)**:

- I/O procs (browse, output)
- URL encoding
- Window control
- Advanced type checking

**P3 (Low - Rare Usage)**:

- External DLL calls
- Shell commands
- Specialized icon manipulation

## Notes

- All procs should be marked with `set opendream_unimplemented = 1` if the runtime doesn't implement them yet
- Return types should use the "as type" syntax we just implemented
- Parameter types should use "as type" where appropriate
- Default values should match BYOND documentation
- Any intentional deviations should be documented

## References

- DM Language Reference: `dm_open_ref/ref/`
- Current DMStandard: `DMCompilerCpp/DMStandard/`
- C# Implementation: OpenDream C# project (for comparison)
