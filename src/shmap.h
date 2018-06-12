#ifndef SHMAP_H
#define SHMAP_H
#ifdef __cplusplus
extern "C" {
#endif

/*
 * sdmap.h
 *
 * #SHORTDOC hash map handling (key-value storage)
 *
 * #DOC Hash map functions handle key-value storage, which is implemented as a
 * #DOC hash lookup + Red-Black tress (smap) as buckets. The implementation
 * #DOC ensures O(log n) time complexity for insert/read/delete.
 * #DOC
 * #DOC
 * #DOC Supported key/value modes are the same as in smap (enum eSM_Type)
 * #DOC
 *
 * Copyright (c) 2015-2016, F. Aragon. All rights reserved. Released under
 * the BSD 3-Clause License (see the doc/LICENSE file included).
 */

#include "smap.h"

/*
 * Structures and types
 */

typedef struct S_HMap srt_hmap;

struct S_HMap
{
	enum eSM_Type t;
	size_t elems;
	srt_map **maps;
};

/*
 * Allocation
 */

/* #API: |Allocate hash map (heap)|map type|map|O(1)|0;0| */
srt_hmap *sh_alloc(const enum eSM_Type t);

/* #API: |Make the hash map using the minimum possible memory|map|-|O(1) for allocators using memory remap; O(n) for naive allocators|0;0| */
void sh_shrink(srt_hmap **s);

/* #API: |Duplicate hash map|input map|output map|O(n)|0;0| */
srt_hmap *sh_dup(const srt_hmap *src);

/* #API: |Clear/reset map (keeping map type)|map||O(1) for simple maps, O(n) for maps having nodes with strings|0;0| */
void sh_clear(srt_hmap *m);

/*
#API: |Free one or more hash maps|map; more hash maps (optional)|-|O(1) for simple dmaps, O(n) for dmaps having nodes with strings|0;0|
void sh_free(srt_hmap **dm, ...)
*/
#define sh_free(...) sh_free_aux(__VA_ARGS__, S_INVALID_PTR_VARG_TAIL)
void sh_free_aux(srt_hmap **s, ...);

/*
 * Accessors
 */

/* #API: |Get number of elements|str_hmap|Number of elements|O(1)|0;0| */
size_t sh_size(const srt_hmap *hm);

/*#API: |Tells if a map is empty (zero elements)|map|S_TRUE: empty vector; S_FALSE: not empty|O(1)|0;0| */
srt_bool sh_empty(const srt_hmap *hm);

/*
 * Copy
 */

/* #API: |Overwrite map with a map copy|output map; input map|output map reference (optional usage)|O(n)|0;0| */
srt_hmap *sh_cpy(srt_hmap **hm, const srt_hmap *src);

/*
 * Random access
 */

/* #API: |Access to int32-int32 map|map; int32 key|int32|O(log n)|0;0| */
int32_t sh_at_ii32(const srt_hmap *hm, const int32_t k);

/* #API: |Access to uint32-uint32 map|map; uint32 key|uint32|O(log n)|0;0| */
uint32_t sh_at_uu32(const srt_hmap *hm, const uint32_t k);

/* #API: |Access to integer-interger map|map; integer key|integer|O(log n)|0;0| */
int64_t sh_at_ii(const srt_hmap *hm, const int64_t k);

/* #API: |Access to integer-string map|map; integer key|string|O(log n)|0;0| */
const srt_string *sh_at_is(const srt_hmap *hm, const int64_t k);

/* #API: |Access to integer-pointer map|map; integer key|pointer|O(log n)|0;0| */
const void *sh_at_ip(const srt_hmap *hm, const int64_t k);

/* #API: |Access to string-integer map|map; string key|integer|O(log n)|0;0| */
int64_t sh_at_si(const srt_hmap *hm, const srt_string *k);

/* #API: |Access to string-string map|map; string key|string|O(log n)|0;0| */
const srt_string *sh_at_ss(const srt_hmap *hm, const srt_string *k);

/* #API: |Access to string-pointer map|map; string key|pointer|O(log n)|0;0| */
const void *sh_at_sp(const srt_hmap *hm, const srt_string *k);

/*
 * Existence check
 */

/* #API: |Map element count/check|map; 32-bit unsigned integer key|S_TRUE: element found; S_FALSE: not in the map|O(log n)|0;0| */
srt_bool sh_count_u(const srt_hmap *hm, const uint32_t k);

/* #API: |Map element count/check|map; integer key|S_TRUE: element found; S_FALSE: not in the map|O(log n)|0;0| */
srt_bool sh_count_i(const srt_hmap *hm, const int64_t k);

/* #API: |Map element count/check|map; string key|S_TRUE: element found; S_FALSE: not in the map|O(log n)|0;0| */
srt_bool sh_count_s(const srt_hmap *hm, const srt_string *k);

/*
 * Insert
 */

/* #API: |Insert into int32-int32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_ii32(srt_hmap **hm, const int32_t k, const int32_t v);

/* #API: |Insert into uint32-uint32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_uu32(srt_hmap **hm, const uint32_t k, const uint32_t v);

/* #API: |Insert into int-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_ii(srt_hmap **hm, const int64_t k, const int64_t v);

/* #API: |Insert into int-string map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_is(srt_hmap **hm, const int64_t k, const srt_string *v);

/* #API: |Insert into int-pointer map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_ip(srt_hmap **hm, const int64_t k, const void *v);

/* #API: |Insert into string-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_si(srt_hmap **hm, const srt_string *k, const int64_t v);

/* #API: |Insert into string-string map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_ss(srt_hmap **hm, const srt_string *k, const srt_string *v);

/* #API: |Insert into string-pointer map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_insert_sp(srt_hmap **hm, const srt_string *k, const void *v);

/* #API: |Increment value into int32-int32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_inc_ii32(srt_hmap **hm, const int32_t k, const int32_t v);

/* #API: |Increment into uint32-uint32 map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_inc_uu32(srt_hmap **hm, const uint32_t k, const uint32_t v);

/* #API: |Increment into int-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_inc_ii(srt_hmap **hm, const int64_t k, const int64_t v);

/* #API: |Increment into string-int map|map; key; value|S_TRUE: OK, S_FALSE: insertion error|O(log n)|0;0| */
srt_bool sh_inc_si(srt_hmap **hm, const srt_string *k, const int64_t v);

/*
 * Delete
 */

/* #API: |Delete map element|map; integer key|S_TRUE: found and deleted; S_FALSE: not found|O(log n)|0;0| */
srt_bool sh_delete_i(srt_hmap *hm, const int64_t k);

/* #API: |Delete map element|map; string key|S_TRUE: found and deleted; S_FALSE: not found|O(log n)|0;0| */
srt_bool sh_delete_s(srt_hmap *hm, const srt_string *k);

#ifdef __cplusplus
} /* extern "C" { */
#endif
#endif /* #ifndef SHMAP_H */

