package com.github.monster860.fastdmm.editing.placement;

import java.util.Set;

import javax.swing.JPopupMenu;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;

/**
 * Placement mode for placing objects randomly within a rectangular selection.
 */
public class RandomPlacementMode implements PlacementMode {

    @Override
    public PlacementHandler getPlacementHandler(FastDMM editor, ObjInstance instance, Location initialLocation) {
        if (instance == null) return null;
        return new RandomBlockPlacementHandler();
    }

    @Override
    public int visualize(Set<RenderInstance> rendInstanceSet, int currCreationIndex) {
        return currCreationIndex;
    }

    @Override
    public void addToTileMenu(FastDMM editor, Location mapLocation, TileInstance instance, JPopupMenu menu) {
        // No tile menu additions specific to random placement
    }

    @Override
    public void flush(FastDMM editor) {
        // nothing to flush
    }
}
