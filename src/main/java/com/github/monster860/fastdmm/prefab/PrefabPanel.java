package com.github.monster860.fastdmm.prefab;

import java.awt.BorderLayout;
import java.awt.Dimension;
// Removed unused ActionEvent/ActionListener imports
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.util.ArrayList;

import javax.swing.DefaultListModel;
import javax.swing.JList;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JPopupMenu;
import javax.swing.JMenuItem;
import javax.swing.ListCellRenderer;
// Removed unused spinner & SwingUtilities imports

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmmmap.Location;
// Removed unused tile/object imports

/** UI panel for managing prefabs: create/select/edit. */
public class PrefabPanel extends JPanel {
    private final FastDMM editor;
    PrefabManager manager; // package-private so cell renderer in same package can access
    private final DefaultListModel<String> listModel = new DefaultListModel<>();
    final JList<String> prefabList = new JList<>(listModel);

    public String getSelectedPrefab() { return prefabList.getSelectedValue(); }
    public void clearSelection() { prefabList.clearSelection(); }

    public PrefabPanel(FastDMM editor) {
        this.editor = editor;
        setLayout(new BorderLayout());
        prefabList.setPreferredSize(new Dimension(180, 200));
        prefabList.setCellRenderer(new PrefabCellRenderer());
        // Right-click context menu for delete
        prefabList.addMouseListener(new MouseAdapter() {
            private void maybeShow(MouseEvent e) {
                if (e.isPopupTrigger()) {
                    int idx = prefabList.locationToIndex(e.getPoint());
                    if (idx >= 0) prefabList.setSelectedIndex(idx);
                    String sel = prefabList.getSelectedValue();
                    if (sel == null) return;
                    JPopupMenu menu = new JPopupMenu();
                    JMenuItem del = new JMenuItem("Delete Prefab");
                    del.addActionListener(ev -> deleteSelectedPrefab(sel));
                    menu.add(del);
                    menu.show(prefabList, e.getX(), e.getY());
                }
            }
            @Override public void mousePressed(MouseEvent e) { maybeShow(e); }
            @Override public void mouseReleased(MouseEvent e) { maybeShow(e); }
        });
        prefabList.addListSelectionListener(e -> {
            if (!e.getValueIsAdjusting()) {
                String sel = prefabList.getSelectedValue();
                if (sel != null) {
                    editor.prefabArmed = true; // persistent until deselected by choosing another brush
                    editor.setTool("prefab:" + sel);
                }
            }
        });
        add(new JScrollPane(prefabList), BorderLayout.CENTER);

    // Buttons removed: creation now via right-click selection context menu; usage auto when selected
    }

    public void attachManager(PrefabManager mgr) {
        this.manager = mgr;
        refreshList();
    }

    public void refreshList() {
        listModel.clear();
        if (manager == null) return;
        manager.prefabs.keySet().stream().sorted().forEach(listModel::addElement);
    }

    private void deleteSelectedPrefab(String name) {
        if (manager == null) return;
        if (!manager.prefabs.containsKey(name)) return;
        // If currently armed with this prefab, disarm and revert tool
        if (editor.prefabArmed && name.equals(getSelectedPrefab())) {
            editor.deselectPrefab();
        }
        manager.prefabs.remove(name);
        manager.save();
        refreshList();
    }

    // Removed unused legacy dialog & methods

    public void captureFromSelection(Prefab prefab) {
        if (!(editor.placementMode instanceof com.github.monster860.fastdmm.editing.placement.SelectPlacementMode)) {
            JOptionPane.showMessageDialog(this, "SelectRegion tool: select area first.");
            return;
        }
        com.github.monster860.fastdmm.editing.placement.SelectPlacementMode spm = (com.github.monster860.fastdmm.editing.placement.SelectPlacementMode) editor.placementMode;
        if (spm.selection.isEmpty()) {
            JOptionPane.showMessageDialog(this, "No tiles selected.");
            return;
        }
        int minx = Integer.MAX_VALUE, miny = Integer.MAX_VALUE, maxx = Integer.MIN_VALUE, maxy = Integer.MIN_VALUE;
        for (Location l : spm.selection) { if (l.x < minx) minx = l.x; if (l.y < miny) miny = l.y; if (l.x > maxx) maxx = l.x; if (l.y > maxy) maxy = l.y; }
        prefab.width = maxx - minx + 1; prefab.height = maxy - miny + 1; prefab.tiles.clear();
        for (Location l : spm.selection) {
            String key = editor.dmm.map.get(l);
            if (key == null) continue;
            com.github.monster860.fastdmm.dmmmap.TileInstance ti = editor.dmm.instances.get(key);
            if (ti == null) continue;
            com.github.monster860.fastdmm.dmmmap.TileInstance copy = new com.github.monster860.fastdmm.dmmmap.TileInstance(new ArrayList<>(ti.objs), editor.dmm);
            prefab.tiles.put(new Location(l.x - minx, l.y - miny, 1), copy);
        }
        prefab.buildPreview(editor);
    }
}

class PrefabCellRenderer extends javax.swing.JLabel implements ListCellRenderer<String> {
    public PrefabCellRenderer() { setOpaque(true); setVerticalAlignment(TOP); }
    @Override
    public java.awt.Component getListCellRendererComponent(JList<? extends String> list, String value, int index, boolean isSelected, boolean cellHasFocus) {
        setText(value);
        java.awt.Color bg = isSelected ? list.getSelectionBackground() : list.getBackground();
        java.awt.Color fg = isSelected ? list.getSelectionForeground() : list.getForeground();
        setBackground(bg); setForeground(fg);
        // attempt to fetch preview from manager via parent panel
        java.awt.Image img = null;
        java.awt.Container p = list.getParent();
        while (p != null && !(p instanceof PrefabPanel)) p = p.getParent();
        if (p instanceof PrefabPanel) {
            PrefabPanel panel = (PrefabPanel) p;
            if (panel.manager != null) {
                Prefab pref = panel.manager.prefabs.get(value);
                if (pref != null && pref.preview != null) img = pref.preview;
            }
        }
        if (img != null) setIcon(new javax.swing.ImageIcon(img)); else setIcon(null);
        return this;
    }
}
