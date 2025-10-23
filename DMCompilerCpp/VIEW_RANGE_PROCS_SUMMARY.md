# View/Range Procs Implementation Summary

## Task 22: Add Missing View/Range Procs

This document summarizes the implementation of view/range procs in DMStandard.

## Implemented Procs

### View Family (in \_Standard.dm)

1. **view(Dist = 5, Center = usr) as /list**

   - Returns a list of visible objects within Dist tiles of Center
   - Default distance: 5
   - Default center: usr

2. **viewers(Depth = world.view, Center = usr) as /list**
   - Returns a list of mobs that can see the center object
   - Default depth: world.view
   - Default center: usr

### Orange/Oview Family (in \_Standard.dm)

3. **orange(Dist = 5, Center = usr) as /list**

   - Like view() but ignores visibility (includes all objects)
   - Excludes Center and its contents
   - Default distance: 5
   - Default center: usr

4. **oview(Dist = 5, Center = usr) as /list**

   - Like view() but excludes Center and its contents
   - Default distance: 5
   - Default center: usr

5. **oviewers(Depth = world.view, Center = usr) as /list**

   - Like viewers() but excludes Center and its contents
   - Default depth: world.view
   - Default center: usr

6. **ohearers(Depth = world.view, Center = usr) as /list**
   - Like hearers() but excludes Center and its contents
   - Default depth: world.view
   - Default center: usr

### Range (in \_Standard.dm)

7. **range(Dist = 5, Center = usr) as /list**
   - Like view() but ignores visibility (includes all objects)
   - Includes Center and its contents
   - Default distance: 5
   - Default center: usr

### Bounds Family (in UnsortedAdditions.dm)

8. **bounds(Ref=src, Dist=0) as /list**

   - Returns atoms within the bounding box of Ref
   - Includes Ref in results
   - Multiple signatures supported (see comments in code)
   - Status: Marked as opendream_unimplemented (stub)

9. **obounds(Ref, Dist=0) as /list**

   - Like bounds() but excludes Ref from results
   - Status: Marked as opendream_unimplemented (stub)

10. **bound_pixloc(atom/Atom, Dir) as /pixloc**
    - Returns a pixloc representing a corner, side, or center of atom bounds
    - Dir: 0 for center, or direction constant for side/corner
    - Status: Marked as opendream_unimplemented (stub)

## Changes Made

### \_Standard.dm

- Updated `viewers()` signature: Added default value `world.view` for Depth parameter
- Updated `oviewers()` signature: Changed default from 5 to `world.view`
- Updated `orange()` return type: Removed `|null`, now just `/list`
- Updated `range()` signature: Added default values (Dist = 5, Center = usr)

### UnsortedAdditions.dm

- Added proper signatures for `bounds()`, `obounds()`, and `bound_pixloc()`
- Added documentation comments explaining multiple signatures for bounds()
- All three procs marked as `opendream_unimplemented` (stubs for future implementation)

## Verification

All procs compile successfully and are accessible in DM code. The test file `test_view_procs.dme` demonstrates usage of all implemented procs.

## Requirements Met

✅ 5.6 - DMStandard procs have correct signatures
✅ 6.1 - All view/range procs are defined and accessible

## Notes

- The bounds family procs (bounds, obounds, bound_pixloc) are currently stubs marked as unimplemented
- These will require runtime support for full implementation
- All signatures match BYOND documentation from dm_open_ref
