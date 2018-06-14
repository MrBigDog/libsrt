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

#define SHM_BITS_I 16
#define SHM_BITS_S 15
#define SHM_BMAP_INIT_ELEMS 12

#define SHM_MFUN(i, FUN, NM, REF)                                              \
	for (i = 0; i < NM; i++)					       \
		FUN(REF->maps[i]);
#define SHM_MFUNC(i, CHK, FUN, NM, REF)                                        \
	if (CHK)                                                               \
	SHM_MFUN(i, FUN, NM, REF)
#define SHM_MFUN_PP(i, m, FUN) SHM_MFUNC(i, m && *m, FUN, (*m)->nmaps, &(*m))
#define SHM_MFUN_P(i, m, FUN) SHM_MFUNC(i, m, FUN, m->nmaps, m)

#define SHM_HFi(hf, k, TK) hf((TK)k, SHM_BITS_I)
#define SHM_HFs(k) hgen(ss_get_buffer_r(k), ss_get_buffer_size(k), SHM_BITS_S)

#define SHM_KFx(fn, TR, TM, TK, hfm, mf)                                       \
	TR fn(TM hm, const TK k)                                               \
	{                                                                      \
		size_t smid;                                                   \
		RETURN_IF(!hm, S_FALSE);                                       \
		smid = hfm;                                                    \
		return mf(hm->maps[smid], k);                                  \
	}
#define SHM_KFi(fn, TR, TM, TK, hf, mf)                                        \
	SHM_KFx(fn, TR, TM, TK, SHM_HFi(hf, k, TK), mf)
#define SHM_KFs(fn, TR, TM, TK, mf) SHM_KFx(fn, TR, TM, TK, SHM_HFs(k), mf)

#define SHM_INSCHK(hm, smid)                                                   \
	if (!hm->maps[smid]) {                                                 \
		hm->maps[smid] = sm_alloc(hm->t,			       \
					  SHM_BMAP_INIT_ELEMS + (smid % 4));   \
		if (!hm->maps[smid])                                           \
			return S_FALSE;                                        \
	}
#define SHM_INSFx(fn, TK, TV, hfm, mf)                                         \
	srt_bool fn(srt_hmap **hm, const TK k, const TV v)                     \
	{                                                                      \
		size_t smid;                                                   \
		RETURN_IF(!hm || !*hm, S_FALSE);                               \
		smid = hfm;                                                    \
		SHM_INSCHK((*hm), smid)                                        \
		return mf(&(*hm)->maps[smid], k, v);                           \
	}
#define SHM_INSFi(fn, TK, TV, hf, mf)                                          \
	SHM_INSFx(fn, TK, TV, SHM_HFi(hf, k, TK), mf)
#define SHM_INSFs(fn, TK, TV, mf) SHM_INSFx(fn, TK, TV, SHM_HFs(k), mf)

/*
 * Internal functions
 */

S_INLINE uint32_t h64(uint64_t in, size_t hbits)
{
	size_t i;
	uint32_t out;
	const uint32_t hmask = (1 << hbits) - 1;
	for (i = 0, out = 0; i < 64; i += hbits)
		out += (in >> i);
	return out & hmask;
}

S_INLINE uint32_t h32(uint32_t in, size_t hbits)
{
	size_t i;
	uint32_t out;
	const uint32_t hmask = (1 << hbits) - 1;
	for (i = 0, out = 0; i < 32; i += hbits)
		out += (in >> i);
	return out & hmask;
}

S_INLINE uint32_t hgen(const void *in0, size_t in_size, size_t hbits)
{
	const uint8_t *in;
	uint64_t aux, acc;
	size_t i, in_size_c8;
	const uint32_t hmask = (1 << hbits) - 1;
	in = (const uint8_t *)in0;
	in_size_c8 = (in_size / sizeof(aux)) * sizeof(aux);
	for (i = 0, acc = 0; i < in_size_c8; i += sizeof(aux)) {
		memcpy(&aux, in + i, sizeof(aux));
		acc ^= aux;
	}
	if (in_size_c8 < in_size) {
		memset(&aux, 0, sizeof(aux));
		memcpy(&aux, in + i, in_size - in_size_c8);
		acc ^= aux;
	}
	return h64(acc, hbits);
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
	hm->nmaps = 1 << (t == SM_SI || t == SM_SS || t == SM_SP ?
			  SHM_BITS_S : SHM_BITS_I);
	hm->maps = (srt_map **)malloc(sizeof(srt_map *) * hm->nmaps);
	if (!hm->maps) {
		free(hm);
		return NULL;
	}
	memset(hm->maps, 0, sizeof(srt_map *) * hm->nmaps);
	return hm;
}

static void sh_free_aux1(srt_hmap **hm)
{
	size_t i;
	if (hm && *hm) {
		SHM_MFUN_PP(i, hm, sm_free);
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
	SHM_MFUN_PP(i, hm, sm_shrink);
}

srt_hmap *sh_dup(const srt_hmap *src)
{
	size_t i;
	srt_hmap *o;
	RETURN_IF(!src, NULL);
	o = sh_alloc(src->t);
	for (i = 0; i < src->nmaps; i++)
		if (src->maps[i])
			o->maps[i] = sm_dup(src->maps[i]);
	if (!o->maps[i]) {
		sh_free(&o);
		return NULL;
	}
	o->elems = src->elems;
	o->nmaps = src->nmaps;
	return o;
}

void sh_clear(srt_hmap *hm)
{
	size_t i;
	SHM_MFUN_P(i, hm, sm_clear);
}

size_t sh_size(const srt_hmap *hm)
{
	return hm ? hm->elems : 0;
}

srt_bool sh_empty(const srt_hmap *hm)
{
	return sh_size(hm) == 0 ? S_TRUE : S_FALSE;
}

srt_hmap *sh_cpy(srt_hmap **hm, const srt_hmap *src)
{
	int i;
	RETURN_IF(!hm, NULL);
	if (!src) {
		/* Copy with null source: clear the target */
		sh_clear(*hm);
	} else if (!*hm) {
		/* In case of null target: equivalent to sh_dup() */
		*hm = sh_dup(src);
	} else {
		/* Case of copying over a HM with different type */
		*hm = sh_alloc(src->t);
		for (i = 0; i < (*hm)->nmaps; i++)
			if (src->maps[i])
				sm_cpy(&(*hm)->maps[i], src->maps[i]);
	}
	return *hm;
}

/* clang-format off */
SHM_KFi(sh_at_ii32, int32_t, const srt_hmap *, int32_t, h32, sm_at_ii32)
SHM_KFi(sh_at_uu32, uint32_t, const srt_hmap *, uint32_t, h32, sm_at_uu32)
SHM_KFi(sh_at_ii, int64_t,const srt_hmap *, int64_t, h64, sm_at_ii)
SHM_KFi(sh_at_is, const srt_string *, const srt_hmap *, int64_t, h64, sm_at_is)
SHM_KFi(sh_at_ip, const void *, const srt_hmap *, int64_t, h64, sm_at_ip)
SHM_KFs(sh_at_si, int64_t, const srt_hmap *, srt_string *, sm_at_si)
SHM_KFs(sh_at_sp, const void *, const srt_hmap *, srt_string *, sm_at_sp)
SHM_KFs(sh_at_ss, const srt_string *, const srt_hmap *, srt_string *, sm_at_ss)

SHM_KFi(sh_count_u, srt_bool, const srt_hmap *, uint32_t, h32, sm_count_u)
SHM_KFi(sh_count_i, srt_bool, const srt_hmap *, int64_t, h64, sm_count_i)
SHM_KFs(sh_count_s, srt_bool, const srt_hmap *, srt_string *, sm_count_s)

SHM_INSFi(sh_insert_ii32, int32_t, int32_t, h32, sm_insert_ii32)
SHM_INSFi(sh_insert_uu32, uint32_t, uint32_t, h32, sm_insert_uu32)
SHM_INSFi(sh_insert_ii, int64_t, int64_t, h64, sm_insert_ii)
SHM_INSFi(sh_insert_is, int64_t, srt_string *, h64, sm_insert_is)
SHM_INSFi(sh_insert_ip, int64_t, void *, h64, sm_insert_ip)
SHM_INSFs(sh_insert_si, srt_string *, int64_t, sm_insert_si)
SHM_INSFs(sh_insert_ss, srt_string *, srt_string *, sm_insert_ss)
SHM_INSFs(sh_insert_sp, srt_string *, void *, sm_insert_sp)

SHM_INSFi(sh_inc_ii32, int32_t, int32_t, h32, sm_inc_ii32)
SHM_INSFi(sh_inc_uu32, uint32_t, uint32_t, h32, sm_inc_uu32)
SHM_INSFi(sh_inc_ii, int64_t, int64_t, h64, sm_inc_ii)
SHM_INSFs(sh_inc_si, srt_string *, int64_t, sm_inc_si)

SHM_KFi(sh_delete_i, srt_bool, srt_hmap *, int64_t, h64, sm_delete_i)
SHM_KFs(sh_delete_s, srt_bool, srt_hmap *, srt_string *, sm_delete_s)

/* clang-format on */
