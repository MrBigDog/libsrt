/*
 * shmap.c
 *
 * Hash map handling
 *
 * Copyright (c) 2015-2016, F. Aragon. All rights reserved. Released under
 * the BSD 3-Clause License (see the doc/LICENSE file included).
 */

#include "shmap.h"

/*
 * Constants and macros
 */

#define SHMAP_BITS 14
#define SHMAP_BMAPS (1 << SHMAP_BITS)
#define SHMAP_BMAP_INIT_ELEMS 32

#define SHM_FUN(i, CHK, REF, FUN, FPFREF)                                      \
	if (CHK) {                                                             \
		for (i = 0; i < SHMAP_BMAPS; i++)                              \
			FUN(FPFREF->maps[i]);                                  \
	}
#define SHM_FUN_PP(i, m, FUN) SHM_FUN(i, m &&*m, *m, FUN, &(*m))
#define SHM_FUN_P(i, m, FUN) SHM_FUN(i, m, m, FUN, m)

/*
 * Internal functions
 */

S_INLINE uint64_t h64_step(uint64_t in, size_t hbits)
{
	size_t i;
	uint64_t out;
	for (i = 0, out = 0; i < 64; i += hbits)
		out += (in >> i);
	return out + (in >> i);
}

S_INLINE uint32_t h32(uint32_t in, size_t hbits, uint32_t hmask)
{
	size_t i;
	uint32_t out;
	for (i = 0, out = 0; i < 32; i += hbits)
		out += (in >> i);
	return (out + (in >> i)) & hmask;
}

S_INLINE uint32_t h64(uint64_t in, size_t hbits, uint32_t hmask)
{
	return (uint32_t)h64_step(in, hbits) & hmask;
}

S_INLINE uint32_t hgen(const void *in0, size_t in_size, size_t hbits,
		       uint32_t hmask)
{
	const uint8_t *in;
	uint64_t aux, acc;
	size_t i, in_size_c8;
	in = (const uint8_t *)in0;
	in_size_c8 = (in_size / sizeof(aux)) * sizeof(aux);
	for (i = 0, acc = 0; i < in_size_c8; i += sizeof(aux)) {
		memcpy(&aux, in + i, sizeof(aux));
		acc ^= h64_step(aux, hbits);
	}
	if (in_size_c8 < in_size) {
		memset(&aux, 0, sizeof(aux));
		memcpy(&aux, in + i, in_size - in_size_c8);
		acc ^= h64_step(aux, hbits);
	}
	return (uint32_t)(acc & hmask);
}

/*
 * Allocation
 */

srt_hmap *sh_alloc(const enum eSM_Type t)
{
	srt_hmap *hm;
	hm = (srt_hmap *)malloc(sizeof(srt_hmap));
	if (!hm)
		return NULL;
	hm->t = t;
	hm->elems = 0;
	hm->maps = (srt_map **)malloc(sizeof(srt_map *) * SHMAP_BMAPS);
	if (!hm->maps) {
		free(hm);
		return NULL;
	}
	memset(hm->maps, 0, sizeof(srt_map *) * SHMAP_BMAPS);
	return hm;
}

static void sh_free_aux1(srt_hmap **hm)
{
	size_t i;
	if (hm && *hm) {
		SHM_FUN_PP(i, hm, sm_free);
		s_free(*hm);
		*hm = NULL;
	}
}

void sh_free_aux(srt_hmap **hm, ...)
{
	va_list ap;
	va_start(ap, hm);
	srt_hmap **next = hm;
	while (!s_varg_tail_ptr_tag(next)) { /* last element tag */
		if (next)
			sh_free_aux1(next);
		next = (srt_hmap **)va_arg(ap, srt_hmap **);
	}
	va_end(ap);
}

void sh_shrink(srt_hmap **hm)
{
	size_t i;
	SHM_FUN_PP(i, hm, sm_shrink);
}

srt_hmap *sh_dup(const srt_hmap *src)
{
	size_t i;
	srt_hmap *o;
	RETURN_IF(!src, NULL);
	o = sh_alloc(src->t);
	for (i = 0; i < SHMAP_BMAPS; i++)
		if (src->maps[i])
			o->maps[i] = sm_dup(src->maps[i]);
	if (!o->maps[i]) {
		sh_free(&o);
		return NULL;
	}
	o->elems = src->elems;
	return o;
}

void sh_clear(srt_hmap *hm)
{
	size_t i;
	SHM_FUN_P(i, hm, sm_clear);
}

/*
 * Accessors
 */

size_t sh_size(const srt_hmap *hm)
{
	return hm ? hm->elems : 0;
}

srt_bool sh_empty(const srt_hmap *hm)
{
	return sh_size(hm) == 0 ? S_TRUE : S_FALSE;
}

/*
 * Copy
 */

srt_hmap *sh_cpy(srt_hmap **hm, const srt_hmap *src)
{
	return NULL; /* TODO/FIXME */
}

/*
 * Random access
 */

int32_t sh_at_ii32(const srt_hmap *hm, const int32_t k)
{
	return 0; /* TODO/FIXME */
}

uint32_t sh_at_uu32(const srt_hmap *hm, const uint32_t k)
{
	return 0; /* TODO/FIXME */
}

int64_t sh_at_ii(const srt_hmap *hm, const int64_t k)
{
	return 0; /* TODO/FIXME */
}

const srt_string *sh_at_is(const srt_hmap *hm, const int64_t k)
{
	return NULL; /* TODO/FIXME */
}

const void *sh_at_ip(const srt_hmap *hm, const int64_t k)
{
	return NULL; /* TODO/FIXME */
}

int64_t sh_at_si(const srt_hmap *hm, const srt_string *k)
{
	return 0; /* TODO/FIXME */
}

const srt_string *sh_at_ss(const srt_hmap *hm, const srt_string *k)
{
	return NULL; /* TODO/FIXME */
}

const void *sh_at_sp(const srt_hmap *hm, const srt_string *k)
{
	return NULL; /* TODO/FIXME */
}

/*
 * Existence check
 */

srt_bool sh_count_u(const srt_hmap *hm, const uint32_t k)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_count_i(const srt_hmap *hm, const int64_t k)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_count_s(const srt_hmap *hm, const srt_string *k)
{
	return S_FALSE; /* TODO/FIXME */
}

/*
 * Insert
 */

srt_bool sh_insert_ii32(srt_hmap **hm, const int32_t k, const int32_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_uu32(srt_hmap **hm, const uint32_t k, const uint32_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_ii(srt_hmap **hm, const int64_t k, const int64_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_is(srt_hmap **hm, const int64_t k, const srt_string *v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_ip(srt_hmap **hm, const int64_t k, const void *v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_si(srt_hmap **hm, const srt_string *k, const int64_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_ss(srt_hmap **hm, const srt_string *k, const srt_string *v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_insert_sp(srt_hmap **hm, const srt_string *k, const void *v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_inc_ii32(srt_hmap **hm, const int32_t k, const int32_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_inc_uu32(srt_hmap **hm, const uint32_t k, const uint32_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_inc_ii(srt_hmap **hm, const int64_t k, const int64_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_inc_si(srt_hmap **hm, const srt_string *k, const int64_t v)
{
	return S_FALSE; /* TODO/FIXME */
}

/*
 * Delete
 */

srt_bool sh_delete_i(srt_hmap *hm, const int64_t k)
{
	return S_FALSE; /* TODO/FIXME */
}

srt_bool sh_delete_s(srt_hmap *hm, const srt_string *k)
{
	return S_FALSE; /* TODO/FIXME */
}

/*
 * Enumeration / export data
 */

size_t sh_itr_ii32(const srt_hmap *hm, int32_t key_min, int32_t key_max,
		   srt_map_it_ii32 f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_uu32(const srt_hmap *hm, uint32_t key_min, uint32_t key_max,
		   srt_map_it_uu32 f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_ii(const srt_hmap *hm, int64_t key_min, int64_t key_max,
		 srt_map_it_ii f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_is(const srt_hmap *hm, int64_t key_min, int64_t key_max,
		 srt_map_it_is f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_ip(const srt_hmap *hm, int64_t key_min, int64_t key_max,
		 srt_map_it_ip f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_si(const srt_hmap *hm, const srt_string *key_min,
		 const srt_string *key_max, srt_map_it_si f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_ss(const srt_hmap *hm, const srt_string *key_min,
		 const srt_string *key_max, srt_map_it_ss f, void *context)
{
	return 0; /* TODO/FIXME */
}

size_t sh_itr_sp(const srt_hmap *hm, const srt_string *key_min,
		 const srt_string *key_max, srt_map_it_sp f, void *context)
{
	return 0; /* TODO/FIXME */
}
