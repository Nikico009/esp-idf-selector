#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <dirent.h>

#define MAX_PATH 1024

// Defines a predefined ESP-IDF installation option.
typedef struct {
    const char *display_name;
    const char *branch_or_tag;
} ESPVersionOption;

static int append_path(char *dest, size_t size, const char *base, const char *suffix) {
    size_t base_len = strlen(base);
    size_t suffix_len = strlen(suffix);

    if (base_len >= size || suffix_len >= size - base_len) {
        return 0;
    }

    memcpy(dest, base, base_len);
    memcpy(dest + base_len, suffix, suffix_len + 1);
    return 1;
}

void print_usage(const char *prog_name) {
    printf("ESP-IDF Manager & Launcher\n\n");
    printf("Usage:\n");
    printf("  %s                                Run default ESP-IDF version (if configured)\n", prog_name);
    printf("  %s -v <version>                   Run interactive shell for specified version\n", prog_name);
    printf("  %s -i                             Interactively install a new ESP-IDF version\n", prog_name);
    printf("  %s -l, --list                     List all installed ESP-IDF versions\n", prog_name);
    printf("  %s -u <version>                   Uninstall/remove a specific ESP-IDF version\n", prog_name);
    printf("  %s -d <version>                   Set a version as default for plain execution\n", prog_name);
    printf("  %s -x <version> \"<command>\"       Execute a single command within the environment\n", prog_name);
}

void get_home_dir(char *buf, size_t size) {
    struct passwd *user_entry = getpwuid(getuid());
    if (!user_entry) {
        perror("Failed to get user home directory");
        exit(1);
    }
    snprintf(buf, size, "%s", user_entry->pw_dir);
}

void list_versions(const char *home_dir) {
    char esp_dir[MAX_PATH];
    snprintf(esp_dir, sizeof(esp_dir), "%s/esp", home_dir);

    DIR *directory_stream = opendir(esp_dir);
    if (!directory_stream) {
        printf("No ESP-IDF installations found (directory %s does not exist).\n", esp_dir);
        return;
    }

    printf("=== Installed ESP-IDF Versions ===\n");
    struct dirent *directory_entry;
    int version_count = 0;
    while ((directory_entry = readdir(directory_stream)) != NULL) {
        if (strncmp(directory_entry->d_name, "esp-idf-", 8) == 0) {
            char export_path[MAX_PATH];
            int path_length = snprintf(export_path, sizeof(export_path), "%s/%s/export.sh", esp_dir, directory_entry->d_name);
            if (path_length > 0 && (size_t)path_length < sizeof(export_path)) {
                if (access(export_path, F_OK) == 0) {
                    printf("  - %s\n", directory_entry->d_name + 8);
                    version_count++;
                }
            }
        }
    }
    closedir(directory_stream);

    if (version_count == 0) {
        printf("  (None found)\n");
    }
}

void uninstall_version(const char *home_dir, const char *version) {
    char target_path[MAX_PATH];
    snprintf(target_path, sizeof(target_path), "%s/esp/esp-idf-%s", home_dir, version);

    if (access(target_path, F_OK) != 0) {
        fprintf(stderr, "Error: Version '%s' not found at %s\n", version, target_path);
        return;
    }

    printf("[+] Removing directory: %s\n", target_path);
    char remove_command[MAX_PATH + 32];
    snprintf(remove_command, sizeof(remove_command), "rm -rf \"%s\"", target_path);
    if (system(remove_command) == 0) {
        printf("[✔] Version '%s' successfully uninstalled.\n", version);
    } else {
        fprintf(stderr, "Error: Failed to remove directory.\n");
    }
}

void set_default_version(const char *home_dir, const char *version) {
    char config_dir[MAX_PATH];
    snprintf(config_dir, sizeof(config_dir), "%s/.config/esp-idf", home_dir);

    char create_directory_command[MAX_PATH + 32];
    snprintf(create_directory_command, sizeof(create_directory_command), "mkdir -p \"%s\"", config_dir);
    system(create_directory_command);

    char config_file[MAX_PATH];
    if (!append_path(config_file, sizeof(config_file), config_dir, "/default")) {
        fprintf(stderr, "Error: Configuration path is too long.\n");
        return;
    }

    FILE *config_file_handle = fopen(config_file, "w");
    if (!config_file_handle) {
        perror("Failed to save default version configuration");
        return;
    }
    fprintf(config_file_handle, "%s\n", version);
    fclose(config_file_handle);
    printf("[✔] Default ESP-IDF version set to: %s\n", version);
}

int get_default_version(const char *home_dir, char *version_buf, size_t size) {
    char config_file[MAX_PATH];
    snprintf(config_file, sizeof(config_file), "%s/.config/esp-idf/default", home_dir);

    FILE *config_file_handle = fopen(config_file, "r");
    if (!config_file_handle) return 0;

    if (fgets(version_buf, (int)size, config_file_handle) != NULL) {
        version_buf[strcspn(version_buf, "\r\n")] = 0;
        fclose(config_file_handle);
        return 1;
    }
    fclose(config_file_handle);
    return 0;
}

void load_environment(const char *home_dir, const char *version) {
    char export_script[MAX_PATH];
    snprintf(export_script, sizeof(export_script), "%s/esp/esp-idf-%s/export.sh", home_dir, version);

    if (access(export_script, F_OK) != 0) {
        fprintf(stderr, "Error: ESP-IDF version '%s' not found.\n", version);
        fprintf(stderr, "Use 'esp-idf -l' to see available versions or 'esp-idf -i' to install.\n");
        exit(1);
    }

    printf("[+] Loading ESP-IDF environment (%s)...\n", version);
    printf("[i] Type 'exit' to leave the environment.\n\n");

    char shell_command[MAX_PATH * 2];
    snprintf(shell_command, sizeof(shell_command), ". %s && exec bash", export_script);
    execl("/bin/bash", "bash", "-c", shell_command, NULL);
    perror("Failed to start shell");
    exit(1);
}

void exec_command(const char *home_dir, const char *version, const char *user_command) {
    char export_script[MAX_PATH];
    snprintf(export_script, sizeof(export_script), "%s/esp/esp-idf-%s/export.sh", home_dir, version);

    if (access(export_script, F_OK) != 0) {
        fprintf(stderr, "Error: ESP-IDF version '%s' not found.\n", version);
        exit(1);
    }

    char environment_command[MAX_PATH * 3];
    snprintf(environment_command, sizeof(environment_command), ". %s > /dev/null 2>&1 && %s", export_script, user_command);
    
    int command_status = system(environment_command);
    exit(WEXITSTATUS(command_status));
}

void interactive_install(const char *home_dir) {
    char esp_dir[MAX_PATH];
    snprintf(esp_dir, sizeof(esp_dir), "%s/esp", home_dir);

    struct stat directory_status = {0};
    if (stat(esp_dir, &directory_status) == -1) {
        mkdir(esp_dir, 0755);
    }

    // A curated list of notable ESP-IDF versions and branches.
    ESPVersionOption options[] = {
        {"ESP-IDF v4.4 (Stable / Legacy)", "release/v4.4"},
        {"ESP-IDF v5.0 (Stable)", "release/v5.0"},
        {"ESP-IDF v5.1 (Stable)", "release/v5.1"},
        {"ESP-IDF v5.2 (Stable)", "release/v5.2"},
        {"ESP-IDF v5.3 (Stable)", "release/v5.3"},
        {"ESP-IDF v5.4 (Stable)", "release/v5.4"},
        {"ESP-IDF Master / Latest (Development)", "master"}
    };
    int option_count = (int)(sizeof(options) / sizeof(options[0]));

    printf("=== ESP-IDF Interactive Installer ===\n");
    printf("Select a version to install:\n");
    for (int i = 0; i < option_count; i++) {
        printf("  %d) %s\n", i + 1, options[i].display_name);
    }
    printf("  %d) Enter custom branch/tag manually\n", option_count + 1);
    printf("Selection [1-%d]: ", option_count + 1);

    int selected_option = 0;
    if (scanf("%d", &selected_option) != 1) {
        fprintf(stderr, "Invalid input.\n");
        return;
    }

    char version_reference[256];
    if (selected_option >= 1 && selected_option <= option_count) {
        strcpy(version_reference, options[selected_option - 1].branch_or_tag);
    } else if (selected_option == option_count + 1) {
        printf("Enter branch, tag or version name (e.g., release/v5.5, v5.3.1): ");
        if (scanf("%255s", version_reference) != 1) {
            fprintf(stderr, "Invalid input.\n");
            return;
        }
    } else {
        fprintf(stderr, "Invalid selection.\n");
        return;
    }

    char directory_version_name[256];
    strcpy(directory_version_name, version_reference);
    for (int i = 0; directory_version_name[i] != '\0'; i++) {
        if (directory_version_name[i] == '/') directory_version_name[i] = '-';
    }

    char target_path[MAX_PATH];
    char target_prefix[MAX_PATH];
    if (!append_path(target_prefix, sizeof(target_prefix), esp_dir, "/esp-idf-") ||
        !append_path(target_path, sizeof(target_path), target_prefix, directory_version_name)) {
        fprintf(stderr, "Error: Installation path is too long.\n");
        return;
    }

    if (access(target_path, F_OK) == 0) {
        printf("[-] Directory %s already exists!\n", target_path);
        return;
    }

    char git_command[MAX_PATH * 2];
    if (strcasecmp(version_reference, "master") == 0) {
        snprintf(git_command, sizeof(git_command), "git clone --recursive https://github.com/espressif/esp-idf.git \"%s\"", target_path);
    } else {
        snprintf(git_command, sizeof(git_command), "git clone -b %s --recursive https://github.com/espressif/esp-idf.git \"%s\"", version_reference, target_path);
    }

    printf("[+] Cloning ESP-IDF (%s)...\n", version_reference);
    if (system(git_command) != 0) {
        fprintf(stderr, "Error: Git clone failed. Check branch or tag name.\n");
        return;
    }

    printf("[+] Running install.sh (this may take a few minutes)...\n");
    char install_command[MAX_PATH + 32];
    snprintf(install_command, sizeof(install_command), "cd \"%s\" && ./install.sh", target_path);
    if (system(install_command) != 0) {
        fprintf(stderr, "Error: install.sh failed.\n");
        return;
    }

    printf("\n[✔] Installation completed successfully!\n");
    printf("[i] Run it using: esp-idf -v %s\n", directory_version_name);
}

int main(int argc, char *argv[]) {
    char home_dir[MAX_PATH];
    get_home_dir(home_dir, sizeof(home_dir));

    if (argc < 2) {
        char default_version[256];
        if (get_default_version(home_dir, default_version, sizeof(default_version))) {
            load_environment(home_dir, default_version);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (strcmp(argv[1], "-v") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Version argument missing.\n");
            print_usage(argv[0]);
            return 1;
        }
        load_environment(home_dir, argv[2]);
    } else if (strcmp(argv[1], "-i") == 0) {
        interactive_install(home_dir);
    } else if (strcmp(argv[1], "-l") == 0 || strcmp(argv[1], "--list") == 0) {
        list_versions(home_dir);
    } else if (strcmp(argv[1], "-u") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Version argument missing for uninstall.\n");
            return 1;
        }
        uninstall_version(home_dir, argv[2]);
    } else if (strcmp(argv[1], "-d") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Version argument missing for default.\n");
            return 1;
        }
        set_default_version(home_dir, argv[2]);
    } else if (strcmp(argv[1], "-x") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Missing version or command for execution.\n");
            print_usage(argv[0]);
            return 1;
        }
        exec_command(home_dir, argv[2], argv[3]);
    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}