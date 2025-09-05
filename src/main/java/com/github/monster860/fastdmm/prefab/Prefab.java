package com.github.monster860.fastdmm.prefab;

import java.util.LinkedHashMap;
import java.util.Map;

import com.github.monster860.fastdmm.dmmmap.DMM;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;

/**
 * Represents a prefab: a rectangular grid of tile compositions (turfs/objs/mobs/areas).
 * Stored as a map of relative Location (x,y,1) -> TileInstance definition.
 */
public class Prefab {
    public String name;
    public int width;
    public int height;
    // Relative tile contents
    public Map<Location, TileInstance> tiles = new LinkedHashMap<>();
    // Cached preview icon (composited image of entire prefab, 3x base size) - not persisted
    public java.awt.Image preview;

    public Prefab(String name, int width, int height) {
        this.name = name;
        this.width = width;
        this.height = height;
    }

    public void buildPreview(com.github.monster860.fastdmm.FastDMM editor) {
        if (editor == null || editor.interface_dmi == null) return;
        int base = 32; // assume BYOND 32x32
        int wpx = width * base;
        int hpx = height * base;
        java.awt.image.BufferedImage img = new java.awt.image.BufferedImage(wpx, hpx, java.awt.image.BufferedImage.TYPE_INT_ARGB);
        java.awt.Graphics2D g = img.createGraphics();
        try {
            // paint each tile's visible objs roughly (turf+obj+mob+area layering already sorted in TileInstance)
            for (Map.Entry<Location, TileInstance> e : tiles.entrySet()) {
                int dx = e.getKey().x * base;
                int dy = (height - 1 - e.getKey().y) * base; // flip Y to match editor view
                TileInstance ti = e.getValue();
                if (ti == null) continue;
                for (com.github.monster860.fastdmm.objtree.ObjInstance oi : ti.getLayerSorted()) {
                    if (oi == null) continue;
                    String icon = oi.getIcon();
                    if (icon == null || icon.isEmpty()) continue;
                    com.github.monster860.fastdmm.dmirender.DMI dmi = editor.getDmi(icon, false);
                    if (dmi == null) continue;
                    String state = oi.getIconState();
                    com.github.monster860.fastdmm.dmirender.IconState is = dmi.getIconState(state);
                    if (is == null) continue;
                    com.github.monster860.fastdmm.dmirender.IconSubstate sub = is.getSubstate(oi.getDir());
                    java.awt.image.BufferedImage subImg = sub.getSubImage();
                    if (sub == null || subImg == null) continue;
                    g.drawImage(subImg, dx, dy, null);
                }
            }
        } finally { g.dispose(); }
        // Previously 3x; now 5x smaller -> 0.6x original chosen region size (3/5 of former 3x scale)
        int targetW = Math.max(1, (int)Math.round(wpx * 0.6));
        int targetH = Math.max(1, (int)Math.round(hpx * 0.6));
        java.awt.Image scaled = img.getScaledInstance(targetW, targetH, java.awt.Image.SCALE_SMOOTH);
        preview = scaled;
    }

    public String toString() { return name + " (" + width + "x" + height + ")"; }
}
