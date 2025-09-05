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
            Location world = new Location(anchor.x + rel.x, anchor.y + rel.y, editor.dmm.storedZ);
            String oldKey = editor.dmm.map.get(world);
            TileInstance copy = new TileInstance(new java.util.ArrayList<>(ti.objs), editor.dmm);
            copy.prefabName = prefab.name;
            copy.prefabRelX = rel.x;
            copy.prefabRelY = rel.y;
            String newKey = editor.dmm.getKeyForInstance(copy);
            editor.dmm.putMap(world, newKey);
            changes.put(world, new String[]{oldKey, newKey});
        }
        editor.addToUndoStack(editor.dmm.popDiffs());
    }

    @Override public void dragTo(Location location) { /* single drop */ }
    @Override public void finalizePlacement() {}
    @Override public int visualize(java.util.Set<RenderInstance> rendInstanceSet, int currCreationIndex) { return currCreationIndex; }
}
