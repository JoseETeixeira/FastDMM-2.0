package com.github.monster860.fastdmm.editing.placement;

import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;
import javax.swing.tree.TreePath;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ModifiedType;
import com.github.monster860.fastdmm.objtree.ObjInstance;
import com.github.monster860.fastdmm.objtree.ObjectTree;

/**
 * Picker mode: clicking a tile selects the topmost object under the cursor
 * and sets it as the current brush (selectedInstance/selectedObject).
 */
public class PickerPlacementMode implements PlacementMode {

    @Override
    public PlacementHandler getPlacementHandler(FastDMM editor, ObjInstance instance, Location initialLocation) {
        if (editor == null || editor.dmm == null) return null;
        String key = editor.dmm.map.get(initialLocation);
        if (key == null) return null;
        TileInstance tInstance = editor.dmm.instances.get(key);
        if (tInstance == null) return null;

        List<ObjInstance> layerSorted = tInstance.getLayerSorted();
        ObjInstance picked = null;
        // Prefer the topmost object that is visible per filter; else fall back to the topmost object
        for (int idx = layerSorted.size() - 1; idx >= 0; idx--) {
            ObjInstance oi = layerSorted.get(idx);
            if (oi == null) continue;
            boolean valid = editor.inFilter(oi);
            if (valid) { picked = oi; break; }
            if (picked == null) picked = oi;
        }

        if (picked != null) {
            final ObjInstance pickedFinal = picked;
            SwingUtilities.invokeLater(() -> {
                synchronized (editor) {
                    editor.selectedObject = pickedFinal instanceof ObjectTree.Item
                        ? (ObjectTree.Item) pickedFinal
                        : ((ModifiedType) pickedFinal).parent;
                    editor.selectedInstance = pickedFinal;

                    // Mirror the tree/instances selection like MakeActiveObjectListener
                    List<Object> path = new LinkedList<>();
                    ObjectTree.Item curr = editor.selectedObject;
                    while (curr != null && (curr.istype("/area") || curr.istype("/mob") || curr.istype("/obj") || curr.istype("/turf"))) {
                        path.add(0, curr);
                        curr = curr.parent;
                    }
                    path.add(0, editor.objTree);

                    editor.objTreeVis.setSelectionPath(new javax.swing.tree.TreePath(path.toArray()));
                    editor.instancesVis.setSelectedValue(editor.selectedInstance, true);
                }
            });
        }

        // No placement occurs in picker mode
        return null;
    }

    @Override
    public int visualize(Set<RenderInstance> rendInstanceSet, int currCreationIndex) {
        return currCreationIndex;
    }

    @Override
    public void addToTileMenu(FastDMM editor, Location mapLocation, TileInstance instance, JPopupMenu menu) {
        // No extra menu items for picker
    }

    @Override
    public void flush(FastDMM editor) {
        // Nothing to flush
    }
}
