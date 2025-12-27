#ifndef VIM_COMMANDS_H
#define VIM_COMMANDS_H

/* utils/helpers/vim_commands.h */
#include "../api/dwm_api.h"

/* Vim-style command mode functionality */

/* Command buffer for storing typed commands */
static char command_buffer[256] = {0};
static int command_buffer_pos = 0;
static int command_mode_active = 0;

/* Enter command mode (like pressing : in vim) */
static void
vim_enter_command_mode(const Arg *arg)
{
    (void)arg;
    command_mode_active = 1;
    command_buffer_pos = 0;
    command_buffer[0] = '\0';
}

/* Exit command mode */
static void
vim_exit_command_mode(const Arg *arg)
{
    (void)arg;
    command_mode_active = 0;
    command_buffer_pos = 0;
    command_buffer[0] = '\0';
}

/* Execute command from buffer */
static void
vim_execute_command(const Arg *arg)
{
    (void)arg;
    if (!command_mode_active) return;
    
    command_buffer[command_buffer_pos] = '\0';
    
    /* Parse and execute common vim commands */
    if (strcmp(command_buffer, "q") == 0) {
        vim_quit(NULL);
    } else if (strcmp(command_buffer, "q!") == 0) {
        vim_force_quit(NULL);
    } else if (strcmp(command_buffer, "w") == 0) {
        vim_write(NULL);
    } else if (strcmp(command_buffer, "wq") == 0) {
        vim_save_quit(NULL);
    } else if (strcmp(command_buffer, "x") == 0) {
        vim_save_quit(NULL);
    } else if (strcmp(command_buffer, "e") == 0) {
        vim_edit(NULL);
    } else if (strncmp(command_buffer, "e ", 2) == 0) {
        Arg file_arg = { .v = (const char*[]){ &command_buffer[2], NULL } };
        vim_edit(&file_arg);
    } else if (strcmp(command_buffer, "ls") == 0) {
        vim_list_buffers(NULL);
    } else if (strcmp(command_buffer, "buffers") == 0) {
        vim_list_buffers(NULL);
    } else if (strncmp(command_buffer, "b", 1) == 0) {
        /* Handle buffer switching like :b1, :b2, etc. */
        int buffer_num = atoi(&command_buffer[1]);
        if (buffer_num > 0 && buffer_num <= ntags) {
            Arg tag_arg = { .ui = 1 << (buffer_num - 1) };
            view(&tag_arg);
        }
    } else if (strcmp(command_buffer, "bn") == 0 || strcmp(command_buffer, "bnext") == 0) {
        vim_next_buffer(NULL);
    } else if (strcmp(command_buffer, "bp") == 0 || strcmp(command_buffer, "bprev") == 0) {
        vim_prev_buffer(NULL);
    } else if (strcmp(command_buffer, "tabnew") == 0) {
        vim_new_tab(NULL);
    } else if (strcmp(command_buffer, "tabclose") == 0) {
        vim_close_tab(NULL);
    } else if (strcmp(command_buffer, "tabnext") == 0) {
        vim_next_tab(NULL);
    } else if (strcmp(command_buffer, "tabprev") == 0) {
        vim_prev_tab(NULL);
    } else if (strcmp(command_buffer, "tabfirst") == 0) {
        vim_first_tab(NULL);
    } else if (strcmp(command_buffer, "tablast") == 0) {
        vim_last_tab(NULL);
    } else if (strncmp(command_buffer, "split", 5) == 0) {
        vim_split_horizontal(NULL);
    } else if (strncmp(command_buffer, "vsplit", 6) == 0) {
        vim_split_vertical(NULL);
    } else if (strcmp(command_buffer, "only") == 0) {
        /* Close all other windows - go to monocle layout */
        Arg layout_arg = { .v = &layouts[2] };
        setlayout(&layout_arg);
    } else if (strncmp(command_buffer, "!", 1) == 0) {
        /* Execute shell command */
        Arg shell_arg = { .v = (const char*[]){ "st", "-e", "sh", "-c", &command_buffer[1], NULL } };
        spawn(&shell_arg);
    }
    
    vim_exit_command_mode(NULL);
}

/* Add character to command buffer */
static void
vim_command_add_char(const Arg *arg)
{
    if (!command_mode_active) return;
    
    if (arg->i == XK_Return) {
        vim_execute_command(NULL);
    } else if (arg->i == XK_Escape) {
        vim_exit_command_mode(NULL);
    } else if (arg->i == XK_BackSpace) {
        if (command_buffer_pos > 0) {
            command_buffer_pos--;
            command_buffer[command_buffer_pos] = '\0';
        }
    } else if (isprint(arg->i)) {
        if (command_buffer_pos < sizeof(command_buffer) - 1) {
            command_buffer[command_buffer_pos++] = (char)arg->i;
            command_buffer[command_buffer_pos] = '\0';
        }
    }
}

/* Command history */
static char command_history[10][256] = {{0}};
static int command_history_pos = 0;
static int command_history_count = 0;

/* Add command to history */
static void
vim_add_to_history(const char *cmd)
{
    if (command_history_count < 10) {
        strcpy(command_history[command_history_count], cmd);
        command_history_count++;
    } else {
        /* Shift history up */
        for (int i = 0; i < 9; i++) {
            strcpy(command_history[i], command_history[i + 1]);
        }
        strcpy(command_history[9], cmd);
    }
    command_history_pos = command_history_count;
}

/* Navigate command history */
static void
vim_command_history_up(const Arg *arg)
{
    (void)arg;
    if (!command_mode_active) return;
    
    if (command_history_pos > 0) {
        command_history_pos--;
        strcpy(command_buffer, command_history[command_history_pos]);
        command_buffer_pos = strlen(command_buffer);
    }
}

static void
vim_command_history_down(const Arg *arg)
{
    (void)arg;
    if (!command_mode_active) return;
    
    if (command_history_pos < command_history_count - 1) {
        command_history_pos++;
        strcpy(command_buffer, command_history[command_history_pos]);
        command_buffer_pos = strlen(command_buffer);
    } else if (command_history_pos == command_history_count - 1) {
        /* Clear to empty command */
        command_history_pos = command_history_count;
        command_buffer[0] = '\0';
        command_buffer_pos = 0;
    }
}

/* Tab completion for commands */
static void
vim_command_tab_complete(const Arg *arg)
{
    (void)arg;
    if (!command_mode_active) return;
    
    /* Simple tab completion for common commands */
    const char *commands[] = {
        "q", "q!", "w", "wq", "x", "e", "ls", "buffers", "bn", "bnext", "bp", "bprev",
        "tabnew", "tabclose", "tabnext", "tabprev", "tabfirst", "tablast",
        "split", "vsplit", "only"
    };
    
    int cmd_count = sizeof(commands) / sizeof(commands[0]);
    
    for (int i = 0; i < cmd_count; i++) {
        if (strncmp(commands[i], command_buffer, command_buffer_pos) == 0) {
            strcpy(command_buffer, commands[i]);
            command_buffer_pos = strlen(command_buffer);
            break;
        }
    }
}

/* Search mode functionality */
static int search_mode_active = 0;
static int search_direction = 1; /* 1 for forward, -1 for backward */
static char search_buffer[256] = {0};
static int search_buffer_pos = 0;

/* Enter search mode */
static void
vim_enter_search_mode(const Arg *arg)
{
    (void)arg;
    search_mode_active = 1;
    search_direction = 1; /* Default to forward search */
    search_buffer_pos = 0;
    search_buffer[0] = '\0';
}

/* Enter backward search mode */
static void
vim_enter_backward_search_mode(const Arg *arg)
{
    (void)arg;
    search_mode_active = 1;
    search_direction = -1; /* Backward search */
    search_buffer_pos = 0;
    search_buffer[0] = '\0';
}

/* Exit search mode */
static void
vim_exit_search_mode(const Arg *arg)
{
    (void)arg;
    search_mode_active = 0;
    search_buffer_pos = 0;
    search_buffer[0] = '\0';
}

/* Execute search */
static void
vim_execute_search(const Arg *arg)
{
    (void)arg;
    if (!search_mode_active) return;
    
    search_buffer[search_buffer_pos] = '\0';
    
    /* Simple search implementation - cycle through tags */
    if (search_direction == 1) {
        view_next_tag(NULL);
    } else {
        view_prev_tag(NULL);
    }
    
    vim_exit_search_mode(NULL);
}

/* Add character to search buffer */
static void
vim_search_add_char(const Arg *arg)
{
    if (!search_mode_active) return;
    
    if (arg->i == XK_Return) {
        vim_execute_search(NULL);
    } else if (arg->i == XK_Escape) {
        vim_exit_search_mode(NULL);
    } else if (arg->i == XK_BackSpace) {
        if (search_buffer_pos > 0) {
            search_buffer_pos--;
            search_buffer[search_buffer_pos] = '\0';
        }
    } else if (isprint(arg->i)) {
        if (search_buffer_pos < sizeof(search_buffer) - 1) {
            search_buffer[search_buffer_pos++] = (char)arg->i;
            search_buffer[search_buffer_pos] = '\0';
        }
    }
}

#endif /* VIM_COMMANDS_H */
