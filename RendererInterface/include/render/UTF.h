#pragma once

#include "Header.h"

inline size_t utf32_to_utf8 ( const u32 utf32, char *const utf8 ) {
	if ( utf32 <= 0x7F ) {
		utf8[0] = utf32;
		return 1;
	}
	if ( utf32 <= 0x7FF ) {
		utf8[0] = 0xC0 | ( utf32 >> 6 );          /* 110xxxxx */
		utf8[1] = 0x80 | ( utf32 & 0x3F );        /* 10xxxxxx */
		return 2;
	}
	if ( utf32 <= 0xFFFF ) {
		utf8[0] = 0xE0 | ( utf32 >> 12 );         /* 1110xxxx */
		utf8[1] = 0x80 | ( ( utf32 >> 6 ) & 0x3F ); /* 10xxxxxx */
		utf8[2] = 0x80 | ( utf32 & 0x3F );        /* 10xxxxxx */
		return 3;
	}
	if ( utf32 <= 0x10FFFF ) {
		utf8[0] = 0xF0 | ( utf32 >> 18 );         /* 11110xxx */
		utf8[1] = 0x80 | ( ( utf32 >> 12 ) & 0x3F ); /* 10xxxxxx */
		utf8[2] = 0x80 | ( ( utf32 >> 6 ) & 0x3F ); /* 10xxxxxx */
		utf8[3] = 0x80 | ( utf32 & 0x3F );        /* 10xxxxxx */
		return 4;
	}
	return 0;
}
inline size_t utf8_to_utf32 ( const char *const utf8, u32 *utf32 ) {
	if ( utf8[0] < 0x80 ) {
		*utf32 = utf8[0];
		return 1;
	} else if ( ( utf8[0] & 0xE0 ) == 0xC0 ) { /* 110xxxxx 10xxxxxx */
		*utf32 = ( ( u32 ) utf8[0] & 0x3F ) << 6 | ( ( u32 ) utf8[1] & 0x3F );
		return 2;
	} else if ( ( utf8[0] & 0xF0 ) == 0xE0 ) { /* 1110xxxx 10xxxxxx 10xxxxxx */
		*utf32 = ( ( u32 ) utf8[0] & 0x1F ) << 12 | ( ( u32 ) utf8[1] & 0x3F ) << 6 | ( ( u32 ) utf8[2] & 0x3F );
		return 3;
	} else if ( ( utf8[0] & 0xF8 ) == 0xF0 ) { /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
		*utf32 = ( ( u32 ) utf8[0] & 0x0F ) << 18 | ( ( u32 ) utf8[1] & 0x3F ) << 12 | ( ( u32 ) utf8[2] & 0x3F ) << 6 | ( ( u32 ) utf8[3] & 0x3F );
		return 4;
	}
	return 0;
}
inline u32 utf8_to_utf32 ( const char *const utf8 ) {
	if ( utf8[0] < 0x80 ) {
		return utf8[0];
	} else if ( ( utf8[0] & 0xE0 ) == 0xC0 ) { /* 110xxxxx 10xxxxxx */
		return ( ( u32 ) utf8[0] & 0x3F ) << 6 | ( ( u32 ) utf8[1] & 0x3F );
	} else if ( ( utf8[0] & 0xF0 ) == 0xE0 ) { /* 1110xxxx 10xxxxxx 10xxxxxx */
		return ( ( u32 ) utf8[0] & 0x1F ) << 12 | ( ( u32 ) utf8[1] & 0x3F ) << 6 | ( ( u32 ) utf8[2] & 0x3F );
	} else if ( ( utf8[0] & 0xF8 ) == 0xF0 ) { /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
		return ( ( u32 ) utf8[0] & 0x0F ) << 18 | ( ( u32 ) utf8[1] & 0x3F ) << 12 | ( ( u32 ) utf8[2] & 0x3F ) << 6 | ( ( u32 ) utf8[3] & 0x3F );
	}
	return 0;
}
inline u32 utf8_to_u32 ( const char *const utf8 ) {
	if ( utf8[0] < 0x80 ) {
		return utf8[0];
	} else if ( ( utf8[0] & 0xE0 ) == 0xC0 ) { /* 110xxxxx 10xxxxxx */
		return utf8[0] << 8 | utf8[1];
	} else if ( ( utf8[0] & 0xF0 ) == 0xE0 ) { /* 1110xxxx 10xxxxxx 10xxxxxx */
		return utf8[0] << 16 | utf8[1] << 8 | utf8[2];
	} else if ( ( utf8[0] & 0xF8 ) == 0xF0 ) { /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
		return utf8[0] << 24 | utf8[1] << 16 | utf8[2] << 8 | utf8[3];
	}
	return 0;
}
inline size_t u32_to_utf8 ( u32 data, char *const utf8 ) {
	if ( data < 0x100 ) {
		utf8[0] = data;
		return 1;
	} else if ( data < 0x100 ) { /* 110xxxxx 10xxxxxx */
		utf8[0] = data >> 8;
		utf8[1] = data;
		return 2;
	} else if ( data < 0x10000 ) { /* 1110xxxx 10xxxxxx 10xxxxxx */
		utf8[0] = data >> 16;
		utf8[1] = data >> 8;
		utf8[2] = data;
		return 3;
	} else { /* 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
		utf8[0] = data >> 24;
		utf8[1] = data >> 16;
		utf8[2] = data >> 8;
		utf8[3] = data;
		return 4;
	}
}