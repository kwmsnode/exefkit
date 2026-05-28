#ifndef EXEFKIT_H
#define EXEFKIT_H

#include <stddef.h>

int exefkit_init_exfid(const unsigned char* exfid);
void exefkit_drawtext(const wchar_t* text, int basex, int basey, int size, unsigned short color);
void exefkit_deinit();

#endif