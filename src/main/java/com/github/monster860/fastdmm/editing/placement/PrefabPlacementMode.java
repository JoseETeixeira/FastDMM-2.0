package com.github.monster860.fastdmm.editing.placement;

import java.awt.Color;
import java.util.Set;

import javax.swing.JPopupMenu;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;
import com.github.monster860.fastdmm.prefab.Prefab;

/** Placement mode that previews and places an entire prefab footprint. */
public class PrefabPlacementMode implements PlacementMode {
    private final FastDMM editor;
    private final Prefab prefab;
    // Top-left preview origin (world tile under cursor when placement starts)
    private Location hover;

    public PrefabPlacementMode(FastDMM editor, Prefab prefab) {
        this.editor = editor;
        this.prefab = prefab;
    }

    @Override
    public PlacementHandler getPlacementHandler(FastDMM editor, ObjInstance instance, Location initialLocation) {
        hover = initialLocation;
        return new PrefabPlacementHandler(prefab) {
            @Override public void dragTo(Location location) { hover = location; }
        };
    }

    @Override
    public int visualize(Set<RenderInstance> rendInstanceSet, int idx) {
        if (hover == null || prefab == null) return idx;
        // Render actual tile contents at their real size with slight tint for preview.
        for (java.util.Map.Entry<Location, TileInstance> e : prefab.tiles.entrySet()) {
            Location rel = e.getKey();
            TileInstance ti = e.getValue();
            if (ti == null) continue;
            for (ObjInstance obj : ti.getLayerSorted()) {
                if (obj == null) continue;
                String icon = obj.getIcon();
                if (icon == null || icon.isEmpty()) continue;
                com.github.monster860.fastdmm.dmirender.DMI dmi = editor.getDmi(icon, false);
                if (dmi == null) continue;
                String state = obj.getIconState();
                com.github.monster860.fastdmm.dmirender.IconState is = dmi.getIconState(state);
                if (is == null) continue;
                com.github.monster860.fastdmm.dmirender.IconSubstate sub = is.getSubstate(obj.getDir());
                if (sub == null) continue;
                RenderInstance ri = new RenderInstance(idx++);
                ri.plane = 100; // above map
                ri.x = hover.x + rel.x;
                ri.y = hover.y + rel.y;
                ri.substate = sub;
                ri.color = new Color(255, 255, 255, 180); // slightly translucent
                rendInstanceSet.add(ri);
            }
        }
        return idx;
    }

    @Override
    public void addToTileMenu(FastDMM editor, Location mapLocation, TileInstance instance, JPopupMenu menu) {}

    @Override
    public void flush(FastDMM editor) { hover = null; }
}
