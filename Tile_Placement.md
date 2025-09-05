**Goal:** Add **Attached Tiles** to FastDMM.

**User story:** As a mapper, when I place a tile that has configured attachments (e.g., a **top-left corner** tile), the editor should **automatically place the adjacent tiles** I configured (e.g., its **right** neighbor and **bottom** neighbor). I want a **small GUI** to define these attachments for any tile/icon\_state and a **JSON file** saved next to the project’s `.dme` so the rules persist across sessions.

---

## Constraints & references

* Codebase: **Java 8 / Gradle** (single runnable JAR). ([GitHub][1])
* Feature already present and relevant: **directional placement** (so hook into the same placement pipeline). ([GitHub][1])
* Keep changes **modular**: new service + small UI panel + minimal wiring in existing placement code.

---

## Data model & JSON

Create a tiny library class to load/save an attachments JSON:

```jsonc
{
  "version": 1,
  "rules": {
    // key uniquely identifies a tile by its icon path (relative to the .dme dir) and icon_state
    "icons/terrain/corners.dmi#nw_corner": {
      "north": null,           // cardinal placements relative to the placed (origin) tile
      "south": { "dmi": "icons/terrain/edges.dmi", "state": "vertical" },
      "east":  { "dmi": "icons/terrain/edges.dmi", "state": "horizontal" },
      "west":  null
    },
    "icons/terrain/tee.dmi#tee_east": {
      "north": { "dmi": "...", "state": "..." },
      "south": null, "east": null, "west": { "dmi": "...", "state": "..." }
    }
  }
}
```

* **Key format**: `relative/path/to.dmi#icon_state` (all lower-case, forward slashes).
* **Value**: object with `"north" | "south" | "east" | "west"` (or `"top" | "bottom" | "left" | "right"`; choose one naming set and stick to it) each pointing to `{dmi,state}` or `null`.
* **File location**: same directory as the opened `.dme` (e.g., `<project>/attached_tiles.json`).
* **If file missing**: start with `{ "version": 1, "rules": {} }`.

---

## New code (suggested file/class names)

1. **Service – loading, saving, and lookups**

* `src/main/.../attached/AttachedTileRules.java`

  * Fields: `int version`, `Map<String, AttachedNeighbors> rules`.
  * `AttachedNeighbors { TileRef north, south, east, west; }`
  * `TileRef { String dmi; String state; }`
* `src/main/.../attached/AttachedTileService.java`

  * Methods:

    * `Path resolveRulesFile(Path dmePath)` → `<dmeDir>/attached_tiles.json`.
    * `AttachedTileRules load(Path dmePath)` (create default if not found).
    * `void save(Path dmePath, AttachedTileRules rules)`.
    * `Optional<AttachedNeighbors> findFor(String dmiPath, String iconState)`; normalizes key.

2. **Placement hook (auto-place attachments)**

* Find the **“tile placement”** code path (where a map click ultimately writes the chosen turf/obj into the map).
  Typical names to search: `Place`, `MapEditor`, `MapCanvas`, `Brush`, `Tool`, `onMouseUp`, `onTilePlaced`, etc.
* Add a single integration point **after** the primary tile is placed successfully:

  ```java
  void placeWithAttachments(TileRef placed, int x, int y, int z) {
      // Save the primary tile (existing behavior) then:
      var key = keyFor(placed.dmi, placed.state); // same normalization as service
      var neighborsOpt = attachedTileService.findFor(placed.dmi, placed.state);
      if (!neighborsOpt.isPresent()) return;
      var n = neighborsOpt.get();
      if (n.north != null) placeSingle(n.north, x, y-1, z);
      if (n.south != null) placeSingle(n.south, x, y+1, z);
      if (n.east  != null) placeSingle(n.east,  x+1, y, z);
      if (n.west  != null) placeSingle(n.west,  x-1, y, z);
  }
  ```

  * **Don’t** place out-of-bounds; **don’t** overwrite locked tiles.
  * If a neighbor location already contains a tile of the **same** `{dmi,state}`, skip.
  * Respect undo/redo (wrap in the same command/transaction system the editor uses).

3. **Graphical editor panel**

* `src/main/.../attached/ui/AttachedTileEditorPanel.java` – small Swing panel (or whatever UI toolkit the app uses) that:

  * Shows the **selected tile** (center) and **four empty squares** for N/S/E/W.
  * Clicking an empty square opens the **icon picker** (reuse any existing asset chooser) to select `{dmi,state}`.
  * Writes into memory (`AttachedTileRules`) and **Save** to JSON when “Save” pressed.
  * Buttons: **Save**, **Revert**, **Clear all**, **Delete rule**.
* Add a toolbar/menu entry: **“Attached Tiles…”** that opens this panel for the **currently selected tile** in the object/turf browser.
* When a tile with existing attachments is selected, pre-populate the four squares.

4. **Persistence lifecycle**

* Load `attached_tiles.json` when a `.dme` is opened; keep in editor context.
* Save on panel **Save** and on project **Close** (if dirty).
* If the user switches `.dme`, unload current rules and load the next project’s rules.

---

## Edge cases & conflict policy

* If auto-placement would overwrite a non-empty tile:

  * Default policy: **skip** and display a subtle status message (“Attachment (east) skipped: occupied”).
  * Future enhancement: an option “Overwrite empty turfs only / overwrite any” in the panel.
* Multi-Z: attachments **stay on the same Z** as the origin.
* Map bounds: clamp and skip if outside bounds.
* Undo/redo: attachments must be in the **same command** as the primary placement so a single undo reverts all.

---

## Minimal UI layout (wireframe)

```
+--------------------------------------------------+
| Attached Tiles for: icons/terrain/corners.dmi#nw |
|                                                  |
|           [N]                                    |
|      [W] [CENTER TILE PREVIEW] [E]               |
|           [S]                                    |
|                                                  |
|  [Save] [Revert] [Clear All] [Delete Rule]       |
+--------------------------------------------------+
```

* `[N|S|E|W]` boxes show either “empty” or the chosen `{dmi#state}` thumbnail.
* Clicking a box opens the existing icon picker; ESC cancels.

---

## Tests / Acceptance checks

* **Placement:**

  * Given a rule with east+south defined, placing the origin tile at `(10,10)` writes east `(11,10)` and south `(10,11)`.
  * Out-of-bounds neighbors are skipped without error.
  * Occupied neighbors (different content) are not overwritten; an unobtrusive status message appears.
  * **Undo/redo** removes/restores the origin and all attachments together.

* **Persistence:**

  * New rule is saved to `<dmeDir>/attached_tiles.json`.
  * Reopening the same `.dme` loads the rules; panel shows existing attachments; placement works without reconfiguration.

* **UI:**

  * Selecting a tile with no rule shows four **empty** boxes.
  * After saving rules, boxes show thumbnails and tooltips of `{dmi#state}`.

---

## Implementation tips (for Copilot)

* Use a **plain Jackson or Gson** dependency if the project already has one; otherwise, keep a tiny hand-rolled JSON read/write to avoid new deps.
* Normalize keys: `Paths.get(dmiPath).normalize()`, lowercase, forward slashes.
* Put all new code under a **single package** e.g. `fastdmm.attached` and wire UI via the existing menu/toolbar system.
* Hook into the **same placement command** that the “directional objects” feature uses so transactions and undo/redo stay consistent. ([GitHub][1])

---

### Out of scope for this PR (leave TODOs)

* Diagonal attachments (NE, NW, SE, SW).
* Rotation-aware rules (mapping attachments based on direction selected).
* Bulk import/export of attachment templates.

---

**Deliverables:**

* `AttachedTileRules.java`, `AttachedTileService.java`, `AttachedTileEditorPanel.java` (+ any icons).
* Integration into placement pipeline.
* New menu item **Tools → Attached Tiles…**
* `attached_tiles.json` written beside the `.dme`.

---

## Done when

* I can open a `.dme`, define attachments for `corners.dmi#nw_corner`, save, place that corner, and watch the east/south tiles appear.
* Close editor, reopen the same project, rules are loaded and continue to work.
* One **Undo** removes origin + attachments; **Redo** restores them.

---

### Notes for reviewers

* Keep all behavior behind a small service and a UI panel so this stays optional and easy to iterate.

---

If you’d like, I’ll turn this into a PR description template next.

**Sources**
FastDMM is a Java/Gradle app packaged as a single runnable JAR; feature list includes directional placement (useful for finding the placement hook). ([GitHub][1])
Original BYOND forum post announcing FastDMM (links to the repo/releases and confirms the “single .jar” packaging). ([byond.com][2])

[1]: https://github.com/monster860/FastDMM "GitHub - monster860/FastDMM: A robust BYOND map editor"
[2]: https://www.byond.com/forum/post/2129534 "BYOND Forums - Tutorials & Snippets - Announcing FastDMM"
