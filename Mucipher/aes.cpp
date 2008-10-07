/* 
 * Cryptographic API.
 *
 * AES Cipher Algorithm.
 *
 * Based on Brian Gladman's code.
 *
 * Linux developers:
 *  Alexander Kjeldaas <astor@fast.no>
 *  Herbert Valerio Riedel <hvr@hvrlab.org>
 *  Kyle McMartin <kyle@debian.org>
 *  Adam J. Richter <adam@yggdrasil.com> (conversion to 2.5 API).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ---------------------------------------------------------------------------
 * Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 * All rights reserved.
 *
 * LICENSE TERMS
 *
 * The free distribution and use of this software in both source and binary
 * form is allowed (with or without changes) provided that:
 *
 *   1. distributions of this source code include the above copyright
 *      notice, this list of conditions and the following disclaimer;
 *
 *   2. distributions in binary form include the above copyright
 *      notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other associated materials;
 *
 *   3. the copyright holder's name is not used to endorse products
 *      built using this software without specific written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this product
 * may be distributed under the terms of the GNU General Public License (GPL),
 * in which case the provisions of the GPL apply INSTEAD OF those given above.
 *
 * DISCLAIMER
 *
 * This software is provided 'as is' with no explicit or implied warranties
 * in respect of its properties, including, but not limited to, correctness
 * and/or fitness for purpose.
 * ---------------------------------------------------------------------------
 */

/* Some changes from the Gladman version:
    s/RIJNDAEL(e_key)/E_KEY/g
    s/RIJNDAEL(d_key)/D_KEY/g
*/
/* Changes by Hyriand:
    replace system includes with museek includes
    implemented (costly) le32 <--> cpu functions
    added block cipher code
*/

#include "mucipher.h"

typedef uint32 u32;
typedef unsigned char u8;


#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32

#define AES_BLOCK_SIZE		16

static inline 
u32 generic_rotr32 (const u32 x, const unsigned bits)
{
	const unsigned n = bits % 32;
	return (x >> n) | (x << (32 - n));
}

static inline 
u32 generic_rotl32 (const u32 x, const unsigned bits)
{
	const unsigned n = bits % 32;
	return (x << n) | (x >> (32 - n));
}

#define rotl generic_rotl32
#define rotr generic_rotr32

/*
 * #define byte(x, nr) ((unsigned char)((x) >> (nr*8))) 
 */
inline static u8
byte(const u32 x, const unsigned n)
{
	return x >> (n << 3);
}

inline static u32 le32_to_cpu(u32 i) {
	unsigned char* x = (unsigned char*)&i;
	return x[0] + (x[1] << 8) + (x[2] << 16) + (x[3] << 24);
}

inline static u32 cpu_to_le32(u32 i) {
	u32 r;
	unsigned char* x = (unsigned char*)&r;
	x[0] = i & 0xff;
	x[1] = (i >> 8) & 0xff;
	x[2] = (i >> 16) & 0xff;
	x[3] = i >> 24;
	return r;
}

#define u32_in(x) le32_to_cpu(*(const u32 *)(x))
#define u32_out(to, from) (*(u32 *)(to) = cpu_to_le32(from))


#define E_KEY ctx->E
#define D_KEY ctx->D

static u8 pow_tab[256];
static u8 log_tab[256];
static u8 sbx_tab[256];
static u8 isb_tab[256];
static u32 rco_tab[10];
static u32 ft_tab[4][256];
static u32 it_tab[4][256];

static u32 fl_tab[4][256];
static u32 il_tab[4][256];

static inline u8
f_mult (u8 a, u8 b)
{
	u8 aa = log_tab[a], cc = aa + log_tab[b];

	return pow_tab[cc + (cc < aa ? 1 : 0)];
}

#define ff_mult(a,b)    (a && b ? f_mult(a, b) : 0)

#define f_rn(bo, bi, n, k)					\
    bo[n] =  ft_tab[0][byte(bi[n],0)] ^				\
             ft_tab[1][byte(bi[(n + 1) & 3],1)] ^		\
             ft_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             ft_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rn(bo, bi, n, k)					\
    bo[n] =  it_tab[0][byte(bi[n],0)] ^				\
             it_tab[1][byte(bi[(n + 3) & 3],1)] ^		\
             it_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             it_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

#define ls_box(x)				\
    ( fl_tab[0][byte(x, 0)] ^			\
      fl_tab[1][byte(x, 1)] ^			\
      fl_tab[2][byte(x, 2)] ^			\
      fl_tab[3][byte(x, 3)] )

#define f_rl(bo, bi, n, k)					\
    bo[n] =  fl_tab[0][byte(bi[n],0)] ^				\
             fl_tab[1][byte(bi[(n + 1) & 3],1)] ^		\
             fl_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             fl_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rl(bo, bi, n, k)					\
    bo[n] =  il_tab[0][byte(bi[n],0)] ^				\
             il_tab[1][byte(bi[(n + 3) & 3],1)] ^		\
             il_tab[2][byte(bi[(n + 2) & 3],2)] ^		\
             il_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

static void
gen_tabs (void)
{
	u32 i, t;
	u8 p, q;

	/* log and power tables for GF(2**8) finite field with
	   0x011b as modular polynomial - the simplest prmitive
	   root is 0x03, used here to generate the tables */

	for (i = 0, p = 1; i < 256; ++i) {
		pow_tab[i] = (u8) p;
		log_tab[p] = (u8) i;

		p ^= (p << 1) ^ (p & 0x80 ? 0x01b : 0);
	}

	log_tab[1] = 0;

	for (i = 0, p = 1; i < 10; ++i) {
		rco_tab[i] = p;

		p = (p << 1) ^ (p & 0x80 ? 0x01b : 0);
	}

	for (i = 0; i < 256; ++i) {
		p = (i ? pow_tab[255 - log_tab[i]] : 0);
		q = ((p >> 7) | (p << 1)) ^ ((p >> 6) | (p << 2));
		p ^= 0x63 ^ q ^ ((q >> 6) | (q << 2));
		sbx_tab[i] = p;
		isb_tab[p] = (u8) i;
	}

	for (i = 0; i < 256; ++i) {
		p = sbx_tab[i];

		t = p;
		fl_tab[0][i] = t;
		fl_tab[1][i] = rotl (t, 8);
		fl_tab[2][i] = rotl (t, 16);
		fl_tab[3][i] = rotl (t, 24);

		t = ((u32) ff_mult (2, p)) |
		    ((u32) p << 8) |
		    ((u32) p << 16) | ((u32) ff_mult (3, p) << 24);

		ft_tab[0][i] = t;
		ft_tab[1][i] = rotl (t, 8);
		ft_tab[2][i] = rotl (t, 16);
		ft_tab[3][i] = rotl (t, 24);

		p = isb_tab[i];

		t = p;
		il_tab[0][i] = t;
		il_tab[1][i] = rotl (t, 8);
		il_tab[2][i] = rotl (t, 16);
		il_tab[3][i] = rotl (t, 24);

		t = ((u32) ff_mult (14, p)) |
		    ((u32) ff_mult (9, p) << 8) |
		    ((u32) ff_mult (13, p) << 16) |
		    ((u32) ff_mult (11, p) << 24);

		it_tab[0][i] = t;
		it_tab[1][i] = rotl (t, 8);
		it_tab[2][i] = rotl (t, 16);
		it_tab[3][i] = rotl (t, 24);
	}
}

#define star_x(x) (((x) & 0x7f7f7f7f) << 1) ^ ((((x) & 0x80808080) >> 7) * 0x1b)

#define imix_col(y,x)       \
    u   = star_x(x);        \
    v   = star_x(u);        \
    w   = star_x(v);        \
    t   = w ^ (x);          \
   (y)  = u ^ v ^ w;        \
   (y) ^= rotr(u ^ t,  8) ^ \
          rotr(v ^ t, 16) ^ \
          rotr(t,24)

/* initialise the key schedule from the user supplied key */

#define loop4(i)                                    \
{   t = rotr(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= E_KEY[4 * i];     E_KEY[4 * i + 4] = t;    \
    t ^= E_KEY[4 * i + 1]; E_KEY[4 * i + 5] = t;    \
    t ^= E_KEY[4 * i + 2]; E_KEY[4 * i + 6] = t;    \
    t ^= E_KEY[4 * i + 3]; E_KEY[4 * i + 7] = t;    \
}

#define loop6(i)                                    \
{   t = rotr(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= E_KEY[6 * i];     E_KEY[6 * i + 6] = t;    \
    t ^= E_KEY[6 * i + 1]; E_KEY[6 * i + 7] = t;    \
    t ^= E_KEY[6 * i + 2]; E_KEY[6 * i + 8] = t;    \
    t ^= E_KEY[6 * i + 3]; E_KEY[6 * i + 9] = t;    \
    t ^= E_KEY[6 * i + 4]; E_KEY[6 * i + 10] = t;   \
    t ^= E_KEY[6 * i + 5]; E_KEY[6 * i + 11] = t;   \
}

#define loop8(i)                                    \
{   t = rotr(t,  8); ; t = ls_box(t) ^ rco_tab[i];  \
    t ^= E_KEY[8 * i];     E_KEY[8 * i + 8] = t;    \
    t ^= E_KEY[8 * i + 1]; E_KEY[8 * i + 9] = t;    \
    t ^= E_KEY[8 * i + 2]; E_KEY[8 * i + 10] = t;   \
    t ^= E_KEY[8 * i + 3]; E_KEY[8 * i + 11] = t;   \
    t  = E_KEY[8 * i + 4] ^ ls_box(t);    \
    E_KEY[8 * i + 12] = t;                \
    t ^= E_KEY[8 * i + 5]; E_KEY[8 * i + 13] = t;   \
    t ^= E_KEY[8 * i + 6]; E_KEY[8 * i + 14] = t;   \
    t ^= E_KEY[8 * i + 7]; E_KEY[8 * i + 15] = t;   \
}

static int
aes_set_key(void *ctx_arg, const u8 *in_key, unsigned int key_len)
{
	struct aes_ctx *ctx = static_cast<struct aes_ctx *>(ctx_arg);
	u32 i, t, u, v, w;

	if (key_len != 16 && key_len != 24 && key_len != 32) {
		return -EINVAL;
	}

	ctx->key_length = key_len;

	E_KEY[0] = u32_in (in_key);
	E_KEY[1] = u32_in (in_key + 4);
	E_KEY[2] = u32_in (in_key + 8);
	E_KEY[3] = u32_in (in_key + 12);

	switch (key_len) {
	case 16:
		t = E_KEY[3];
		for (i = 0; i < 10; ++i)
			loop4 (i);
		break;

	case 24:
		E_KEY[4] = u32_in (in_key + 16);
		t = E_KEY[5] = u32_in (in_key + 20);
		for (i = 0; i < 8; ++i)
			loop6 (i);
		break;

	case 32:
		E_KEY[4] = u32_in (in_key + 16);
		E_KEY[5] = u32_in (in_key + 20);
		E_KEY[6] = u32_in (in_key + 24);
		t = E_KEY[7] = u32_in (in_key + 28);
		for (i = 0; i < 7; ++i)
			loop8 (i);
		break;
	}

	D_KEY[0] = E_KEY[0];
	D_KEY[1] = E_KEY[1];
	D_KEY[2] = E_KEY[2];
	D_KEY[3] = E_KEY[3];

	for (i = 4; i < key_len + 24; ++i) {
		imix_col (D_KEY[i], E_KEY[i]);
	}

	return 0;
}

/* encrypt a block of text */

#define f_nround(bo, bi, k) \
    f_rn(bo, bi, 0, k);     \
    f_rn(bo, bi, 1, k);     \
    f_rn(bo, bi, 2, k);     \
    f_rn(bo, bi, 3, k);     \
    k += 4

#define f_lround(bo, bi, k) \
    f_rl(bo, bi, 0, k);     \
    f_rl(bo, bi, 1, k);     \
    f_rl(bo, bi, 2, k);     \
    f_rl(bo, bi, 3, k)

static void aes_encrypt(void *ctx_arg, u8 *out, const u8 *in)
{
	const struct aes_ctx *ctx = static_cast<const struct aes_ctx *>(ctx_arg);
	u32 b0[4], b1[4];
	const u32 *kp = E_KEY + 4;

	b0[0] = u32_in (in) ^ E_KEY[0];
	b0[1] = u32_in (in + 4) ^ E_KEY[1];
	b0[2] = u32_in (in + 8) ^ E_KEY[2];
	b0[3] = u32_in (in + 12) ^ E_KEY[3];

	if (ctx->key_length > 24) {
		f_nround (b1, b0, kp);
		f_nround (b0, b1, kp);
	}

	if (ctx->key_length > 16) {
		f_nround (b1, b0, kp);
		f_nround (b0, b1, kp);
	}

	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_nround (b0, b1, kp);
	f_nround (b1, b0, kp);
	f_lround (b0, b1, kp);

	u32_out (out, b0[0]);
	u32_out (out + 4, b0[1]);
	u32_out (out + 8, b0[2]);
	u32_out (out + 12, b0[3]);
}

/* decrypt a block of text */

#define i_nround(bo, bi, k) \
    i_rn(bo, bi, 0, k);     \
    i_rn(bo, bi, 1, k);     \
    i_rn(bo, bi, 2, k);     \
    i_rn(bo, bi, 3, k);     \
    k -= 4

#define i_lround(bo, bi, k) \
    i_rl(bo, bi, 0, k);     \
    i_rl(bo, bi, 1, k);     \
    i_rl(bo, bi, 2, k);     \
    i_rl(bo, bi, 3, k)

static void aes_decrypt(void *ctx_arg, u8 *out, const u8 *in)
{
	const struct aes_ctx *ctx = static_cast<const struct aes_ctx *>(ctx_arg);
	u32 b0[4], b1[4];
	const int key_len = ctx->key_length;
	const u32 *kp = D_KEY + key_len + 20;

	b0[0] = u32_in (in) ^ E_KEY[key_len + 24];
	b0[1] = u32_in (in + 4) ^ E_KEY[key_len + 25];
	b0[2] = u32_in (in + 8) ^ E_KEY[key_len + 26];
	b0[3] = u32_in (in + 12) ^ E_KEY[key_len + 27];

	if (key_len > 24) {
		i_nround (b1, b0, kp);
		i_nround (b0, b1, kp);
	}

	if (key_len > 16) {
		i_nround (b1, b0, kp);
		i_nround (b0, b1, kp);
	}

	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_nround (b0, b1, kp);
	i_nround (b1, b0, kp);
	i_lround (b0, b1, kp);

	u32_out (out, b0[0]);
	u32_out (out + 4, b0[1]);
	u32_out (out + 8, b0[2]);
	u32_out (out + 12, b0[3]);
}


static char tabs_genned = 0;

void cipherKeySHA256(struct aes_ctx* ctx, char* key, int len) {
	unsigned char digest[32];
	
	if(! tabs_genned) {
		gen_tabs();
		tabs_genned = 1;
	}
	
	sha256Block((unsigned char*)key, len, digest);
	aes_set_key(ctx, digest, 32);
}

void cipherKeyMD5(struct aes_ctx* ctx, char* key, int len) {
	unsigned char digest[16];
	
	if(! tabs_genned) {
		gen_tabs();
		tabs_genned = 1;
	}
	
	md5Block((unsigned char*)key, len, digest);
	aes_set_key(ctx, digest, 16);
}

void blockCipher(struct aes_ctx* ctx, unsigned char* dataIn, int length, unsigned char* dataOut) {
	unsigned char pad[16];
	unsigned char* block = dataIn;
	unsigned int i = 0;
	
	for(i = 0; static_cast<int>(i) < length / 16; i++) {
		aes_encrypt(ctx, dataOut, block);
		block += 16;
		dataOut += 16;
	}
	
	if(length % 16) {
		for(i = 0; static_cast<int>(i) < length % 16; i++) {
			pad[i] = *block;
			block++;
		}
		
		for(; i < 16; i++)
			pad[i] = rand()%256;
		
		aes_encrypt(ctx, dataOut, pad);
	}
}

void blockDecipher(struct aes_ctx* ctx, unsigned char* dataIn, int length, unsigned char* dataOut) {
	unsigned int i;
	
	length = CIPHER_BLOCK(length);
	
	for(i = 0; static_cast<int>(i) < length / 16; i++) {
		aes_decrypt(ctx, dataOut, dataIn);
		dataIn += 16;
		dataOut += 16;
	}
}
