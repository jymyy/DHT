#include "fileio.h"
#include "typedefs.h"
#include "hash.h"

int form_path(char *dir, char *fullpath, sha1_t key) {
    char fname[SHA1_STR_LEN];
    if (strlen(dir) + SHA1_STR_LEN > MAX_PATH_LEN) {
        DIE("directory path too long");
    }
    shatostr(key, fname);
    strcpy(fullpath, dir);
    strcat(fullpath, "/");
    strcat(fullpath, fname);
    DEBUG("Formed path: %s\n", fullpath);
    return 0;
}

int write_block(char *dir, sha1_t key, byte *buf, int blocklen) {
    DEBUG("Writing block... ");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    FILE *file = fopen(fullpath, "wb");
    if (file == NULL) {
        DIE("failed to open file");
    }

    int bytes_written = fwrite(buf, sizeof(byte), blocklen, file);
    fclose(file);
    DEBUG("ready");
    return bytes_written;
}

int read_block(char *dir, sha1_t key, byte *buf, int buflen) {
    DEBUG("Reading block... ");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    FILE *file = fopen(fullpath, "rb");
    if (file == NULL) {
        DIE("failed to open file");
    }

    int bytes_read = fread(buf, sizeof(byte), buflen, file);
    fclose(file);
    DEBUG("ready");
    return bytes_read;
}

int rm_block(char *dir, sha1_t key) {
    DEBUG("Removing block... ");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    return remove(fullpath);

}