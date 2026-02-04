package com.github.monster860.fastdmm.editing.ui;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.dmirender.DMI;
import com.github.monster860.fastdmm.dmirender.IconSubstate;
import com.github.monster860.fastdmm.dmmmap.DMM;
import com.github.monster860.fastdmm.dmmmap.Location;
import com.github.monster860.fastdmm.dmmmap.TileInstance;
import com.github.monster860.fastdmm.objtree.ObjInstance;
import com.github.monster860.fastdmm.objtree.ObjectTree;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
import java.awt.*;
import java.util.*;
import java.util.List;

/**
 * Find and Replace dialog for finding all instances of a tile type
 * and optionally deleting or replacing them across the entire map.
 */
public class FindReplaceDialog extends JDialog {
    private final FastDMM editor;
    
    // Find panel components
    private JTextField findSearchField;
    private JTree findTree;
    private JLabel findPreviewLabel;
    private JLabel findTypeLabel;
    private String selectedFindType = null;
    
    // Replace panel components
    private JTextField replaceSearchField;
    private JTree replaceTree;
    private JLabel replacePreviewLabel;
    private JLabel replaceTypeLabel;
    private String selectedReplaceType = null;
    
    // Options
    private JCheckBox currentZOnlyCheckbox;
    private JCheckBox matchExactCheckbox;
    
    // Results
    private JLabel resultsLabel;
    private JList<LocationResult> resultsList;
    private DefaultListModel<LocationResult> resultsModel;
    
    // Buttons
    private JButton findButton;
    private JButton deleteAllButton;
    private JButton replaceAllButton;
    private JButton goToButton;

    public FindReplaceDialog(FastDMM editor) {
        super(editor, "Find and Replace", false); // Non-modal so user can see map
        this.editor = editor;
        setMinimumSize(new Dimension(800, 700));
        buildUI();
        pack();
        setLocationRelativeTo(editor);
    }

    private void buildUI() {
        setLayout(new BorderLayout(8, 8));
        
        // Main content panel
        JPanel mainPanel = new JPanel(new BorderLayout(8, 8));
        mainPanel.setBorder(BorderFactory.createEmptyBorder(8, 8, 8, 8));
        
        // Top section: Find and Replace selectors side by side
        JPanel selectorsPanel = new JPanel(new GridLayout(1, 2, 16, 0));
        selectorsPanel.add(createFindPanel());
        selectorsPanel.add(createReplacePanel());
        mainPanel.add(selectorsPanel, BorderLayout.NORTH);
        
        // Middle section: Options and action buttons
        JPanel middlePanel = new JPanel(new BorderLayout(8, 8));
        middlePanel.add(createOptionsPanel(), BorderLayout.NORTH);
        middlePanel.add(createActionsPanel(), BorderLayout.CENTER);
        mainPanel.add(middlePanel, BorderLayout.CENTER);
        
        // Bottom section: Results list
        mainPanel.add(createResultsPanel(), BorderLayout.SOUTH);
        
        add(mainPanel, BorderLayout.CENTER);
        
        // Close button at bottom
        JPanel bottomPanel = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JButton closeButton = new JButton("Close");
        closeButton.addActionListener(e -> dispose());
        bottomPanel.add(closeButton);
        add(bottomPanel, BorderLayout.SOUTH);
    }

    private JPanel createFindPanel() {
        JPanel panel = new JPanel(new BorderLayout(4, 4));
        panel.setBorder(BorderFactory.createTitledBorder("Find Type"));
        
        // Search field
        JPanel searchPanel = new JPanel(new BorderLayout(4, 4));
        searchPanel.add(new JLabel("Search:"), BorderLayout.WEST);
        findSearchField = new JTextField();
        searchPanel.add(findSearchField, BorderLayout.CENTER);
        panel.add(searchPanel, BorderLayout.NORTH);
        
        // Tree view
        findTree = new JTree(editor.objTree);
        findTree.setCellRenderer(new ObjectTreeRenderer(editor));
        findTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        collapseTree(findTree);
        
        JScrollPane treeScroll = new JScrollPane(findTree);
        treeScroll.setPreferredSize(new Dimension(350, 200));
        panel.add(treeScroll, BorderLayout.CENTER);
        
        // Preview panel
        JPanel previewPanel = new JPanel(new BorderLayout(4, 4));
        findPreviewLabel = new JLabel();
        findPreviewLabel.setHorizontalAlignment(SwingConstants.CENTER);
        findPreviewLabel.setPreferredSize(new Dimension(64, 64));
        findTypeLabel = new JLabel("No type selected");
        findTypeLabel.setHorizontalAlignment(SwingConstants.CENTER);
        previewPanel.add(findPreviewLabel, BorderLayout.CENTER);
        previewPanel.add(findTypeLabel, BorderLayout.SOUTH);
        panel.add(previewPanel, BorderLayout.EAST);
        
        // Tree selection listener
        findTree.addTreeSelectionListener(e -> {
            TreePath path = findTree.getSelectionPath();
            if (path != null && path.getLastPathComponent() instanceof ObjectTree.Item) {
                ObjectTree.Item item = (ObjectTree.Item) path.getLastPathComponent();
                selectedFindType = item.path;
                findTypeLabel.setText(item.path);
                updatePreview(findPreviewLabel, item);
                updateButtonStates();
            }
        });
        
        // Search filter
        findSearchField.getDocument().addDocumentListener(new DocumentListener() {
            @Override public void insertUpdate(DocumentEvent e) { applyFindFilter(); }
            @Override public void removeUpdate(DocumentEvent e) { applyFindFilter(); }
            @Override public void changedUpdate(DocumentEvent e) { applyFindFilter(); }
        });
        
        return panel;
    }

    private JPanel createReplacePanel() {
        JPanel panel = new JPanel(new BorderLayout(4, 4));
        panel.setBorder(BorderFactory.createTitledBorder("Replace With (optional)"));
        
        // Search field
        JPanel searchPanel = new JPanel(new BorderLayout(4, 4));
        searchPanel.add(new JLabel("Search:"), BorderLayout.WEST);
        replaceSearchField = new JTextField();
        searchPanel.add(replaceSearchField, BorderLayout.CENTER);
        
        // Clear selection button
        JButton clearBtn = new JButton("Clear");
        clearBtn.addActionListener(e -> {
            replaceTree.clearSelection();
            selectedReplaceType = null;
            replaceTypeLabel.setText("No type selected (delete mode)");
            replacePreviewLabel.setIcon(null);
            updateButtonStates();
        });
        searchPanel.add(clearBtn, BorderLayout.EAST);
        panel.add(searchPanel, BorderLayout.NORTH);
        
        // Tree view
        replaceTree = new JTree(editor.objTree);
        replaceTree.setCellRenderer(new ObjectTreeRenderer(editor));
        replaceTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        collapseTree(replaceTree);
        
        JScrollPane treeScroll = new JScrollPane(replaceTree);
        treeScroll.setPreferredSize(new Dimension(350, 200));
        panel.add(treeScroll, BorderLayout.CENTER);
        
        // Preview panel
        JPanel previewPanel = new JPanel(new BorderLayout(4, 4));
        replacePreviewLabel = new JLabel();
        replacePreviewLabel.setHorizontalAlignment(SwingConstants.CENTER);
        replacePreviewLabel.setPreferredSize(new Dimension(64, 64));
        replaceTypeLabel = new JLabel("No type selected (delete mode)");
        replaceTypeLabel.setHorizontalAlignment(SwingConstants.CENTER);
        previewPanel.add(replacePreviewLabel, BorderLayout.CENTER);
        previewPanel.add(replaceTypeLabel, BorderLayout.SOUTH);
        panel.add(previewPanel, BorderLayout.EAST);
        
        // Tree selection listener
        replaceTree.addTreeSelectionListener(e -> {
            TreePath path = replaceTree.getSelectionPath();
            if (path != null && path.getLastPathComponent() instanceof ObjectTree.Item) {
                ObjectTree.Item item = (ObjectTree.Item) path.getLastPathComponent();
                selectedReplaceType = item.path;
                replaceTypeLabel.setText(item.path);
                updatePreview(replacePreviewLabel, item);
                updateButtonStates();
            }
        });
        
        // Search filter
        replaceSearchField.getDocument().addDocumentListener(new DocumentListener() {
            @Override public void insertUpdate(DocumentEvent e) { applyReplaceFilter(); }
            @Override public void removeUpdate(DocumentEvent e) { applyReplaceFilter(); }
            @Override public void changedUpdate(DocumentEvent e) { applyReplaceFilter(); }
        });
        
        return panel;
    }

    private JPanel createOptionsPanel() {
        JPanel panel = new JPanel(new FlowLayout(FlowLayout.LEFT, 16, 4));
        panel.setBorder(BorderFactory.createTitledBorder("Options"));
        
        currentZOnlyCheckbox = new JCheckBox("Current Z-level only", false);
        currentZOnlyCheckbox.setToolTipText("If checked, only search/replace on the current Z-level");
        panel.add(currentZOnlyCheckbox);
        
        matchExactCheckbox = new JCheckBox("Exact type match", false);
        matchExactCheckbox.setToolTipText("If unchecked, will also match subtypes (e.g., /obj/item matches /obj/item/pen)");
        panel.add(matchExactCheckbox);
        
        return panel;
    }

    private JPanel createActionsPanel() {
        JPanel panel = new JPanel(new FlowLayout(FlowLayout.CENTER, 16, 8));
        
        findButton = new JButton("Find All");
        findButton.setToolTipText("Find all instances of the selected type");
        findButton.addActionListener(e -> performFind());
        findButton.setEnabled(false);
        panel.add(findButton);
        
        deleteAllButton = new JButton("Delete All Found");
        deleteAllButton.setToolTipText("Delete all instances found in the results");
        deleteAllButton.addActionListener(e -> performDeleteAll());
        deleteAllButton.setEnabled(false);
        panel.add(deleteAllButton);
        
        replaceAllButton = new JButton("Replace All");
        replaceAllButton.setToolTipText("Replace all found instances with the selected replacement type");
        replaceAllButton.addActionListener(e -> performReplaceAll());
        replaceAllButton.setEnabled(false);
        panel.add(replaceAllButton);
        
        goToButton = new JButton("Go To Selected");
        goToButton.setToolTipText("Navigate to the selected result in the map");
        goToButton.addActionListener(e -> goToSelected());
        goToButton.setEnabled(false);
        panel.add(goToButton);
        
        return panel;
    }

    private JPanel createResultsPanel() {
        JPanel panel = new JPanel(new BorderLayout(4, 4));
        panel.setBorder(BorderFactory.createTitledBorder("Results"));
        
        resultsLabel = new JLabel("No search performed yet");
        panel.add(resultsLabel, BorderLayout.NORTH);
        
        resultsModel = new DefaultListModel<>();
        resultsList = new JList<>(resultsModel);
        resultsList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        resultsList.setCellRenderer(new LocationResultRenderer());
        resultsList.addListSelectionListener(e -> {
            goToButton.setEnabled(resultsList.getSelectedValue() != null);
        });
        // Double-click to go to location
        resultsList.addMouseListener(new java.awt.event.MouseAdapter() {
            @Override
            public void mouseClicked(java.awt.event.MouseEvent e) {
                if (e.getClickCount() == 2) {
                    goToSelected();
                }
            }
        });
        
        JScrollPane scroll = new JScrollPane(resultsList);
        scroll.setPreferredSize(new Dimension(750, 150));
        panel.add(scroll, BorderLayout.CENTER);
        
        return panel;
    }

    private void updatePreview(JLabel label, ObjectTree.Item item) {
        if (item == null) {
            label.setIcon(null);
            return;
        }
        try {
            String iconPath = item.getVar("icon");
            if (iconPath != null && !iconPath.equals("null")) {
                DMI dmi = editor.getDmi(iconPath, false);
                if (dmi != null) {
                    String iconState = item.getVar("icon_state");
                    if (iconState != null) {
                        iconState = iconState.replace("\"", "");
                    }
                    IconSubstate substate = dmi.getIconState(iconState != null ? iconState : "").getSubstate(2);
                    if (substate != null) {
                        label.setIcon(substate.getScaled());
                        return;
                    }
                }
            }
        } catch (Exception e) {
            // Ignore icon loading errors
        }
        label.setIcon(null);
    }

    private void updateButtonStates() {
        boolean hasFindType = selectedFindType != null;
        boolean hasReplaceType = selectedReplaceType != null;
        boolean hasResults = resultsModel.getSize() > 0;
        boolean hasMap = editor.dmm != null;
        
        findButton.setEnabled(hasFindType && hasMap);
        deleteAllButton.setEnabled(hasResults && hasMap);
        replaceAllButton.setEnabled(hasResults && hasReplaceType && hasMap);
    }

    private void collapseTree(JTree tree) {
        for (int i = tree.getRowCount() - 1; i >= 1; i--) {
            tree.collapseRow(i);
        }
    }

    private void expandTree(JTree tree) {
        int oldCount = -1;
        while (tree.getRowCount() != oldCount) {
            oldCount = tree.getRowCount();
            for (int i = 0; i < oldCount; i++) {
                tree.expandRow(i);
            }
        }
    }

    private void applyFindFilter() {
        String query = findSearchField.getText().trim().toLowerCase();
        if (query.isEmpty()) {
            findTree.setModel(editor.objTree);
            collapseTree(findTree);
        } else {
            findTree.setModel(new FilteredTreeModel(editor.objTree, query));
            expandTree(findTree);
        }
    }

    private void applyReplaceFilter() {
        String query = replaceSearchField.getText().trim().toLowerCase();
        if (query.isEmpty()) {
            replaceTree.setModel(editor.objTree);
            collapseTree(replaceTree);
        } else {
            replaceTree.setModel(new FilteredTreeModel(editor.objTree, query));
            expandTree(replaceTree);
        }
    }

    private void performFind() {
        if (selectedFindType == null || editor.dmm == null) {
            return;
        }
        
        resultsModel.clear();
        DMM dmm = editor.dmm;
        boolean currentZOnly = currentZOnlyCheckbox.isSelected();
        boolean exactMatch = matchExactCheckbox.isSelected();
        int currentZ = editor.getCurrentZ();
        
        int count = 0;
        for (Map.Entry<Location, String> entry : dmm.map.entrySet()) {
            Location loc = entry.getKey();
            
            // Z-level filter
            if (currentZOnly && loc.z != currentZ) {
                continue;
            }
            
            String key = entry.getValue();
            TileInstance tile = dmm.instances.get(key);
            if (tile == null) continue;
            
            for (ObjInstance obj : tile.objs) {
                if (obj == null) continue;
                
                boolean matches;
                if (exactMatch) {
                    matches = obj.typeString().equals(selectedFindType);
                } else {
                    matches = obj.istype(selectedFindType);
                }
                
                if (matches) {
                    resultsModel.addElement(new LocationResult(loc, obj.typeString(), obj.getVar("name")));
                    count++;
                }
            }
        }
        
        resultsLabel.setText("Found " + count + " instance(s) of " + selectedFindType);
        updateButtonStates();
    }

    private void performDeleteAll() {
        if (resultsModel.getSize() == 0 || editor.dmm == null) {
            return;
        }
        
        int confirm = JOptionPane.showConfirmDialog(this,
                "Are you sure you want to delete " + resultsModel.getSize() + " instance(s)?\nThis action can be undone.",
                "Confirm Delete",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.WARNING_MESSAGE);
        
        if (confirm != JOptionPane.YES_OPTION) {
            return;
        }
        
        DMM dmm = editor.dmm;
        boolean exactMatch = matchExactCheckbox.isSelected();
        int deletedCount = 0;
        
        // Group by location to avoid modifying the same tile multiple times
        Map<Location, List<String>> toDelete = new HashMap<>();
        for (int i = 0; i < resultsModel.getSize(); i++) {
            LocationResult result = resultsModel.get(i);
            toDelete.computeIfAbsent(result.location, k -> new ArrayList<>()).add(result.typePath);
        }
        
        synchronized (editor) {
            for (Map.Entry<Location, List<String>> entry : toDelete.entrySet()) {
                Location loc = entry.getKey();
                String key = dmm.map.get(loc);
                if (key == null) continue;
                
                TileInstance tile = dmm.instances.get(key);
                if (tile == null) continue;
                
                // Create new object list without the matching types
                List<ObjInstance> newObjs = new ArrayList<>();
                for (ObjInstance obj : tile.objs) {
                    if (obj == null) continue;
                    
                    boolean shouldDelete = false;
                    for (String typeToDelete : entry.getValue()) {
                        if (exactMatch) {
                            if (obj.typeString().equals(typeToDelete)) {
                                shouldDelete = true;
                                break;
                            }
                        } else {
                            if (obj.typeString().equals(typeToDelete) || obj.istype(selectedFindType)) {
                                shouldDelete = true;
                                break;
                            }
                        }
                    }
                    
                    if (!shouldDelete) {
                        newObjs.add(obj);
                    } else {
                        deletedCount++;
                    }
                }
                
                // Create new tile instance and update map
                if (!newObjs.isEmpty()) {
                    TileInstance newTile = new TileInstance(newObjs, dmm);
                    String newKey = dmm.getKeyForInstance(newTile);
                    dmm.putMap(loc, newKey);
                }
            }
            
            // Add to undo stack
            editor.addToUndoStack(dmm.popDiffs());
        }
        
        resultsLabel.setText("Deleted " + deletedCount + " instance(s)");
        resultsModel.clear();
        updateButtonStates();
        
        JOptionPane.showMessageDialog(this,
                "Deleted " + deletedCount + " instance(s).\nUse Ctrl+Z to undo.",
                "Delete Complete",
                JOptionPane.INFORMATION_MESSAGE);
    }

    private void performReplaceAll() {
        if (resultsModel.getSize() == 0 || selectedReplaceType == null || editor.dmm == null) {
            return;
        }
        
        int confirm = JOptionPane.showConfirmDialog(this,
                "Are you sure you want to replace " + resultsModel.getSize() + " instance(s) with " + selectedReplaceType + "?\nThis action can be undone.",
                "Confirm Replace",
                JOptionPane.YES_NO_OPTION,
                JOptionPane.WARNING_MESSAGE);
        
        if (confirm != JOptionPane.YES_OPTION) {
            return;
        }
        
        DMM dmm = editor.dmm;
        ObjectTree.Item replaceItem = editor.objTree.get(selectedReplaceType);
        if (replaceItem == null) {
            JOptionPane.showMessageDialog(this, "Replace type not found in object tree.", "Error", JOptionPane.ERROR_MESSAGE);
            return;
        }
        
        boolean exactMatch = matchExactCheckbox.isSelected();
        int replacedCount = 0;
        
        // Group by location
        Map<Location, List<String>> toReplace = new HashMap<>();
        for (int i = 0; i < resultsModel.getSize(); i++) {
            LocationResult result = resultsModel.get(i);
            toReplace.computeIfAbsent(result.location, k -> new ArrayList<>()).add(result.typePath);
        }
        
        synchronized (editor) {
            for (Map.Entry<Location, List<String>> entry : toReplace.entrySet()) {
                Location loc = entry.getKey();
                String key = dmm.map.get(loc);
                if (key == null) continue;
                
                TileInstance tile = dmm.instances.get(key);
                if (tile == null) continue;
                
                // Create new object list with replacements
                List<ObjInstance> newObjs = new ArrayList<>();
                Set<String> alreadyReplaced = new HashSet<>(); // Avoid duplicating replacement
                
                for (ObjInstance obj : tile.objs) {
                    if (obj == null) continue;
                    
                    boolean shouldReplace = false;
                    for (String typeToReplace : entry.getValue()) {
                        if (exactMatch) {
                            if (obj.typeString().equals(typeToReplace)) {
                                shouldReplace = true;
                                break;
                            }
                        } else {
                            if (obj.typeString().equals(typeToReplace) || obj.istype(selectedFindType)) {
                                shouldReplace = true;
                                break;
                            }
                        }
                    }
                    
                    if (shouldReplace) {
                        // Add replacement instead, but only once per location
                        if (!alreadyReplaced.contains(selectedReplaceType)) {
                            newObjs.add(replaceItem);
                            alreadyReplaced.add(selectedReplaceType);
                        }
                        replacedCount++;
                    } else {
                        newObjs.add(obj);
                    }
                }
                
                // Create new tile instance and update map
                if (!newObjs.isEmpty()) {
                    TileInstance newTile = new TileInstance(newObjs, dmm);
                    String newKey = dmm.getKeyForInstance(newTile);
                    dmm.putMap(loc, newKey);
                }
            }
            
            // Add to undo stack
            editor.addToUndoStack(dmm.popDiffs());
        }
        
        resultsLabel.setText("Replaced " + replacedCount + " instance(s) with " + selectedReplaceType);
        resultsModel.clear();
        updateButtonStates();
        
        JOptionPane.showMessageDialog(this,
                "Replaced " + replacedCount + " instance(s).\nUse Ctrl+Z to undo.",
                "Replace Complete",
                JOptionPane.INFORMATION_MESSAGE);
    }

    private void goToSelected() {
        LocationResult selected = resultsList.getSelectedValue();
        if (selected == null || editor.dmm == null) {
            return;
        }
        
        // Navigate viewport to the location
        editor.viewportX = selected.location.x;
        editor.viewportY = selected.location.y;
        editor.setCurrentZ(selected.location.z);
    }

    /**
     * Represents a search result with location and type info.
     */
    private static class LocationResult {
        final Location location;
        final String typePath;
        final String name;
        
        LocationResult(Location location, String typePath, String name) {
            this.location = new Location(location.x, location.y, location.z);
            this.typePath = typePath;
            this.name = name;
        }
        
        @Override
        public String toString() {
            String displayName = (name != null && !name.equals("null")) ? name : typePath;
            return String.format("(%d, %d, %d) - %s [%s]", location.x, location.y, location.z, displayName, typePath);
        }
    }

    /**
     * Renderer for the results list.
     */
    private class LocationResultRenderer extends DefaultListCellRenderer {
        @Override
        public Component getListCellRendererComponent(JList<?> list, Object value, int index, boolean isSelected, boolean cellHasFocus) {
            super.getListCellRendererComponent(list, value, index, isSelected, cellHasFocus);
            if (value instanceof LocationResult) {
                LocationResult result = (LocationResult) value;
                setText(result.toString());
                
                // Try to show icon
                try {
                    ObjectTree.Item item = editor.objTree.get(result.typePath);
                    if (item != null) {
                        String iconPath = item.getVar("icon");
                        if (iconPath != null && !iconPath.equals("null")) {
                            DMI dmi = editor.getDmi(iconPath, false);
                            if (dmi != null) {
                                String iconState = item.getVar("icon_state");
                                if (iconState != null) iconState = iconState.replace("\"", "");
                                IconSubstate substate = dmi.getIconState(iconState != null ? iconState : "").getSubstate(2);
                                if (substate != null) {
                                    setIcon(substate.getScaled());
                                }
                            }
                        }
                    }
                } catch (Exception e) {
                    // Ignore
                }
            }
            return this;
        }
    }

    /**
     * TreeModel that filters ObjectTree items by a case-insensitive substring match.
     */
    private static class FilteredTreeModel implements TreeModel {
        private final ObjectTree base;
        private final String query;

        FilteredTreeModel(ObjectTree base, String queryLower) {
            this.base = base;
            this.query = queryLower;
        }

        @Override public Object getRoot() { return base.getRoot(); }
        @Override public Object getChild(Object parent, int index) { return filteredChildren(parent).get(index); }
        @Override public int getChildCount(Object parent) { return filteredChildren(parent).size(); }
        @Override public boolean isLeaf(Object node) { return filteredChildren(node).isEmpty(); }
        @Override public void valueForPathChanged(TreePath path, Object newValue) { }
        @Override public int getIndexOfChild(Object parent, Object child) { return filteredChildren(parent).indexOf(child); }
        @Override public void addTreeModelListener(javax.swing.event.TreeModelListener l) { }
        @Override public void removeTreeModelListener(javax.swing.event.TreeModelListener l) { }

        private java.util.List<Object> filteredChildren(Object parent) {
            java.util.List<Object> out = new java.util.ArrayList<>();
            if (parent == base) {
                int count = base.getChildCount(parent);
                for (int i = 0; i < count; i++) {
                    Object ch = base.getChild(parent, i);
                    if (matchesOrHasMatches(ch)) out.add(ch);
                }
            } else if (parent instanceof ObjectTree.Item) {
                ObjectTree.Item it = (ObjectTree.Item) parent;
                for (ObjectTree.Item sub : it.subtypes) {
                    if (matchesOrHasMatches(sub)) out.add(sub);
                }
            }
            return out;
        }

        private boolean matchesOrHasMatches(Object node) {
            if (node instanceof ObjectTree.Item) {
                ObjectTree.Item it = (ObjectTree.Item) node;
                if (it.path != null && it.path.toLowerCase().contains(query)) return true;
                for (ObjectTree.Item sub : it.subtypes) if (matchesOrHasMatches(sub)) return true;
            } else if (node == base) {
                int count = base.getChildCount(node);
                for (int i = 0; i < count; i++) if (matchesOrHasMatches(base.getChild(node, i))) return true;
            }
            return false;
        }
    }
}
