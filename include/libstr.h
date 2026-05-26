/*
 * libstr.h overlay — tkl_* as standard names for -nostdlib bare-metal link
 */
#include_next <libstr.h>
#undef use_libstr_func_as_std
#define use_libstr_func_as_std  1
#define memset  tkl_memset
#define memcpy  tkl_memcpy
#define memmove tkl_memmove
#define memcmp  tkl_memcmp
#define strcmp  tkl_strcmp
#define strcpy  tkl_strcpy
#define strncpy tkl_strncpy
#define strlen  tkl_strlen
#define strcat  tkl_strcat
#define strncat tkl_strncat
