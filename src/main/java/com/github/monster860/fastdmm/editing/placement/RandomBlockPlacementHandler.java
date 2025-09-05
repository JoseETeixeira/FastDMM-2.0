package com.github.monster860.fastdmm.editing.placement;

import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import javax.swing.JOptionPane;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.Util;
import com.github.monster860.fastdmm.dmirender.DMI;
import com.github.monster860.fastdmm.dmirender.IconSubstate;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;

public class RandomBlockPlacementHandler implements PlacementHandler {
    private Location startLocation;
    private Location endLocation;
    private FastDMM editor;
    private ObjInstance oInstance;
    private float chance = 0.5f; // default 50%
    private final Random rng = new Random();

    @Override
    public void init(FastDMM editor, ObjInstance instance, Location initialLocation) {
        this.editor = editor;
        this.oInstance = instance;
        this.startLocation = this.endLocation = initialLocation;
    // Read percent from toolbar spinner on init
    int pct = editor.randomChancePercent;
    if (pct < 0) pct = 0; if (pct > 100) pct = 100;
    this.chance = pct / 100f;
    }

    @Override
    public void dragTo(Location location) {
        endLocation = location;
    }

    @Override
    public int visualize(Set<RenderInstance> rendInstanceSet, int currCreationIndex) {
        // Show placement preview as bounding box and faint examples
        Location l1 = new Location(Math.min(startLocation.x, endLocation.x), Math.min(startLocation.y, endLocation.y), startLocation.z);
        Location l2 = new Location(Math.max(startLocation.x, endLocation.x), Math.max(startLocation.y, endLocation.y), startLocation.z);

        DMI dmi = editor.getDmi(oInstance.getIcon(), true);
        if (dmi != null) {
            String iconState = oInstance.getIconState();
            IconSubstate substate = dmi.getIconState(iconState).getSubstate(oInstance.getDir());
            for (int x = l1.x; x <= l2.x; x++) {
                for (int y = l1.y; y <= l2.y; y++) {
                    RenderInstance ri = new RenderInstance(currCreationIndex++);
                    ri.layer = oInstance.getLayer();
                    ri.plane = oInstance.getPlane();
                    ri.x = x + (oInstance.getPixelX() / (float) editor.objTree.icon_size);
                    ri.y = y + (oInstance.getPixelY() / (float) editor.objTree.icon_size);
                    ri.substate = substate;
                    // preview color slightly transparent
                    ri.color = new java.awt.Color(oInstance.getColor().getRed(), oInstance.getColor().getGreen(), oInstance.getColor().getBlue(), 128);
                    rendInstanceSet.add(ri);
                }
            }
        }

        currCreationIndex = Util.drawBox(editor, rendInstanceSet, currCreationIndex, l1, l2);
        return currCreationIndex;
    }

    @Override
    public void finalizePlacement() {
        Location l1 = new Location(Math.min(startLocation.x, endLocation.x), Math.min(startLocation.y, endLocation.y), startLocation.z);
        Location l2 = new Location(Math.max(startLocation.x, endLocation.x), Math.max(startLocation.y, endLocation.y), startLocation.z);
        HashSet<Location> changed = new HashSet<>();
        for (int x = l1.x; x <= l2.x; x++) {
            for (int y = l1.y; y <= l2.y; y++) {
                if (rng.nextFloat() > chance) continue;
                Location l = new Location(x, y, l1.z);
                String key = editor.dmm.map.get(l);
                if (key != null) {
                    TileInstance tInstance = editor.dmm.instances.get(key);
                    String newKey = tInstance.addObject(oInstance);
                    editor.dmm.putMap(l, newKey);
                    changed.add(l);
                    if (editor.attachedService != null) {
                        editor.attachedService.applyAttachments(l, oInstance);
                    }
                }
            }
        }
        if (!changed.isEmpty()) {
            editor.addToUndoStack(editor.dmm.popDiffs());
        }
    }
}
