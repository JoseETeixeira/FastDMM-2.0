package com.github.monster860.fastdmm.editing.ui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ModifiedType;
import com.github.monster860.fastdmm.objtree.ObjInstance;

/**
 * A docked inspector panel that lets you view and edit variables of an object on a map tile.
 */
public class TileInspectorPanel extends JPanel {
    private static final long serialVersionUID = 1L;

    private final FastDMM editor;

    private final JLabel titleLabel = new JLabel("Inspector");
    private final JLabel locationLabel = new JLabel("(x, y, z)");
    private final JComboBox<ObjInstance> objectPicker = new JComboBox<>();
    private final JTable table;
    private ModifiedTypeTableModel tableModel;

    private Location currentLoc;
    private List<ObjInstance> currentObjs = new ArrayList<>();
    private ObjInstance currentObj;

    public TileInspectorPanel(FastDMM editor) {
        super(new BorderLayout());
        this.editor = editor;

        // Header
        JPanel header = new JPanel(new BorderLayout());
        header.setBorder(BorderFactory.createEmptyBorder(6, 6, 6, 6));
        header.add(titleLabel, BorderLayout.WEST);
        header.add(locationLabel, BorderLayout.EAST);
        add(header, BorderLayout.NORTH);

        // Object picker
        JPanel pickRow = new JPanel(new BorderLayout());
        pickRow.setBorder(BorderFactory.createEmptyBorder(6, 6, 6, 6));
        pickRow.add(new JLabel("Object:"), BorderLayout.WEST);
        objectPicker.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                ObjInstance chosen = (ObjInstance) objectPicker.getSelectedItem();
                if (chosen != null && chosen != currentObj) {
                    setObject(chosen);
                }
            }
        });
        pickRow.add(objectPicker, BorderLayout.CENTER);
        add(pickRow, BorderLayout.BEFORE_FIRST_LINE);

    // Table (start with an empty, inert type)
    tableModel = new ModifiedTypeTableModel(new ModifiedType(new java.util.TreeMap<>(), "/obj"));
        table = new JTable(tableModel);
        table.setFillsViewportHeight(true);
        table.setDefaultRenderer(Object.class, new ModifiedTypeRenderer(tableModel));
        add(new JScrollPane(table), BorderLayout.CENTER);

        // Buttons
        JPanel actions = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JButton revertBtn = new JButton("Revert");
        revertBtn.addActionListener(e -> {
            if (currentObj != null) setObject(currentObj); // rebuild from source
        });
        JButton applyBtn = new JButton("Apply");
        applyBtn.addActionListener(e -> applyChanges());
        actions.add(revertBtn);
        actions.add(applyBtn);
        add(actions, BorderLayout.SOUTH);

        setPreferredSize(new Dimension(360, 10));
        setMinimumSize(new Dimension(280, 10));
        setEnabled(false);
    }

    /**
     * Populate inspector with objects on the given location and select the provided object.
     */
    public void inspect(Location loc, ObjInstance obj) {
        if (editor.dmm == null || loc == null) {
            clear();
            return;
        }
        synchronized (editor) {
            String key = editor.dmm.map.get(loc);
            if (key == null) {
                clear();
                return;
            }
            TileInstance ti = editor.dmm.instances.get(key);
            if (ti == null) {
                clear();
                return;
            }
            this.currentLoc = new Location(loc.x, loc.y, loc.z);
            this.currentObjs = new ArrayList<>(ti.getLayerSorted());
            this.locationLabel.setText("(" + loc.x + ", " + loc.y + ", " + loc.z + ")");

            objectPicker.removeAllItems();
            ObjInstance toSelect = null;
            for (ObjInstance oi : currentObjs) {
                if (oi == null) continue;
                objectPicker.addItem(oi);
                if (toSelect == null && obj != null) {
                    if (equalsBySignature(oi, obj)) {
                        toSelect = oi;
                    }
                }
            }
            if (toSelect == null && objectPicker.getItemCount() > 0) {
                toSelect = objectPicker.getItemAt(objectPicker.getItemCount() - 1); // topmost
            }
            objectPicker.setSelectedItem(toSelect);
            setObject(toSelect);
            setEnabled(true);
        }
    }

    private boolean equalsBySignature(ObjInstance a, ObjInstance b) {
        if (a == b) return true;
        if (a == null || b == null) return false;
        // Compare by DM string representation which is stable in this project
        return Objects.equals(a.toString(), b.toString());
    }

    private void setObject(ObjInstance obj) {
        this.currentObj = obj;
        if (obj == null) {
            // Empty model
            tableModel = new ModifiedTypeTableModel(new ModifiedType(new java.util.TreeMap<>(), "/obj"));
            table.setModel(tableModel);
            table.setDefaultRenderer(Object.class, new ModifiedTypeRenderer(tableModel));
            return;
        }
        ModifiedType mt = ModifiedType.deriveFrom(obj);
        tableModel = new ModifiedTypeTableModel(mt);
        table.setModel(tableModel);
        table.setDefaultRenderer(Object.class, new ModifiedTypeRenderer(tableModel));
    }

    private void applyChanges() {
        if (editor.dmm == null || currentLoc == null || currentObj == null) return;
        synchronized (editor) {
            String key = editor.dmm.map.get(currentLoc);
            if (key == null) return;
            TileInstance ti = editor.dmm.instances.get(key);
            if (ti == null) return;

            // Build the ModifiedType to apply from the table model (already holds edits)
            ModifiedType edited = tableModel.type;
            if (edited == null) return;

            // Ensure edited type is registered like other edit flows
            if (edited.parent != null) {
                if (editor.modifiedTypes.containsKey(edited.toString())) {
                    edited = editor.modifiedTypes.get(edited.toString());
                } else {
                    editor.modifiedTypes.put(edited.toString(), edited);
                    edited.parent.addInstance(edited);
                }
            }

            String newKey = ti.replaceObject(currentObj, edited.vars.size() != 0 ? edited : edited.parent);
            editor.dmm.putMap(currentLoc, newKey);
            // Record undo step
            editor.addToUndoStack(editor.dmm.popDiffs());

            // Refresh inspector content using the updated object from the tile
            String key2 = editor.dmm.map.get(currentLoc);
            TileInstance ti2 = editor.dmm.instances.get(key2);
            if (ti2 != null) {
                // Try to find the replaced instance by string signature
                ObjInstance match = null;
                String sig = (edited.vars.size() != 0 ? edited : edited.parent).toString();
                for (ObjInstance oi : ti2.getLayerSorted()) {
                    if (oi != null && sig.equals(oi.toString())) { match = oi; }
                }
                inspect(currentLoc, match);
            }
        }
    }

    public void clear() {
        currentLoc = null;
        currentObj = null;
        currentObjs.clear();
        objectPicker.removeAllItems();
        locationLabel.setText("(x, y, z)");
        setObject(null);
        setEnabled(false);
    }

    // No dummy instance needed; we seed with an empty ModifiedType when idle.
}
