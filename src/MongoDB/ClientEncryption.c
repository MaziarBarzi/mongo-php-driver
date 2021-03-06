/*
 * Copyright 2019 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <php.h>
#include <Zend/zend_interfaces.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "phongo_compat.h"
#include "php_phongo.h"

zend_class_entry* php_phongo_clientencryption_ce;

/* {{{ proto MongoDB\BSON\Binary MongoDB\Driver\ClientEncryption::createDataKey(string $kmsProvider[, array $options])
   Creates a new key document and inserts into the key vault collection. */
static PHP_METHOD(ClientEncryption, createDataKey)
{
	char*                          kms_provider     = NULL;
	size_t                         kms_provider_len = 0;
	zval*                          options          = NULL;
	zend_error_handling            error_handling;
	php_phongo_clientencryption_t* intern;

	intern = Z_CLIENTENCRYPTION_OBJ_P(getThis());
	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|a!", &kms_provider, &kms_provider_len, &options) == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}

	zend_restore_error_handling(&error_handling);

	phongo_clientencryption_create_datakey(intern, return_value, kms_provider, options);
} /* }}} */

/* {{{ proto MongoDB\BSON\Binary MongoDB\Driver\ClientEncryption::encrypt(mixed $value[, array $options])
   Encrypts a value with a given key and algorithm */
static PHP_METHOD(ClientEncryption, encrypt)
{
	zval*                          value   = NULL;
	zval*                          options = NULL;
	zend_error_handling            error_handling;
	php_phongo_clientencryption_t* intern;

	intern = Z_CLIENTENCRYPTION_OBJ_P(getThis());
	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|a!", &value, &options) == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}

	zend_restore_error_handling(&error_handling);

	phongo_clientencryption_encrypt(intern, value, return_value, options);
} /* }}} */

/* {{{ proto mixed MongoDB\Driver\ClientEncryption::decrypt(MongoDB\BSON\BinaryInterface $value)
   Decrypts an encrypted value (BSON binary of subtype 6). Returns the original BSON value */
static PHP_METHOD(ClientEncryption, decrypt)
{
	zval*                          ciphertext;
	zend_error_handling            error_handling;
	php_phongo_clientencryption_t* intern;

	intern = Z_CLIENTENCRYPTION_OBJ_P(getThis());
	zend_replace_error_handling(EH_THROW, phongo_exception_from_phongo_domain(PHONGO_ERROR_INVALID_ARGUMENT), &error_handling);
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &ciphertext, php_phongo_binary_interface_ce) == FAILURE) {
		zend_restore_error_handling(&error_handling);
		return;
	}

	zend_restore_error_handling(&error_handling);

	phongo_clientencryption_decrypt(intern, ciphertext, return_value);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_ClientEncryption_createDataKey, 0, 0, 1)
	ZEND_ARG_INFO(0, kmsProvider)
	ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClientEncryption_encrypt, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_ARRAY_INFO(0, options, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClientEncryption_decrypt, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, keyVaultClient, MongoDB\\BSON\\BinaryInterface, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClientEncryption_void, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry php_phongo_clientencryption_me[] = {
	/* clang-format off */
	PHP_ME(ClientEncryption, createDataKey, ai_ClientEncryption_createDataKey, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(ClientEncryption, encrypt, ai_ClientEncryption_encrypt, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_ME(ClientEncryption, decrypt, ai_ClientEncryption_decrypt, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	ZEND_NAMED_ME(__construct, PHP_FN(MongoDB_disabled___construct), ai_ClientEncryption_void, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL)
	ZEND_NAMED_ME(__wakeup, PHP_FN(MongoDB_disabled___wakeup), ai_ClientEncryption_void, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	PHP_FE_END
	/* clang-format on */
};
/* }}} */

/* {{{ MongoDB\Driver\ClientEncryption object handlers */
static zend_object_handlers php_phongo_handler_clientencryption;

static void php_phongo_clientencryption_free_object(zend_object* object) /* {{{ */
{
	php_phongo_clientencryption_t* intern = Z_OBJ_CLIENTENCRYPTION(object);

	zend_object_std_dtor(&intern->std);

	if (intern->client_encryption) {
		mongoc_client_encryption_destroy(intern->client_encryption);
	}

	/* Free the keyVaultClient last to ensure that a potential non-persistent
	 * client outlives the mongoc_client_encryption_t as needed */
	if (!Z_ISUNDEF(intern->key_vault_client_manager)) {
		zval_ptr_dtor(&intern->key_vault_client_manager);
	}
} /* }}} */

static zend_object* php_phongo_clientencryption_create_object(zend_class_entry* class_type) /* {{{ */
{
	php_phongo_clientencryption_t* intern = NULL;

	intern = PHONGO_ALLOC_OBJECT_T(php_phongo_clientencryption_t, class_type);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	intern->std.handlers = &php_phongo_handler_clientencryption;

	return &intern->std;
} /* }}} */

static HashTable* php_phongo_clientencryption_get_debug_info(phongo_compat_object_handler_type* object, int* is_temp) /* {{{ */
{
	php_phongo_clientencryption_t* intern = NULL;
	zval                           retval = ZVAL_STATIC_INIT;

	*is_temp = 1;
	intern   = Z_OBJ_CLIENTENCRYPTION(PHONGO_COMPAT_GET_OBJ(object));

	array_init(&retval);

	return Z_ARRVAL(retval);
} /* }}} */
/* }}} */

void php_phongo_clientencryption_init_ce(INIT_FUNC_ARGS) /* {{{ */
{
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "MongoDB\\Driver", "ClientEncryption", php_phongo_clientencryption_me);
	php_phongo_clientencryption_ce                = zend_register_internal_class(&ce);
	php_phongo_clientencryption_ce->create_object = php_phongo_clientencryption_create_object;
	PHONGO_CE_FINAL(php_phongo_clientencryption_ce);
	PHONGO_CE_DISABLE_SERIALIZATION(php_phongo_clientencryption_ce);

	memcpy(&php_phongo_handler_clientencryption, phongo_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_phongo_handler_clientencryption.get_debug_info = php_phongo_clientencryption_get_debug_info;
	php_phongo_handler_clientencryption.free_obj       = php_phongo_clientencryption_free_object;
	php_phongo_handler_clientencryption.offset         = XtOffsetOf(php_phongo_clientencryption_t, std);

	zend_declare_class_constant_string(php_phongo_clientencryption_ce, ZEND_STRL("AEAD_AES_256_CBC_HMAC_SHA_512_DETERMINISTIC"), MONGOC_AEAD_AES_256_CBC_HMAC_SHA_512_DETERMINISTIC);
	zend_declare_class_constant_string(php_phongo_clientencryption_ce, ZEND_STRL("AEAD_AES_256_CBC_HMAC_SHA_512_RANDOM"), MONGOC_AEAD_AES_256_CBC_HMAC_SHA_512_RANDOM);
} /* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
