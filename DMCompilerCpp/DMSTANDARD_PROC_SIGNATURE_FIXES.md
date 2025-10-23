# DMStandard Proc Signature Fixes

This document summarizes the corrections made to proc signatures in `_Standard.dm` to match the official BYOND documentation from `dm_open_ref`.

## Changes Made

### Parameter Type Corrections

1. **block()** - Fixed parameter order and defaults

   - Before: `proc/block(atom/Start, atom/End, StartZ, EndX=Start, EndY=End, EndZ=StartZ)`
   - After: `proc/block(Start, End, StartX, StartY, StartZ, EndX=StartX, EndY=StartY, EndZ=StartZ)`
   - Removed unnecessary `atom/` type constraints and fixed parameter order

2. **bounds_dist()** - Removed type constraints

   - Before: `proc/bounds_dist(atom/Ref, atom/Target)`
   - After: `proc/bounds_dist(Ref, Target)`

3. **get_step_to()** - Removed incorrect return type
   - Before: `proc/get_step_to(Ref, Trg, Min=0) as num`
   - After: `proc/get_step_to(Ref, Trg, Min=0)`
   - Returns location or null, not a number

### Variadic Argument Corrections

4. **cmptext()** - Changed to variadic

   - Before: `proc/cmptext(T1)`
   - After: `proc/cmptext(...)`
   - Accepts any number of text strings to compare

5. **cmptextEx()** - Changed to variadic

   - Before: `proc/cmptextEx(T1)`
   - After: `proc/cmptextEx(...)`
   - Accepts any number of text strings to compare

6. **sorttext()** - Changed to variadic

   - Before: `proc/sorttext(T1, T2)`
   - After: `proc/sorttext(...)`
   - Accepts any number of text strings to sort

7. **sorttextEx()** - Changed to variadic
   - Before: `proc/sorttextEx(T1, T2)`
   - After: `proc/sorttextEx(...)`
   - Accepts any number of text strings to sort

### Return Type Corrections

8. **icon_states()** - Fixed return type

   - Before: `proc/icon_states(Icon, mode = 0) as text|null`
   - After: `proc/icon_states(Icon, mode = 0) as /list`
   - Returns a list of text strings, not a single text value

9. **text2ascii()** - Fixed return type

   - Before: `proc/text2ascii(T, pos = 1) as text`
   - After: `proc/text2ascii(T, pos = 1) as num`
   - Returns ASCII/Unicode code as number

10. **text2ascii_char()** - Fixed return type

    - Before: `proc/text2ascii_char(T, pos = 1) as text`
    - After: `proc/text2ascii_char(T, pos = 1) as num`
    - Returns ASCII/Unicode code as number

11. **trimtext()** - Simplified return type
    - Before: `proc/trimtext(Text) as text|null`
    - After: `proc/trimtext(Text) as text`
    - Always returns text (empty string if null input)

### Parameter Type Constraint Removals

12. **lowertext()** - Removed unnecessary type constraint

    - Before: `proc/lowertext(T as text)`
    - After: `proc/lowertext(T)`

13. **uppertext()** - Removed unnecessary type constraint

    - Before: `proc/uppertext(T as text)`
    - After: `proc/uppertext(T)`

14. **jointext()** - Simplified parameter types
    - Before: `proc/jointext(list/List as /list|text, Glue as text|null, Start = 1 as num, End = 0 as num)`
    - After: `proc/jointext(List, Glue, Start = 1, End = 0)`

### Missing Procs Added

15. **bound_pixloc()** - Added missing proc

    - `proc/bound_pixloc(Atom, Dir)`
    - Returns pixloc representing corner/side/center of atom bounds

16. **bounds()** - Added missing proc

    - `proc/bounds(Ref=src, Dist=0, x_offset, y_offset, extra_width=0, extra_height=0, x, y, width, height, z) as /list`
    - Returns list of atoms within bounding box

17. **newlist()** - Added missing proc

    - `proc/newlist(...) as /list`
    - Creates list of new objects

18. **obounds()** - Added missing proc

    - `proc/obounds(Ref, Dist=0, x_offset, y_offset, extra_width=0, extra_height=0) as /list`
    - Like bounds() but excludes Ref from results

19. **pick()** - Added missing proc

    - `proc/pick(...)`
    - Randomly selects from arguments or list

20. **prob()** - Added missing proc
    - `proc/prob(P) as num`
    - Returns 1 with probability P percent

### Implementation Function Signature Fixes

21. **replacetextEx_char()** - Removed type constraint

    - Before: `proc/replacetextEx_char(Haystack as text, ...)`
    - After: `proc/replacetextEx_char(Haystack, ...)`

22. **get_dir()** - Removed type constraints

    - Before: `proc/get_dir(atom/Loc1, atom/Loc2)`
    - After: `proc/get_dir(Loc1, Loc2)`

23. **get_dist()** - Removed type constraints

    - Before: `proc/get_dist(atom/Loc1, atom/Loc2)`
    - After: `proc/get_dist(Loc1, Loc2)`

24. **get_step_away()** - Removed type constraints

    - Before: `proc/get_step_away(atom/movable/Ref, /atom/Trg, Max = 5)`
    - After: `proc/get_step_away(Ref, Trg, Max = 5)`

25. **get_step_rand()** - Removed type constraints

    - Before: `proc/get_step_rand(atom/movable/Ref)`
    - After: `proc/get_step_rand(Ref)`

26. **get_step_towards()** - Removed type constraints

    - Before: `proc/get_step_towards(atom/movable/Ref, /atom/Trg)`
    - After: `proc/get_step_towards(Ref, Trg)`

27. **step()** - Removed type constraints and /proc/ prefix

    - Before: `/proc/step(atom/movable/Ref as /atom/movable, var/Dir, var/Speed=0)`
    - After: `proc/step(Ref, Dir, Speed=0)`

28. **step_away()** - Removed type constraints and /proc/ prefix

    - Before: `/proc/step_away(atom/movable/Ref as /atom/movable, /atom/Trg, Max=5, Speed=0)`
    - After: `proc/step_away(Ref, Trg, Max=5, Speed=0)`

29. **step_rand()** - Removed type constraints and /proc/ prefix

    - Before: `/proc/step_rand(atom/movable/Ref, Speed=0)`
    - After: `proc/step_rand(Ref, Speed=0)`

30. **step_to()** - Removed type constraints and /proc/ prefix

    - Before: `/proc/step_to(atom/movable/Ref, atom/Trg, Min = 0, Speed = 0)`
    - After: `proc/step_to(Ref, Trg, Min = 0, Speed = 0)`

31. **step_towards()** - Removed type constraints and /proc/ prefix
    - Before: `/proc/step_towards(atom/movable/Ref as /atom/movable, /atom/Trg, Speed=0)`
    - After: `proc/step_towards(Ref, Trg, Speed=0)`

## Rationale

These changes align the DMCompilerCpp implementation with the official BYOND documentation to ensure:

1. **Compatibility**: Code written for BYOND will compile correctly
2. **Correctness**: Return types and parameter counts match expected behavior
3. **Flexibility**: Removing unnecessary type constraints allows more flexible usage
4. **Completeness**: Adding missing procs ensures full standard library coverage

## References

All changes were verified against the official BYOND documentation in `dm_open_ref/ref/proc/*.md`.

## Intentional Deviations

None. All signatures now match the official BYOND documentation.
