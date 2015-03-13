/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * hprose_bytes_io.c                                      *
 *                                                        *
 * hprose bytes io for pecl source file.                  *
 *                                                        *
 * LastModified: Mar 12, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_bytes_io.h"

HPROSE_CLASS_BEGIN_EX(bytes_io, bytes)
    int32_t mark;        
HPROSE_CLASS_END(bytes_io)        

static zend_object_handlers hprose_bytes_io_handlers;

#if PHP_MAJOR_VERSION < 7

static void php_hprose_bytes_io_free(void *object TSRMLS_DC) {
    php_hprose_bytes_io_t *intern = (php_hprose_bytes_io_t *)object;
    if (intern->bytes) {
        hprose_bytes_io_free(intern->bytes);
        intern->bytes = NULL;
    }
    zend_object_std_dtor(&intern->std TSRMLS_CC);
    efree(intern);
}

#if PHP_API_VERSION < 20100412

static zend_object_value php_hprose_bytes_io_new(
    zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    php_hprose_bytes_io_t *intern;
    zval *tmp;

    intern = emalloc(sizeof(php_hprose_bytes_io_t));
    memset(intern, 0, sizeof(php_hprose_bytes_io_t));

    zend_object_std_init(&intern->std, ce TSRMLS_CC);

    zend_hash_copy(
        intern->std.properties, &ce->default_properties,
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_hprose_bytes_io_free,
        NULL TSRMLS_CC);
    retval.handlers = &hprose_bytes_io_handlers;

    return retval;
}

#else

static zend_object_value php_hprose_bytes_io_new(
    zend_class_entry *ce TSRMLS_DC) {
    zend_object_value retval;
    php_hprose_bytes_io_t *intern;

    intern = emalloc(sizeof(php_hprose_bytes_io_t));
    memset(intern, 0, sizeof(php_hprose_bytes_io_t));

    zend_object_std_init(&intern->std, ce TSRMLS_CC);

    object_properties_init(&intern->std, ce);

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)php_hprose_bytes_io_free,
        NULL TSRMLS_CC);
    retval.handlers = &hprose_bytes_io_handlers;

    return retval;
}
#endif

#else

static void php_hprose_bytes_io_free(zend_object *object) {
    php_hprose_bytes_io_t *intern = (php_hprose_bytes_io_t *)((char*)(object) - XtOffsetOf(php_hprose_bytes_io_t, std));
    if (intern->bytes) {
        hprose_bytes_io_free(intern->bytes);
        intern->bytes = NULL;
    }
    zend_object_std_dtor(&intern->std);
}

static zend_object *php_hprose_bytes_io_new(zend_class_entry *ce) {
    php_hprose_bytes_io_t *intern;
    intern = ecalloc(1, sizeof(php_hprose_bytes_io_t) + zend_object_properties_size(ce));
    memset(intern, 0, sizeof(php_hprose_bytes_io_t) + zend_object_properties_size(ce));
    zend_object_std_init(&intern->std, ce);
    object_properties_init(&intern->std, ce);
    intern->std.handlers = &hprose_bytes_io_handlers;
    return &intern->std;
}
#endif

ZEND_METHOD(hprose_bytes_io, __construct) {
    char *buf = NULL;
    int len = 0;
    HPROSE_OBJECT_INTERN(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &buf, &len) == FAILURE) {
        return;
    }
    if (buf) {
        intern->bytes = hprose_bytes_io_create(buf, len);
    }
    else {
        intern->bytes = hprose_bytes_io_new();
    }
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, __destruct) {
    HPROSE_OBJECT_INTERN(bytes_io);
    if (intern->bytes) {
        hprose_bytes_io_free(intern->bytes);
        intern->bytes = NULL;
    }
}

ZEND_METHOD(hprose_bytes_io, close) {
    HPROSE_OBJECT_INTERN(bytes_io);
    hprose_bytes_io_close(intern->bytes);
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, length) {
    HPROSE_OBJECT(bytes_io, bytes);
    RETURN_LONG(bytes->len);
}

ZEND_METHOD(hprose_bytes_io, getc) {
    HPROSE_OBJECT(bytes_io, bytes);
    if (bytes->pos < bytes->len) {
        char *c = hprose_bytes_io_read(bytes, 1);
        RETURN_STRINGL_0(c, 1);
    }
    RETURN_EMPTY_STRING();
}

ZEND_METHOD(hprose_bytes_io, read) {
    char *s;
    long n;
    HPROSE_OBJECT(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    if (bytes->pos + n > bytes->len) {
        n = bytes->len - bytes->pos;
    }
    s = hprose_bytes_io_read(bytes, n);
    RETURN_STRINGL_0(s, n);
}

ZEND_METHOD(hprose_bytes_io, readfull) {
    char *s;
    int32_t l;
    HPROSE_OBJECT(bytes_io, bytes);
    s = hprose_bytes_io_readfull(bytes, &l);
    RETURN_STRINGL_0(s, l);
}

ZEND_METHOD(hprose_bytes_io, readuntil) {
    char *s, *tag;
    int len;
    int32_t l;
    HPROSE_OBJECT(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &tag, &len) == FAILURE) {
        RETURN_NULL();
    }
    if (len > 0) {
        s = hprose_bytes_io_readuntil(bytes, tag[0], &l);
    }
    else {
        s = hprose_bytes_io_readfull(bytes, &l);
    }
    RETURN_STRINGL_0(s, l);
}

ZEND_METHOD(hprose_bytes_io, readString) {
    char *s;
    long n;
    int32_t l;
    HPROSE_OBJECT(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    s = hprose_bytes_io_read_string(bytes, n, &l);
    RETURN_STRINGL_0(s, l);
}

ZEND_METHOD(hprose_bytes_io, mark) {
    HPROSE_OBJECT_INTERN(bytes_io);
    intern->mark = intern->bytes->pos;
}

ZEND_METHOD(hprose_bytes_io, unmark) {
    HPROSE_OBJECT_INTERN(bytes_io);
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, reset) {
    HPROSE_OBJECT_INTERN(bytes_io);
    if (intern->mark != -1) {
        intern->bytes->pos = intern->mark;
    }
}

ZEND_METHOD(hprose_bytes_io, skip) {
    long n;
    HPROSE_OBJECT(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        return;
    }
    if (n > 0) {
        if (n > bytes->len - bytes->pos) {
            bytes->pos = bytes->len;
        }
        else {
            bytes->pos += n;
        }
    }
}

ZEND_METHOD(hprose_bytes_io, eof) {
    HPROSE_OBJECT(bytes_io, bytes);
    RETURN_BOOL(hprose_bytes_io_eof(bytes));
}

ZEND_METHOD(hprose_bytes_io, write) {
    char *str;
    int len;
    long n = -1;
    HPROSE_OBJECT(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &str, &len, &n) == FAILURE) {
        return;
    }
    if (n > len) n = len;
    hprose_bytes_io_write(bytes, str, n);
}

ZEND_METHOD(hprose_bytes_io, toString) {
    HPROSE_OBJECT(bytes_io, bytes);
    RETURN_STRINGL_1(bytes->buf, bytes->len);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_construct_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_long_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_readuntil_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, tag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_write_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_bytes_io_methods[] = {
    ZEND_ME(hprose_bytes_io, __construct, hprose_bytes_io_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_bytes_io, __destruct, hprose_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_bytes_io, close, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, length, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, getc, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, read, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readfull, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readuntil, hprose_bytes_io_readuntil_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readString, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, mark, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, unmark, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, reset, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, skip, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, eof, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, write, hprose_bytes_io_write_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, toString, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(hprose_bytes_io, __toString, toString, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

zend_class_entry *hprose_bytes_io_ce;

HPROSE_STARTUP_FUNCTION(bytes_io) {
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Hprose", "BytesIO", hprose_bytes_io_methods)
    ce.create_object = php_hprose_bytes_io_new;
#if PHP_MAJOR_VERSION < 7
    hprose_bytes_io_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
    memcpy(&hprose_bytes_io_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
#else
    hprose_bytes_io_ce = zend_register_internal_class_ex(&ce, NULL);
    memcpy(&hprose_bytes_io_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    hprose_bytes_io_handlers.offset = XtOffsetOf(php_hprose_bytes_io_t, std);
    hprose_bytes_io_handlers.free_obj = php_hprose_bytes_io_free;
#endif
    zend_register_class_alias("HproseBytesIO", hprose_bytes_io_ce);
    return SUCCESS;
}
