#pragma once

#include <render/Header.h>

size_t utf32_to_utf8 ( const u32 utf32, unsigned char *const utf8 );
size_t utf8_to_utf32 ( const char *const utf8, u32 *utf32 );
u32 utf8_to_utf32 ( const char *const utf8 );