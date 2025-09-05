package com.github.monster860.fastdmm.editing.placement;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.IconState;
import com.github.monster860.fastdmm.dmirender.RenderInstance;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.editing.FloatingSelection;
import com.github.monster860.fastdmm.objtree.ObjInstance;

public class SelectPlacementMode implements PlacementMode {
    public FastDMM editor;

    public Set<Location> selection = new HashSet<Location>();
    FloatingSelection floatSelect;
    @Override
    public PlacementHandler getPlacementHandler(FastDMM editor, ObjInstance instance, Location initialLocation) {
        this.editor = editor;
        
        if(selection.contains(initialLocation) && !editor.isAltPressed && !editor.isCtrlPressed && !editor.isShiftPressed) {
        	floatSelect = new FloatingSelection(editor.dmm, selection, editor);
			synchronized(editor) {
				HashMap<Location, String[]> changes = new HashMap<Location, String[]>();
				for(Location l : selection) {
					String key = editor.dmm.map.get(l);
					if(key == null)
						continue;
					TileInstance ti = editor.dmm.instances.get(key);
					if(ti == null)
						continue;
				
					String newKey = ti.deleteAllInFilter(editor);
					
					String[] keys = {key, newKey};
					changes.put(l, keys);
					
					editor.dmm.putMap(l, newKey);
				}
				editor.addToUndoStack(editor.dmm.popDiffs());
			}
			clearSelection();
        }
        
        if(floatSelect != null) {
        	if(floatSelect.inSelection(initialLocation)) {
        		return new MoveFloatingSelectionPlacementHandler();
        	} else {
        		floatSelect.anchor(editor.dmm);
        		floatSelect = null;
        		return null;
        	}
        }
        
        
        return new SelectPlacementHandler();
    }

    @Override
    public int visualize(Set<RenderInstance> rendInstanceSet, int currCreationIndex) {
        if(editor == null)
            return currCreationIndex;
		for(Location l : selection) {
			int dirs = 0;
			for(int i = 0; i < 4; i++) {
				int cdir = IconState.indexToDirArray[i];
				Location l2 = l.getStep(cdir);
				// Show edge only when neighbor tile is not part of selection or area type differs
				boolean neighborInSel = selection.contains(l2);
				if (!neighborInSel) {
					dirs |= cdir;
				} else if (editor != null && editor.dmm != null) {
					String k1 = editor.dmm.map.get(l);
					String k2 = editor.dmm.map.get(l2);
					if (k1 != null && k2 != null) {
						TileInstance t1 = editor.dmm.instances.get(k1);
						TileInstance t2 = editor.dmm.instances.get(k2);
						if (t1 != null && t2 != null) {
							com.github.monster860.fastdmm.objtree.ObjInstance a1 = t1.getArea();
							com.github.monster860.fastdmm.objtree.ObjInstance a2 = t2.getArea();
							String a1t = (a1 != null) ? a1.typeString() : null;
							String a2t = (a2 != null) ? a2.typeString() : null;
							if (a1t != null && a2t != null && !a1t.equals(a2t)) {
								dirs |= cdir;
							}
						}
					}
				}
			}
            if(dirs != 0) {
                RenderInstance ri = new RenderInstance(currCreationIndex++);
                ri.plane = 101;
                ri.x = l.x;
                ri.y = l.y;
                ri.substate = editor.interface_dmi.getIconState("" + dirs).getSubstate(2);
                ri.color = new Color(255,255,255);

                rendInstanceSet.add(ri);
            }
        }
        setCount();
        if(floatSelect != null) {
        	currCreationIndex = floatSelect.visualize(rendInstanceSet, currCreationIndex, editor);
        }
        return currCreationIndex;
    }

    public void setCount() {
        if(selection.size() > 1) {
            editor.statusstring = selection.size() + " tiles selected. ";
        } else if(selection.size() == 1) {
            editor.statusstring = selection.size() + " tile selected. ";
        } else if(selection.size() == 0) {
            editor.statusstring = "No tiles selected. ";
        }
        editor.selection.setText(editor.statusstring);
    }

    public void clearSelection() {
        selection.clear();
    }

	@Override
	public void addToTileMenu(FastDMM editor, Location mapLocation, TileInstance instance, JPopupMenu menu) {
		this.editor = editor;
		if(selection.contains(mapLocation)) {
			JMenuItem item = new JMenuItem("Delete in Selection");
			item.addActionListener(new SelectPlacementMode.SelectListener(editor, this, true, false));
			menu.add(item);
			// New: create prefab from current selection
			JMenuItem prefabItem = new JMenuItem("Create Prefab from Selection");
			prefabItem.addActionListener(ev -> {
				if (editor.prefabManager == null) return;
				String name = javax.swing.JOptionPane.showInputDialog(menu, "Prefab name:");
				if (name == null || name.trim().isEmpty()) return;
				com.github.monster860.fastdmm.prefab.Prefab p = editor.prefabManager.create(name.trim(), 1, 1);
				// capture selection into prefab (reuse panel logic)
				if (editor.prefabPanel != null) {
					editor.prefabPanel.captureFromSelection(p);
					editor.prefabManager.save();
					editor.prefabPanel.refreshList();
				}
			});
			menu.add(prefabItem);
			item = new JMenuItem("Copy Selection");
			item.addActionListener(new SelectPlacementMode.SelectListener(editor, this, false, true));
			menu.add(item);
			item = new JMenuItem("Cut Selection");
			item.addActionListener(new SelectPlacementMode.SelectListener(editor, this, true, true));
			menu.add(item);
		}
		if(floatSelect != null && mapLocation.x >= floatSelect.x && mapLocation.x >= floatSelect.x && 
			mapLocation.x < (floatSelect.x+floatSelect.width) && mapLocation.y < (floatSelect.y+floatSelect.height)) {
			
			JMenuItem item = new JMenuItem("Delete in Selection");
			item.addActionListener(new SelectPlacementMode.FloatingSelectionListener(editor, this, true, false));
			menu.add(item);
			item = new JMenuItem("Copy Selection");
			item.addActionListener(new SelectPlacementMode.FloatingSelectionListener(editor, this, false, true));
			menu.add(item);
			item = new JMenuItem("Cut Selection");
			item.addActionListener(new SelectPlacementMode.FloatingSelectionListener(editor, this, true, true));
			menu.add(item);
		}
		JMenuItem item = new JMenuItem("Paste");
		item.addActionListener(new SelectPlacementMode.PasteListener(editor, this, mapLocation));
		menu.add(item);
	}
	
	@Override
	public void flush(FastDMM editor) {
		if(floatSelect != null) {
			floatSelect.anchor(editor.dmm);
			floatSelect = null; // prevent lingering visualization after flush (e.g., on save or tool switch)
		}
	}


	public static class SelectListener implements ActionListener {
		FastDMM editor;
		SelectPlacementMode selection;
		boolean doDelete;
		boolean doCopy;
		
		public SelectListener(FastDMM editor, SelectPlacementMode selection, boolean doDelete, boolean doCopy) {
			this.editor = editor;
			this.selection = selection;
			this.doDelete = doDelete;
			this.doCopy = doCopy;
		}
		
		@Override
		public void actionPerformed(ActionEvent e) {
			synchronized(editor) {
				if(editor.dmm == null)
					return;
				if(doCopy) {
					new FloatingSelection(editor.dmm, selection.selection, editor).toClipboard();
				}
				if(doDelete) {
					HashMap<Location, String[]> changes = new HashMap<Location, String[]>();
					for(Location l : selection.selection) {
						String key = editor.dmm.map.get(l);
						if(key == null)
							continue;
						TileInstance ti = editor.dmm.instances.get(key);
						if(ti == null)
							continue;
					
						String newKey = ti.deleteAllInFilter(editor);
						
						String[] keys = {key, newKey};
						changes.put(l, keys);
						
						editor.dmm.putMap(l, newKey);
					}
					editor.addToUndoStack(editor.dmm.popDiffs());
					selection.clearSelection();
				}
			}
		}
	}
	
	public static class FloatingSelectionListener implements ActionListener {
		FastDMM editor;
		SelectPlacementMode selection;
		boolean doDelete;
		boolean doCopy;
		
		public FloatingSelectionListener(FastDMM editor, SelectPlacementMode selection, boolean doDelete, boolean doCopy) {
			this.editor = editor;
			this.selection = selection;
			this.doDelete = doDelete;
			this.doCopy = doCopy;
		}
		
		@Override
		public void actionPerformed(ActionEvent e) {
			synchronized(editor) {
				if(editor.dmm == null)
					return;
				if(doCopy) {
					selection.floatSelect.toClipboard();
				}
				if(doDelete) {
					selection.floatSelect = null;
				}
			}
		}
	}
	
	public static class PasteListener implements ActionListener {
		FastDMM editor;
		SelectPlacementMode selection;
		Location mapLocation;
		
		public PasteListener(FastDMM editor, SelectPlacementMode selection, Location l) {
			this.editor = editor;
			this.selection = selection;
			this.mapLocation = l;
		}
		
		@Override
		public void actionPerformed(ActionEvent e) {
			synchronized(editor) {
				if(editor.dmm == null)
					return;
				FloatingSelection newFloatSel = FloatingSelection.fromClipboard(editor.objTree,editor.dmm);
				if(newFloatSel == null)
					return;
				newFloatSel.x = mapLocation.x-(newFloatSel.width/2);
				newFloatSel.y = mapLocation.y-(newFloatSel.height/2);
				newFloatSel.z = mapLocation.z;
				selection.clearSelection();
				selection.floatSelect = newFloatSel;
			}
		}
	}
}
