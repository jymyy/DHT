#ifndef HASH_H
#define HASH_H

#include "typedefs.h"

/*
* Hash TCP address and put result in key. Return length of the key.
*/
int hash_addr(struct tcp_addr *addr, sha1_t key);

#endif