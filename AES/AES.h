#ifndef __AES_H
#define __AES_H
#include "sys.h"

extern u8 w[11][4][4];
extern u8 key[16];

u8* Cipher(u8* input);
u8* InvCipher(u8* input);
void KeyExpansion(u8* key, u8 w[][4][4]);
#endif
