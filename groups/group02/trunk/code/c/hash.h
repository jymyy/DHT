#ifndef HASH_H
#define HASH_H

#include "typedefs.h"

int hash_addr(struct tcp_addr *addr, sha1_t *key);

#endif