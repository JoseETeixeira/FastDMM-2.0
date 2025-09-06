package com.github.monster860.fastdmm.editing.placement;

import java.util.HashMap;
import java.util.Map;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;
import com.github.monster860.fastdmm.prefab.Prefab;

/** Places all tiles from a prefab relative to first clicked tile. */
public class PrefabPlacementHandler implements PlacementHandler {
    private final Prefab prefab;
    private FastDMM editor;
    private Location anchor; // world tile where placement began

    public PrefabPlacementHandler(Prefab prefab) { this.prefab = prefab; }

    @Override
    public void init(FastDMM editor, ObjInstance instance, Location initialLocation) {
        this.editor = editor;
        anchor = initialLocation;
        place();
    }

    private void place() {
        if (editor == null || editor.dmm == null) return;
        HashMap<Location, String[]> changes = new HashMap<>();
        for (Map.Entry<Location, TileInstance> e : prefab.tiles.entrySet()) {
            Location rel = e.getKey();
            TileInstance ti = e.getValue();
            if (ti == null || ti.objs == null) continue; // safety
            Location world = new Location(anchor.x + rel.x, anchor.y + rel.y, editor.dmm.storedZ);
            String oldKey = editor.dmm.map.get(world);
            TileInstance existing = (oldKey != null) ? editor.dmm.instances.get(oldKey) : null;
            java.util.List<com.github.monster860.fastdmm.objtree.ObjInstance> merged;
            if (existing != null) {
                if (existing.objs == null) existing.objs = new java.util.ArrayList<>();
                merged = new java.util.ArrayList<>(existing.objs); // start with what's there
                // Detect if prefab supplies a turf/area; if so, remove existing of that category before merge
                boolean prefabHasTurf = false, prefabHasArea = false;
                for (com.github.monster860.fastdmm.objtree.ObjInstance oi : ti.objs) {
                    String t = oi.typeString();
                    if (t != null) {
                        if (t.startsWith("/turf")) prefabHasTurf = true; else if (t.startsWith("/area")) prefabHasArea = true;
                    }
                }
                if (prefabHasTurf) {
                    merged.removeIf(o -> { String t = o.typeString(); return t != null && t.startsWith("/turf"); });
                }
                if (prefabHasArea) {
                    merged.removeIf(o -> { String t = o.typeString(); return t != null && t.startsWith("/area"); });
                }
                // Add prefab objects (including turf/area if present)
                for (com.github.monster860.fastdmm.objtree.ObjInstance oi : ti.objs) {
                    if (oi != null) merged.add(oi);
                }
            } else {
                // No existing tile content; just copy prefab
                merged = new java.util.ArrayList<>();
                for (com.github.monster860.fastdmm.objtree.ObjInstance oi : ti.objs) if (oi != null) merged.add(oi);
            }
            // If merge produced identical composition to existing, skip
            TileInstance newTi = new TileInstance(merged, editor.dmm);
            newTi.prefabName = prefab.name;
            newTi.prefabRelX = rel.x;
            newTi.prefabRelY = rel.y;
            String newKey = editor.dmm.getKeyForInstance(newTi);
            if (oldKey != null && oldKey.equals(newKey)) continue; // no actual change
            editor.dmm.putMap(world, newKey);
            changes.put(world, new String[]{oldKey, newKey});
        }
        editor.addToUndoStack(editor.dmm.popDiffs());
    }

    @Override public void dragTo(Location location) { /* single drop */ }
    @Override public void finalizePlacement() {}
    @Override public int visualize(java.util.Set<RenderInstance> rendInstanceSet, int currCreationIndex) { return currCreationIndex; }
}
