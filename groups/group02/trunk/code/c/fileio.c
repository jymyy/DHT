#include "fileio.h"
#include "typedefs.h"
#include "hash.h"

int write_block(char *path, sha1_t key, byte *buf, int blocklen) {
    DEBUG("Writing block... ");
    char fname[SHA1_STR_LEN];
    char fullpath[MAX_PATH_LEN];
    if (strlen(path) + SHA1_STR_LEN > MAX_PATH_LEN) {
        DIE("write path too long");
    }

    shatostr(key, fname);
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, fname);
    FILE *file = fopen(fullpath, "wb");
    if (file == NULL) {
        DIE("failed to open file");
    }

    int bytes_written = fwrite(buf, sizeof(byte), blocklen, file);
    fclose(file);
    DEBUG("ready");
    return bytes_written;
}

int read_block(char *path, sha1_t key, byte *buf, int buflen) {
    DEBUG("Reading block... ");
    char fname[SHA1_STR_LEN];
    char fullpath[MAX_PATH_LEN];
    if (strlen(path) + SHA1_STR_LEN > MAX_PATH_LEN) {
        DIE("read path too long");
    }

    shatostr(key, fname);
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, fname);
    FILE *file = fopen(fullpath, "rb");
    if (file == NULL) {
        DIE("failed to open file");
    }

    int bytes_read = fread(buf, sizeof(byte), buflen, file);
    fclose(file);
    DEBUG("ready");
    return bytes_read;
}