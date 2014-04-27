#include "fileio.h"

int form_path(char *dir, char *fullpath, sha1_t key) {
    char fname[SHA1_STR_LEN];
    if (strlen(dir) + SHA1_STR_LEN > MAX_PATH_LEN) {
        LOG_ERROR(TAG_FILE, "Directory path too long");
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
    int status;

    if ((status = create_dir(dir)) == 0) {
        FILE *file = fopen(fullpath, "wb");
        if (file == NULL) {
            LOG_ERROR(TAG_FILE, "Failed to open file");
            return -1;
        } else {
            int bytes_written = fwrite(buf, sizeof(byte), blocklen, file);
            fclose(file);
            return bytes_written;
        }
    } else {
        return -1;
    }
    
}

int read_block(char *dir, sha1_t key, byte *buf, int buflen) {
    LOG_DEBUG(TAG_FILE, "Reading block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    int status;

    if ((status = create_dir(dir)) == 0) {
        FILE *file = fopen(fullpath, "rb");
        if (file == NULL) {
            LOG_ERROR(TAG_FILE, "Failed to open file");
            return -1;
        } else {
            int bytes_read = fread(buf, sizeof(byte), buflen, file);
            fclose(file);
            return bytes_read;
        }
    } else {
        return -1;
    }
    
}

int rm_block(char *dir, sha1_t key) {
    LOG_DEBUG(TAG_FILE, "Removing block");
    char fullpath[MAX_PATH_LEN];
    form_path(dir, fullpath, key);
    return remove(fullpath);

}

int create_dir(char *dir) {
    struct stat statbuf;
    int status;
    
    if ((status = stat(dir, &statbuf)) == -1) {
        if (mkdir(dir, S_IRWXU) != 0) {
            LOG_ERROR(TAG_FILE, "Failed to create block directory: %s", strerror(errno));
            return 1;
        }
    } else if (S_ISREG(statbuf.st_mode)) {
        LOG_ERROR(TAG_FILE, "There is already a file with name %s", dir);
        return 1;
    }
    return 0;
}