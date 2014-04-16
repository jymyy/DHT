#include "fileio.h"
#include "typedefs.h"
#include "hash.h"
#include "log.h"

const char *TAG_FILEIO = "File IO";

int form_path(char *dir, char *fullpath, sha1_t key) {
    char fname[SHA1_STR_LEN];
    if (strlen(dir) + SHA1_STR_LEN > MAX_PATH_LEN) {
        DIE(TAG_FILEIO, "Directory path too long");
    }
    shatostr(key, fname);
    strcpy(fullpath, dir);
    strcat(fullpath, "/");
    strcat(fullpath, fname);
    LOG_DEBUG(TAG_FILEIO, "Formed path: %s\n", fullpath);
    return 0;
}

int write_block(char *dir, sha1_t key, byte *buf, int blocklen) {
    LOG_DEBUG(TAG_FILEIO, "Writing block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    FILE *file = fopen(fullpath, "wb");
    if (file == NULL) {
        DIE(TAG_FILEIO, "Failed to open file");
    }

    int bytes_written = fwrite(buf, sizeof(byte), blocklen, file);
    fclose(file);
    return bytes_written;
}

int read_block(char *dir, sha1_t key, byte *buf, int buflen) {
    LOG_DEBUG(TAG_FILEIO, "Reading block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    FILE *file = fopen(fullpath, "rb");
    if (file == NULL) {
        DIE(TAG_FILEIO, "Failed to open file");
    }

    int bytes_read = fread(buf, sizeof(byte), buflen, file);
    fclose(file);
    return bytes_read;
}

int rm_block(char *dir, sha1_t key) {
    LOG_DEBUG(TAG_FILEIO, "Removing block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    return remove(fullpath);

}