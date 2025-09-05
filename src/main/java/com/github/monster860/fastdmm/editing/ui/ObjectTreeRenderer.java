package com.github.monster860.fastdmm.editing.ui;

import java.awt.Component;

import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.DMI;
import com.github.monster860.fastdmm.dmirender.IconSubstate;
import com.github.monster860.fastdmm.objtree.ObjectTree;
import com.github.monster860.fastdmm.objtree.ObjectTree.Item;

public class ObjectTreeRenderer extends DefaultTreeCellRenderer {
	private static final long serialVersionUID = 931493078348635512L;
	public FastDMM fdmm;
	
	public ObjectTreeRenderer(FastDMM fdmm) {
		this.fdmm = fdmm;
	}
	public Component getTreeCellRendererComponent(JTree tree, Object value, boolean sel, boolean expanded, boolean leaf, int row, boolean hasFocus) {
		super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
		if(value instanceof ObjectTree.Item) {
			ObjectTree.Item item = (ObjectTree.Item)value;
			setText(item.parentlessName());
			setToolTipText(item.path);
			
			DMI dmi = fdmm.getDmi(item.getIcon(), false);
			if(dmi != null) {
				String iconState = item.getIconState();
				IconSubstate substate = dmi.getIconState(iconState).getSubstate(item.getDir());
				setIcon(substate.getScaled());
			} else {
				setIcon(null);
			}
		}
		
		return this;
	}
}
