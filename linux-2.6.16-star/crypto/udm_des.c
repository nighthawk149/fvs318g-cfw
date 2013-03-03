/*
 * Cryptographic API.
 *
 * Support for Safenet hardware crypto engine.
 *
 * DES & Triple DES EDE Cipher Algorithms.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <asm/byteorder.h>
#include <linux/bitops.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/types.h>
#include <linux/udm.h>


#define DES_KEY_SIZE		8
#define DES_EXPKEY_WORDS	32
#define DES_BLOCK_SIZE		8

#define DES3_EDE_KEY_SIZE   (3 * DES_KEY_SIZE)
#define DES3_EDE_EXPKEY_WORDS   (3 * DES_EXPKEY_WORDS)
#define DES3_EDE_BLOCK_SIZE DES_BLOCK_SIZE

struct des_ctx {
	u32 key[DES_KEY_SIZE];
};

struct des3_ede_ctx {
	u32 key[DES3_EDE_KEY_SIZE];
};

static int udm_des_setkey(void *ctx, const u8 *key, unsigned int keylen, u32 *flags)
{
	struct des_ctx *dctx = ctx;
	/* Copy to output */
	memcpy(dctx->key, key, sizeof(dctx->key));

	return 0;
}

static void udm_des_encrypt(void *ctx, u8 *dst, const u8 *src)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = src;
	params.output = dst;
	params.key = (const char *) ((struct des_ctx *)ctx)->key;
	params.key_length = 8;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_DES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = 8;

	udm_crypt(&params);
}

static void udm_des_decrypt(void *ctx, u8 *dst, const u8 *src)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = src;
	params.output = dst;
	params.key = (const char *)((struct des_ctx *)ctx)->key; 
	params.key_length = 8;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_DES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = 8;
	
	udm_crypt(&params);
}

static unsigned int udm_des_encrypt_ecb(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *) ((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 8;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_DES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = nbytes;
	
	udm_crypt(&params);
	
	return nbytes;
}

static unsigned int udm_des_decrypt_ecb(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *) ((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 8;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_DES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = nbytes;
	
	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_des_encrypt_cbc(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *) ((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 8;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_DES;
	params.crypto_mode = SA_CRYPTO_MODE_CBC;
	params.iv = desc->info;
	params.iv_length = 8;
	params.data_length = nbytes;

	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_des_decrypt_cbc(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *) ((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 8;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_DES;
	params.crypto_mode = SA_CRYPTO_MODE_CBC;
	params.iv = desc->info;
	params.iv_length = 8;
	params.data_length = nbytes;
	
	udm_crypt(&params);
	
	return nbytes;
}

static struct crypto_alg udm_des_alg = {
	.cra_name		=	"des",
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	DES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct des_ctx),
	.cra_module		=	THIS_MODULE,
	.cra_alignmask		=	3,
	.cra_list		=	LIST_HEAD_INIT(udm_des_alg.cra_list),
	.cra_u			=	{ 
		.cipher = {
			.cia_min_keysize	=	DES_KEY_SIZE,
			.cia_max_keysize	=	DES_KEY_SIZE,
			.cia_setkey			=	udm_des_setkey,
			.cia_encrypt		=	udm_des_encrypt,
			.cia_decrypt		=	udm_des_decrypt,
			.cia_encrypt_ecb    =   udm_des_encrypt_ecb,
			.cia_decrypt_ecb    =   udm_des_decrypt_ecb,
			.cia_encrypt_cbc    =   udm_des_encrypt_cbc,
			.cia_decrypt_cbc    =   udm_des_decrypt_cbc,
		} 
	}
};

/*
 *  * RFC2451:
 *
 * *   For DES-EDE3, there is no known need to reject weak or
 * *   complementation keys.  Any weakness is obviated by the use of
 * *   multiple keys.
 * *
 * *   However, if the first two or last two independent 64-bit keys are
 * *   equal (k1 == k2 or k2 == k3), then the DES3 operation is simply the
 * *   same as DES.  Implementers MUST reject keys that exhibit this
 * *   property.
 *
 * */
static int udm_tdes_setkey(void *ctx, const u8 *key, unsigned int keylen, u32 *flags)
{
	struct des3_ede_ctx *dctx = ctx;
	/* Copy to output */
	memcpy(dctx->key, key, sizeof(dctx->key));

	return 0;
}
#if 1
static void udm_tdes_encrypt(void *ctx, u8 *dst, const u8 *src)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = src;
	params.output = dst;
	params.key = (const char *)((struct des_ctx *)ctx)->key;
	params.key_length = 24;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_TDES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = 8;
	
	udm_crypt(&params);
}

static void udm_tdes_decrypt(void *ctx, u8 *dst, const u8 *src)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = src;
	params.output = dst;
	params.key = (const char *) ((struct des_ctx *)ctx)->key;
	params.key_length = 24;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_TDES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = 8;
	
	udm_crypt(&params);
}

static unsigned int udm_tdes_encrypt_ecb(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (char const *) ((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 24;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_TDES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = nbytes;
	
	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_tdes_decrypt_ecb(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *)((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 24;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_TDES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = nbytes;
	
	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_tdes_encrypt_cbc(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	u8 *iv = desc->info;
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *)((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 24;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_TDES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = iv;
	params.iv_length = 8;
	params.data_length = nbytes;
	
	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_tdes_decrypt_cbc(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	u8 *iv = desc->info;
	UDM_CRYPT_PARAMS params;
	
	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (const char *)((struct des_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = 24;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_TDES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = iv;
	params.iv_length = 8;
	params.data_length = nbytes;
	
	udm_crypt(&params);
	return nbytes;
}

static struct crypto_alg udm_tdes_alg = {
	.cra_name       =   "des3_ede",
	.cra_flags      =   CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize      =   DES3_EDE_BLOCK_SIZE,
	.cra_ctxsize        =   sizeof(struct des3_ede_ctx),
	.cra_module     =   THIS_MODULE,
	.cra_list       =   LIST_HEAD_INIT(udm_tdes_alg.cra_list),
	.cra_u          =   { 
		.cipher = {
			.cia_min_keysize    = DES3_EDE_KEY_SIZE,
			.cia_max_keysize    = DES3_EDE_KEY_SIZE,
			.cia_setkey			= udm_tdes_setkey,
			.cia_encrypt        = udm_tdes_encrypt,
			.cia_decrypt        = udm_tdes_decrypt,
			.cia_encrypt_ecb    = udm_tdes_encrypt_ecb,
			.cia_decrypt_ecb    = udm_tdes_decrypt_ecb,
			.cia_encrypt_cbc    = udm_tdes_encrypt_cbc,
			.cia_decrypt_cbc    = udm_tdes_decrypt_cbc,
		}
	}
};
#endif

static int __init udm_des_init(void)
{
	int ret = 0;

	ret = crypto_register_alg(&udm_des_alg);
	if (ret < 0)
		goto out;
#if 1
	ret = crypto_register_alg(&udm_tdes_alg);
	if (ret < 0)
        crypto_unregister_alg(&udm_tdes_alg);
#endif

out:
	return ret;
}

static void __exit udm_des_fini(void)
{
#if 1
	crypto_unregister_alg(&udm_tdes_alg);
#endif
	crypto_unregister_alg(&udm_des_alg);
}

module_init(udm_des_init);
module_exit(udm_des_fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("UDM DES & Triple DES EDE Cipher Algorithms");
MODULE_AUTHOR("Jerry Chen");
