/* 
 * Cryptographic API.
 *
 * Support for Safenet hardware crypto engine.
 *
 * AES Cipher Algorithm.
 *
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <asm/byteorder.h>
#include <linux/udm.h>

#define AES_MIN_KEY_SIZE	16
#define AES_MAX_KEY_SIZE	32

#define AES_BLOCK_SIZE		16

struct aes_ctx {
	int key_length;
	char key[AES_MAX_KEY_SIZE];
};


static int udm_aes_set_key(void *ctx_arg, const u8 *in_key, unsigned int key_len, u32 *flags)
{
	struct aes_ctx *ctx = ctx_arg;

	if (key_len != 16 && key_len != 24 && key_len != 32) {
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}

	ctx->key_length = key_len;

	memcpy(ctx->key, in_key, key_len);
	return 0;
}

static void udm_aes_encrypt(void *ctx_arg, u8 *out, const u8 *in)
{
	const struct aes_ctx *ctx = ctx_arg;
	UDM_CRYPT_PARAMS params;

	params.input = in;
	params.output = out;
	params.key = (const char *)((struct aes_ctx *)ctx)->key;
	params.key_length = ctx->key_length;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_AES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = AES_BLOCK_SIZE;

	udm_crypt(&params);
}

static void udm_aes_decrypt(void *ctx_arg, u8 *out, const u8 *in)
{
	const struct aes_ctx *ctx = ctx_arg;
	UDM_CRYPT_PARAMS params;

	params.input = in;
	params.output = out;
	params.key = (const char *)((struct aes_ctx *)ctx)->key;
	params.key_length = ctx->key_length;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_AES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = AES_BLOCK_SIZE;
	
	udm_crypt(&params);
}

static unsigned int udm_aes_encrypt_ecb(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{

	UDM_CRYPT_PARAMS params;

	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (char const *) ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key_length;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_AES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = nbytes;

	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_aes_decrypt_ecb(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;

	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (char const *) ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key_length;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_AES;
	params.crypto_mode = SA_CRYPTO_MODE_ECB;
	params.iv = NULL;
	params.iv_length = 0;
	params.data_length = nbytes;

	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_aes_encrypt_cbc(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;

	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (char const *) ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key_length;
	params.op = SA_OPCODE_ENCRYPT;
	params.algo = SA_CRYPTO_AES;
	params.crypto_mode = SA_CRYPTO_MODE_CBC;
	params.iv = desc->info;
	params.iv_length = 16;
	params.data_length = nbytes;

	udm_crypt(&params);

	return nbytes;
}

static unsigned int udm_aes_decrypt_cbc(const struct cipher_desc *desc, u8 *out, const u8 *in, unsigned int nbytes)
{
	UDM_CRYPT_PARAMS params;

	params.input = in;
	params.output = out;
	/* Jerry: Extract key from "struct cipher_desc"*/
	params.key = (char const *) ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key;
	params.key_length = ((struct aes_ctx *)crypto_tfm_ctx(desc->tfm))->key_length;
	params.op = SA_OPCODE_DECRYPT;
	params.algo = SA_CRYPTO_AES;
	params.crypto_mode = SA_CRYPTO_MODE_CBC;
	params.iv = desc->info;
	params.iv_length = 16;
	params.data_length = nbytes;

	udm_crypt(&params);

	return nbytes;
}

static struct crypto_alg aes_alg = {
	.cra_name		=	"aes",
	.cra_driver_name	=	"udm-aes-generic",
	.cra_priority		=	100,
	.cra_flags		=	CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize		=	AES_BLOCK_SIZE,
	.cra_ctxsize		=	sizeof(struct aes_ctx),
	.cra_alignmask		=	3,
	.cra_module		=	THIS_MODULE,
	.cra_list		=	LIST_HEAD_INIT(aes_alg.cra_list),
	.cra_u			=	{
		.cipher = {
			.cia_min_keysize	= AES_MIN_KEY_SIZE,
			.cia_max_keysize	= AES_MAX_KEY_SIZE,
			.cia_setkey	   		= udm_aes_set_key,
			.cia_encrypt	 	= udm_aes_encrypt,
			.cia_decrypt	  	= udm_aes_decrypt,
			.cia_encrypt_ecb    = udm_aes_encrypt_ecb,
            .cia_decrypt_ecb    = udm_aes_decrypt_ecb,
			.cia_encrypt_cbc    = udm_aes_encrypt_cbc,
            .cia_decrypt_cbc    = udm_aes_decrypt_cbc,
		}
	}
};

static int __init udm_aes_init(void)
{
	return crypto_register_alg(&aes_alg);
}

static void __exit udm_aes_fini(void)
{
	crypto_unregister_alg(&aes_alg);
}


module_init(udm_aes_init);
module_exit(udm_aes_fini);

MODULE_DESCRIPTION("UDM AES Cipher Algorithm");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry Chen");
