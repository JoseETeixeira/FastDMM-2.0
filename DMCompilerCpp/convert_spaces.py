import os
import sys

def spaces_to_tabs(filepath):
    """Convert leading spaces to tabs in a file"""
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    modified = []
    for line in lines:
        # Count leading spaces
        stripped = line.lstrip(' ')
        num_spaces = len(line) - len(stripped)
        
        # Convert spaces to tabs (assuming 4 spaces = 1 tab)
        # But DM typically uses 1 tab per indent level
        # Count indent level by spaces
        if num_spaces > 0:
            # Replace each tab-width of spaces with a tab
            num_tabs = num_spaces // 4  # Assuming 4 spaces per tab
            remainder = num_spaces % 4
            line = '\t' * num_tabs + ' ' * remainder + stripped
        
        modified.append(line)
    
    with open(filepath, 'w', encoding='utf-8', newline='\n') as f:
        f.writelines(modified)
    
    print(f"Converted {filepath}")

# Convert all .dm files in DMStandard/Types that use spaces
types_dir = r"DMStandard\Types"
for filename in os.listdir(types_dir):
    if filename.endswith('.dm'):
        filepath = os.path.join(types_dir, filename)
        spaces_to_tabs(filepath)

# Also convert nested dirs
atoms_dir = os.path.join(types_dir, "Atoms")
if os.path.exists(atoms_dir):
    for filename in os.listdir(atoms_dir):
        if filename.endswith('.dm'):
            filepath = os.path.join(atoms_dir, filename)
            spaces_to_tabs(filepath)

print("Done!")
