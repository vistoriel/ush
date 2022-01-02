// command.c

#include "ush.h"

/*

    command_handle(app, command):
        if command has ~, -, /:
            replace template

        if command has ${}:
            replace variables

        if command has $():
            for subcommand in subcommands:
                command_handle(app, subcommand)

        command_run(app, command)

*/

int command_handle(app_t *app, char **command, bool primary) {
    char *name = NULL;
    char **args = NULL;

    if (command_replace_vars(app, command) != 0) return -1;
    if (command_split_args(app, *command, &name, &args) != 0) return -1;

    command_run(app, name, args);

    //mx_del_strarr(&args);

    if(primary) return 0;
    return 0;
}

int command_replace_vars(app_t *app, char **command) {
    char *str = *command;
    int len = strlen(str);
    t_list *tokens = NULL;
    int vars_len = 0;
    int tokens_len = 0;
    int new_len = 0;
    char *new_command = NULL;

    for (int i = 0; i < len; i++) {
        int token_start = -1;
        int token_length = -1;
        char *token_value = NULL;

        if(i + 1 < len)
            if(str[i] == '$' && str[i + 1] == '{') {
                if(i - 1 >= 0)
                    if(str[i - 1] == '\\') continue;

                token_start = i;

                for (int j = i + 2; j < len; j++) {
                    if(str[j] == '}') {
                        token_length = j - token_start - 2;
                        i = j;
                        break;
                    }
                }

                if(token_length == -1) {
                    mx_printerr("ush: bad substitution\n");
                    return -1;
                }

                token_value = (char *) malloc(token_length * sizeof(char) + 1);
                token_value[token_length] = '\0';

                for (int j = 0; j < token_length; j++) {
                    token_value[j] = str[token_start + j + 2];
                }
            }

        if(token_start != -1) {
            token_t *token = (token_t *) malloc(sizeof(token_t));
            token->start = token_start;
            token->length = token_length;
            token->value = token_value;

            if(tokens == NULL) {
                tokens = mx_create_node(token);
            }
            else {
                mx_push_back(&tokens, token);
            }
        }
    }

    for(t_list *node = tokens; node != NULL; node = node->next) {
        token_t *token = (token_t *)node->data;
        tokens_len = tokens_len + token->length + 3;
        //printf("start: %d\tlength: %d\tname: %s\tvalue: %s\n", token->start, token->length, token->value, var_get(app, token->value));
    }
    for(t_list *node = tokens; node != NULL; node = node->next) {
        token_t *token = (token_t *)node->data;
        char *value = var_get(app, token->value);
        if(value == NULL) value = "";
        vars_len += strlen(value);
    }

    new_len = len - tokens_len + vars_len;

    new_command = (char *) malloc((new_len + 1) * sizeof(char));
    memset(new_command, 0, new_len + 1);

    int last_pos = 0, last_pos_new = 0;
    for(t_list *node = tokens; node != NULL; node = node->next) {
        token_t *token = (token_t *)node->data;
        char *value = var_get(app, token->value);
        if(value == NULL) value = "";
        
        for (int i = last_pos; i < token->start; i++, last_pos_new++) {
            new_command[last_pos_new] = str[i];
        }

        for (unsigned int i = 0; i < strlen(value); i++, last_pos_new++) {
            new_command[last_pos_new] = value[i];
        }

        last_pos = token->start + token->length + 3;
    }
    for (int i = last_pos; i < len; i++, last_pos_new++) {
        new_command[last_pos_new] = str[i];
    }

    free(*command);
    *command = new_command;

    return 0;
}

int command_split_args(app_t *app, char *command, char **name, char ***args) {
    app->is_running = app->is_running;


    if(get_args_count(command) < 0) return -1;

    *args = mx_strsplit(command, ' ');
    *name = args[0][0];

    return 0;
}

int command_run(app_t *app, char *name, char **args) {
    for(t_list *node = app->commands; node != NULL; node = node->next) {
        command_t *command = (command_t *)node->data;

        if(!strcmp(name, command->name)) return command->func(app, args);
    }
    if(mx_get_char_index(name, '=') >= 0) return exec_setvar(app, args);

    return command_launch(app, name, args);

    mx_printerr("ush: command not found: ");
    mx_printerr(name);
    mx_printerr("\n");
    return -1;
}

int command_launch(app_t *app, char *name, char **args) {
    pid_t pid = 0;
    int status = 0;

    pid = fork();
    if (pid == 0) {
        int exec_status = 0;

        signal(SIGTSTP, SIG_DFL);
        exec_status = execvp(name, args);

        if (exec_status == -1) {
            mx_printerr("ush: command not found: ");
            mx_printerr(name);
            mx_printerr("\n");
        }
        
        exit(exec_status);
    } 
    else if (pid < 0) {
        perror("ush");
    } 
    else {
        waitpid(pid, &status, WUNTRACED);

        if (WIFSTOPPED(status)) {
            process_t *process = (process_t *) malloc(sizeof(process_t));
            process->id = mx_list_size(app->processes) + 1;
            process->pid = pid;
            process->name = mx_strdup(name);
            process->status = -2;

            if(app->processes == NULL)
                app->processes = mx_create_node(process);
            else
                mx_push_back(&app->processes, process);

            printf("\n[%d]  + %d suspended  %s\n", process->id, process->pid, process->name);
        }
    }

    return status;
}

void command_add(app_t *app, char *name, int (*func)(app_t *, char **)) {
    command_t *command = (command_t *) malloc(sizeof(command_t));
    command->name = strdup(name);
    command->func = func;

    if(app->commands == NULL)
        app->commands = mx_create_node(command);
    else
        mx_push_back(&app->commands, command);
}