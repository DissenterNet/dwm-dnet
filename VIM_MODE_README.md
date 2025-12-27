# DWM Vim Mode Documentation

This document describes the comprehensive vim motion mode implementation for DWM (Dynamic Window Manager).

## Overview

The vim mode provides a complete vim-like interface for window management in DWM, including normal mode, insert mode, visual mode, and command mode functionality. The implementation closely mimics real vim motions and keybindings.

## Activation

To switch to vim mode, use the keybinding:
- `Mod+Shift+v` (Windows key + Shift + v)

## Modes

### Normal Mode (KEYMODE_VIM)
The primary mode for navigation and window management.

#### Navigation
- `h`, `j`, `k`, `l` - Move focus between windows (left/down/up/right)
- `w` - Move to next window (like vim's "next word")
- `b` - Move to previous window (like vim's "previous word")
- `e` - Move to end of current window (like vim's "end of word")
- `0` - Focus first window in current tag (like vim's "start of line")
- `$` - Focus last window in current tag (like vim's "end of line")
- `^` - Focus first visible window (like vim's "first non-blank")
- `gg` - Go to first tag and focus first window
- `G` - Go to last tag and focus last window
- `f<char>` - Find window forward (simplified)
- `F<char>` - Find window backward (simplified)
- `t<char>` - Till forward (simplified)
- `T<char>` - Till backward (simplified)
- `%` - Toggle between master and stack windows
- `/` - Search forward (cycle to next tag)
- `?` - Search backward (cycle to previous tag)
- `n` - Repeat last search forward
- `N` - Repeat last search backward
- `.` - Repeat last motion

#### Window Management
- `q`, `x` - Close current window (like vim's :q)
- `Q`, `Z` - Force close window (like vim's :q!)
- `space`, `Return` - Zoom/make current window master

#### Layout Management
- `m` - Switch to monocle layout
- `t` - Switch to tile layout
- `f` - Toggle floating
- `F11` - Toggle fullscreen

#### Window Resizing
- `=` - Increase master area size
- `-` - Decrease master area size
- `+` - Increase number of master windows
- `_` - Decrease number of master windows

#### Tag Navigation (Buffer equivalents)
- `1-9` - Switch to specific tag
- `,` - Previous tag
- `.` - Next tag
- `[`, `b` - Previous buffer/tag
- `]`, `B` - Next buffer/tag
- `l` - List buffers/tags

#### Tab Operations
- `Ctrl+t` - New tab (find empty tag and spawn terminal)
- `Ctrl+w` - Close tab (kill all windows on current tag)
- `Ctrl+Tab` - Next tab
- `Ctrl+Shift+Tab` - Previous tab
- `Ctrl+1` - First tab
- `Ctrl+9` - Last tab

#### Splits
- `s` - Horizontal split (spawn new terminal)
- `S` - Vertical split (spawn new terminal)

#### Mode Switching
- `i` - Enter insert mode
- `v`, `V` - Enter visual mode
- `Tab` - Toggle between modes
- `Escape`, `Ctrl+[` - Return to normal mode
- `BackSpace` - Exit vim mode to normal mode

#### Command Mode
- `:` - Enter command mode

### Insert Mode (KEYMODE_VIM_INSERT)
In this mode, most keys are passed through to applications. Only mode switching keys are captured.

#### Mode Switching
- `Escape`, `Ctrl+[`, `Ctrl+R` - Return to normal mode
- `Tab` - Toggle between modes

### Visual Mode (KEYMODE_VIM_VISUAL)
Similar to normal mode but with selection operations.

#### Navigation
All navigation keys from normal mode are available.

#### Visual Operations
- `y`, `Y` - Yank/copy (using xclip)
- `d`, `D`, `x`, `X`, `Delete` - Delete/cut current window
- `c`, `C`, `s`, `S`, `r`, `R` - Change/substitute (delete window)
- `>`, `<` - Indentation (increase/decrease master windows)
- `J` - Join (increase master windows)
- `=` - Format (increase master area)
- `~` - Toggle case (toggle floating)
- `Ctrl+a` - Select all
- `Ctrl+i` - Invert selection

#### Mode Switching
- `v`, `V` - Exit to normal mode
- `Ctrl+v` - Block visual mode
- `Shift+v`, `Shift+V` - Line visual mode
- `Tab` - Toggle between modes
- `Escape`, `Ctrl+[` - Return to normal mode
- `BackSpace`, `Return` - Exit to normal mode

## Command Mode

### Entering Command Mode
Press `:` in normal mode to enter command mode.

### Available Commands
- `:q` - Quit current window
- `:q!` - Force quit current window
- `:w` - Write (no-op in DWM context)
- `:wq`, `:x` - Save and quit (just quit in DWM)
- `:e [file]` - Edit file (open in nvim)
- `:ls`, `:buffers` - List buffers/tags
- `:b[number]` - Switch to buffer/tag (e.g., `:b1` for tag 1)
- `:bn`, `:bnext` - Next buffer/tag
- `:bp`, `:bprev` - Previous buffer/tag
- `:tabnew` - New tab
- `:tabclose` - Close tab
- `:tabnext` - Next tab
- `:tabprev` - Previous tab
- `:tabfirst` - First tab
- `:tablast` - Last tab
- `:split` - Horizontal split
- `:vsplit` - Vertical split
- `:only` - Close all other windows (monocle layout)
- `:!command` - Execute shell command

### Command Mode Features
- **Tab Completion**: Press `Tab` to complete commands
- **History**: Use `Up`/`Down` arrows to navigate command history
- **Execution**: Press `Return` to execute command
- **Cancellation**: Press `Escape` to cancel command mode

## Search Mode

### Forward Search
Press `/` in normal mode to enter forward search mode.

### Backward Search
Press `?` in normal mode to enter backward search mode.

### Search Features
- Type search term and press `Return` to execute
- Press `Escape` to cancel search
- Press `Backspace` to delete characters
- Search cycles through tags (simplified implementation)

## File Structure

The vim mode implementation is organized into several files:

```
utils/
├── helpers/
│   ├── helpers_vim.h      # Vim-style helper functions
│   ├── vim_motions.h      # Vim motion implementations
│   ├── vim_modes.h        # Vim mode management
│   └── vim_commands.h     # Command mode functionality
└── keys/
    ├── keymode_vim.h       # Normal mode keybindings
    ├── keymode_vim_insert.h # Insert mode keybindings
    └── keymode_vim_visual.h # Visual mode keybindings
```

## Integration

The vim mode is integrated into the main DWM configuration through:

1. **Includes**: All vim mode files are included in `config.def.h`
2. **Keymode Constants**: Defined in `dwm.c` (KEYMODE_VIM, KEYMODE_VIM_INSERT, KEYMODE_VIM_VISUAL)
3. **Mode Switching**: Accessible via `Mod+Shift+v` or programmatically

## Key Differences from Vim

While this implementation closely mimics vim, there are some adaptations for the window manager context:

1. **"Words"** are interpreted as windows
2. **"Lines"** are interpreted as window stacks
3. **"Files"** are interpreted as tags/workspaces
4. **"Buffers"** are mapped to tags
5. **"Tabs"** are also mapped to tags
6. **"Splits"** spawn new terminals (DWM doesn't support true window splitting)
7. **"Save"** operations are no-ops or mapped to quit operations

## Customization

The vim mode can be customized by modifying the respective header files:

- Modify `keymode_vim.h` for normal mode keybindings
- Modify `keymode_vim_insert.h` for insert mode keybindings
- Modify `keymode_vim_visual.h` for visual mode keybindings
- Modify `vim_motions.h` for motion behaviors
- Modify `helpers_vim.h` for helper functions
- Modify `vim_commands.h` for command mode functionality

## Troubleshooting

### Common Issues

1. **Keys not responding**: Ensure vim mode is activated with `Mod+Shift+v`
2. **Command mode not working**: Check that `vim_commands.h` is properly included
3. **Visual mode operations failing**: Ensure xclip is installed for yank operations

### Dependencies

- `xclip` - For yank/copy operations in visual mode
- `nvim` - For `:e` command (can be changed to other editors)
- `st` - Default terminal for split operations (can be changed)

## Future Enhancements

Potential improvements for the vim mode:

1. **Enhanced search**: Implement actual window content searching
2. **Marks**: Implement mark functionality for window positions
3. **Macros**: Implement recording and playback of window operations
4. **Registers**: Implement multiple clipboards for yank operations
5. **Count support**: Better support for numeric prefixes (e.g., `3w` for 3 windows forward)
6. **Text objects**: Implement text object-like window groupings
