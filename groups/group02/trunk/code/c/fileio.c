#include "fileio.h"

int form_path(char *dir, char *fullpath, sha1_t key) {
    char fname[SHA1_STR_LEN];
    if (strlen(dir) + SHA1_STR_LEN > MAX_PATH_LEN) {
        DIE(TAG_FILE, "Directory path too long");
    }
    shatostr(key, fname, SHA1_STR_LEN);
    strcpy(fullpath, dir);
    strcat(fullpath, "/");
    strcat(fullpath, fname);
    LOG_DEBUG(TAG_FILE, "Formed path: %s\n", fullpath);
    return 0;
}

int write_block(char *dir, sha1_t key, byte *buf, int blocklen) {
    LOG_DEBUG(TAG_FILE, "Writing block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    FILE *file = fopen(fullpath, "wb");
    if (file == NULL) {
        DIE(TAG_FILE, "Failed to open file");
    }

    int bytes_written = fwrite(buf, sizeof(byte), blocklen, file);
    fclose(file);
    return bytes_written;
}

int read_block(char *dir, sha1_t key, byte *buf, int buflen) {
    LOG_DEBUG(TAG_FILE, "Reading block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    FILE *file = fopen(fullpath, "rb");
    if (file == NULL) {
        DIE(TAG_FILE, "Failed to open file");
    }

    int bytes_read = fread(buf, sizeof(byte), buflen, file);
    fclose(file);
    return bytes_read;
}

int rm_block(char *dir, sha1_t key) {
    LOG_DEBUG(TAG_FILE, "Removing block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    return remove(fullpath);

}