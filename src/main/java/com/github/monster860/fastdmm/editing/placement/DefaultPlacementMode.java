package com.github.monster860.fastdmm.editing.placement;

import java.util.Set;

import javax.swing.JPopupMenu;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;

public class DefaultPlacementMode implements PlacementMode {

	@Override
	public PlacementHandler getPlacementHandler(FastDMM editor, ObjInstance instance, Location initialLocation) {
		if(instance == null)
			return null;
		
		if(editor.isCtrlPressed)
			return new DirectionalPlacementHandler();
		else if(editor.isShiftPressed)
			return new BlockPlacementHandler();
		else
			return new DefaultPlacementHandler();
	}

	@Override
	public int visualize(Set<RenderInstance> rendInstanceSet, int currCreationIndex) {
		return currCreationIndex;
	}

	@Override
	public void addToTileMenu(FastDMM editor, Location mapLocation, TileInstance instance, JPopupMenu menu) {
	}

	@Override
	public void flush(FastDMM editor) {
	}

}
