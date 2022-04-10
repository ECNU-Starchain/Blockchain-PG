/*
 * Updated to C++, zedwood.com 2012
 * Based on Olivier Gay's version
 * See Modified BSD License below:
 *
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Issue date:  04/30/2005
 * http://www.ouah.org/ogay/sha2/
 *
 * Copyright (C) 2005, 2007 Olivier Gay <olivier.gay@a3.epfl.ch>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <string.h>
#include "clientsha256.h"




void init(struct SHA256 *sha256)
{
    sha256->m_h[0] = 0x6a09e667;
    sha256->m_h[1] = 0xbb67ae85;
    sha256->m_h[2] = 0x3c6ef372;
    sha256->m_h[3] = 0xa54ff53a;
    sha256->m_h[4] = 0x510e527f;
    sha256->m_h[5] = 0x9b05688c;
    sha256->m_h[6] = 0x1f83d9ab;
    sha256->m_h[7] = 0x5be0cd19;
    sha256->m_len = 0;
    sha256->m_tot_len = 0;
}

void transform(struct SHA256* sha256, const unsigned char *message, unsigned int block_nb){
    uint32 w[64];
    uint32 wv[8];
    uint32 t1, t2;
    const unsigned char *sub_block;
    int i;
    int j;
    for (i = 0; i < (int) block_nb; i++) {
        sub_block = message + (i << 6);
        for (j = 0; j < 16; j++) {
            SHA2_PACK32(&sub_block[j << 2], &w[j]);
        }
        for (j = 16; j < 64; j++) {
            w[j] =  SHA256_F4(w[j -  2]) + w[j -  7] + SHA256_F3(w[j - 15]) + w[j - 16];
        }
        for (j = 0; j < 8; j++) {
            wv[j] = sha256->m_h[j];
        }
        for (j = 0; j < 64; j++) {
            t1 = wv[7] + SHA256_F2(wv[4]) + SHA2_CH(wv[4], wv[5], wv[6])
                 + sha256_k[j] + w[j];
            t2 = SHA256_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
            wv[7] = wv[6];
            wv[6] = wv[5];
            wv[5] = wv[4];
            wv[4] = wv[3] + t1;
            wv[3] = wv[2];
            wv[2] = wv[1];
            wv[1] = wv[0];
            wv[0] = t1 + t2;
        }
        for (j = 0; j < 8; j++) {
            sha256->m_h[j] += wv[j];
        }
    }
}

void update(struct SHA256* sha256, const unsigned char *message, unsigned int len)
{
    unsigned int block_nb;
    unsigned int new_len, rem_len, tmp_len;
    const unsigned char *shifted_message;
    tmp_len = SHA224_256_BLOCK_SIZE - sha256->m_len;
    rem_len = len < tmp_len ? len : tmp_len;
    memcpy(&sha256->m_block[sha256->m_len], message, rem_len);
    if (sha256->m_len + len < SHA224_256_BLOCK_SIZE) {
        sha256->m_len += len;
        return;
    }
    new_len = len - rem_len;
    block_nb = new_len / SHA224_256_BLOCK_SIZE;
    shifted_message = message + rem_len;
    transform(sha256, sha256->m_block, 1);
    transform(sha256, shifted_message, block_nb);
    rem_len = new_len % SHA224_256_BLOCK_SIZE;
    memcpy(sha256->m_block, &shifted_message[block_nb << 6], rem_len);
    sha256->m_len = rem_len;
    sha256->m_tot_len += (block_nb + 1) << 6;
}

void final(struct SHA256* sha256, unsigned char *digest){
    unsigned int block_nb;
    unsigned int pm_len;
    unsigned int len_b;
    int i;
    block_nb = (1 + ((SHA224_256_BLOCK_SIZE - 9)
                     < (sha256->m_len % SHA224_256_BLOCK_SIZE)));
    len_b = (sha256->m_tot_len + sha256->m_len) << 3;
    pm_len = block_nb << 6;
    memset(sha256->m_block + sha256->m_len, 0, pm_len - sha256->m_len);
    sha256->m_block[sha256->m_len] = 0x80;
    SHA2_UNPACK32(len_b, sha256->m_block + pm_len - 4);
    transform(sha256, sha256->m_block, block_nb);
    for (i = 0 ; i < 8; i++) {
        SHA2_UNPACK32(sha256->m_h[i], &digest[i << 2]);
    }
}



char* sha256(char* input)
{
    unsigned char digest[DIGEST_SIZE];
    memset(digest,0,DIGEST_SIZE);

    struct SHA256* ctx = (SHA256*)malloc(sizeof(SHA256));
    init(ctx);
    update(ctx, (unsigned char*)input, strlen(input));
    final(ctx, digest);
    char buf[2*DIGEST_SIZE+1];
    buf[2*DIGEST_SIZE] = 0;
    int i;
    for (i = 0; i < (DIGEST_SIZE)/2; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    return (buf);
}

void sha256_pg(char* input,char* output)
{
    unsigned char digest[DIGEST_SIZE];
    memset(digest,0,DIGEST_SIZE);

    struct SHA256* ctx = (SHA256*)malloc(sizeof(SHA256));
    init(ctx);
    update(ctx, (unsigned char*)input, strlen(input));
    final(ctx, digest);
    char buf[2*DIGEST_SIZE+1];
    buf[2*DIGEST_SIZE] = 0;
    int i =0;
    for (i = 0; i < (DIGEST_SIZE)/2; i++)
        sprintf(buf+i*2, "%02x", digest[i]);
    strcpy(output,buf);
    free(ctx);
//    return (buf);
}
