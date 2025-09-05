package com.github.monster860.fastdmm.attached;

import com.github.monster860.fastdmm.FastDMM;
import com.github.monster860.fastdmm.editing.ui.ObjectTreeRenderer;
import com.github.monster860.fastdmm.objtree.ObjectTree;
import com.github.monster860.fastdmm.objtree.ObjectTree.Item;

import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
import java.awt.*;
import java.util.ArrayList;
import java.util.List;

/**
 * Picker dialog that shows the actual Object Tree with icons and allows selecting one or more items.
 */
public class ObjectPickerDialog extends JDialog {
    private final FastDMM editor;
    private final JTextField search = new JTextField();
    private final JTree tree;
    private final TreeModel baseModel;
    private List<String> result = new ArrayList<>();

    public ObjectPickerDialog(Frame owner, FastDMM editor) {
        super(owner, "Select Object(s)", true);
        this.editor = editor;
        setMinimumSize(new Dimension(500, 600));

    // Build tree using the live ObjectTree model
    tree = new JTree(editor.objTree);
    baseModel = editor.objTree;
        tree.setCellRenderer(new ObjectTreeRenderer(editor));
        tree.getSelectionModel().setSelectionMode(TreeSelectionModel.DISCONTIGUOUS_TREE_SELECTION);
    // Start collapsed: only show top-level roots
    collapseAll();

        buildUI();
    }

    private void buildUI() {
        setLayout(new BorderLayout(8,8));
        JPanel top = new JPanel(new BorderLayout(4,4));
        top.add(new JLabel("Search:"), BorderLayout.WEST);
        top.add(search, BorderLayout.CENTER);
        add(top, BorderLayout.NORTH);

        JScrollPane scroll = new JScrollPane(tree);
        add(scroll, BorderLayout.CENTER);

        JPanel btns = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JButton add = new JButton("Add Selected");
        JButton cancel = new JButton("Cancel");
        btns.add(add);
        btns.add(cancel);
        add(btns, BorderLayout.SOUTH);

        add.addActionListener(e -> {
            result = new ArrayList<>();
            TreePath[] paths = tree.getSelectionPaths();
            if (paths != null) {
                for (TreePath p : paths) {
                    Object last = p.getLastPathComponent();
                    if (last instanceof ObjectTree.Item) {
                        result.add(((Item) last).path);
                    }
                }
            }
            dispose();
        });
        cancel.addActionListener(e -> {
            result = new ArrayList<>();
            dispose();
        });

        search.getDocument().addDocumentListener(new DocumentListener() {
            @Override public void insertUpdate(DocumentEvent e) { applyFilter(search.getText()); }
            @Override public void removeUpdate(DocumentEvent e) { applyFilter(search.getText()); }
            @Override public void changedUpdate(DocumentEvent e) { applyFilter(search.getText()); }
        });
    }

    private void collapseAll() {
        for (int i = tree.getRowCount() - 1; i >= 1; i--) tree.collapseRow(i);
    }

    private void applyFilter(String q) {
        String query = q == null ? "" : q.trim().toLowerCase();
        if (query.isEmpty()) {
            tree.setModel(baseModel);
            tree.clearSelection();
            collapseAll();
            return;
        }
        // Wrap the model with a filter that only exposes matching nodes
        tree.setModel(new FilteredTreeModel(editor.objTree, query));
        expandAll();
    }

    private void expandAll() {
        int oldCount = -1;
        while (tree.getRowCount() != oldCount) {
            oldCount = tree.getRowCount();
            for (int i = 0; i < oldCount; i++) tree.expandRow(i);
        }
    }

    private Item findFirstMatch(String query) {
        ObjectTree ot = editor.objTree;
        if (ot == null) return null;
        // Prefer common roots
        String[] roots = new String[]{"/obj","/turf","/area","/mob"};
        for (String r : roots) {
            for (Item it : ot.items.values()) {
                if (it.path != null && it.path.startsWith(r) && it.path.toLowerCase().contains(query)) return it;
            }
        }
        for (Item it : ot.items.values()) {
            if (it.path != null && it.path.toLowerCase().contains(query)) return it;
        }
        return null;
    }

    private TreePath buildPath(Item item) {
        java.util.List<Object> nodes = new ArrayList<>();
        nodes.add(item);
        Item cur = item.parent;
        while (cur != null) {
            nodes.add(0, cur);
            cur = cur.parent;
        }
        nodes.add(0, editor.objTree); // root
        return new TreePath(nodes.toArray());
    }

    public List<String> getSelectedTypePaths() {
        return result;
    }

    /**
     * TreeModel that filters ObjectTree items by a case-insensitive substring match on their type path.
     * Only nodes that match or have matching descendants are exposed.
     */
    private static class FilteredTreeModel implements TreeModel {
        private final ObjectTree base;
        private final String q;

        FilteredTreeModel(ObjectTree base, String queryLower) {
            this.base = base;
            this.q = queryLower;
        }

        @Override public Object getRoot() { return base.getRoot(); }
        @Override public Object getChild(Object parent, int index) { return filteredChildren(parent).get(index); }
        @Override public int getChildCount(Object parent) { return filteredChildren(parent).size(); }
        @Override public boolean isLeaf(Object node) { return filteredChildren(node).isEmpty(); }
        @Override public void valueForPathChanged(TreePath path, Object newValue) { /* no-op */ }
        @Override public int getIndexOfChild(Object parent, Object child) { return filteredChildren(parent).indexOf(child); }
        @Override public void addTreeModelListener(javax.swing.event.TreeModelListener l) { /* no dynamic changes */ }
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
                if (it.path != null && it.path.toLowerCase().contains(q)) return true;
                for (ObjectTree.Item sub : it.subtypes) if (matchesOrHasMatches(sub)) return true;
            } else if (node == base) {
                int count = base.getChildCount(node);
                for (int i = 0; i < count; i++) if (matchesOrHasMatches(base.getChild(node, i))) return true;
            }
            return false;
        }
    }
}
