/*-*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Fluent Bit
 *  ==========
 *  Copyright (C) 2019-2020 The Fluent Bit Authors
 *  Copyright (C) 2015-2018 Treasure Data Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#include <stdlib.h>
#include <string.h>

#include <fluent-bit/flb_macros.h>
#include <fluent-bit/flb_log.h>
// #include <fluent-bit/flb_info.h>
#include <fluent-bit/flb_mem.h>
// #include <fluent-bit/flb_sds.h>
#include <fluent-bit/flb_error.h>
// #include <fluent-bit/flb_utils.h>
#include <fluent-bit/flb_sds.h>
// #include <fluent-bit/flb_time.h>
// #include <fluent-bit/flb_pack.h>
// #include <fluent-bit/flb_unescape.h>
#include <fluent-bit/flb_avro.h>

// #include <msgpack.h>

// #include <avro.h>

static inline int do_avro(bool call, const char *msg) {
    if (call) {
            fprintf(stderr, "%s:\n  %s\n", msg, avro_strerror());
            return FLB_FALSE;
    }
    return FLB_TRUE;
}

avro_value_iface_t  *flb_avro_init(avro_value_t *aobject, char *json, size_t json_len, avro_schema_t *aschema)
{
    // avro_schema_t aschema;

    fprintf(stderr, "before:error:%s:json len:%zu:\n", avro_strerror(), json_len);

    // if (aobject == NULL) {
    //     fprintf(stderr, "avro init called but avro object is null\n");
	// 	return FLB_ERROR;
    // }

	if (avro_schema_from_json_length(json, json_len, aschema)) {
		fprintf(stderr, "Unable to parse aobject schema:%s:error:%s:\n", json, avro_strerror());
		return NULL;
	}

   avro_value_iface_t  *aclass = avro_generic_class_from_schema(*aschema);

    if(aclass == NULL) {
        fprintf(stderr, "Unable to instantiate class from schema:%s:\n", avro_strerror());
		return NULL;
    }

    if(avro_generic_value_new(aclass, aobject) != 0) {
 		fprintf(stderr, "Unable to allocate new avro value:%s:\n", avro_strerror());
		return NULL;
    }

    // avro_free(aclass, 0);

    return aclass;
}

// typedef struct pool
// {
//   char * next;
//   char * end;
// } POOL;

// POOL * pool_create( size_t size ) {
//     POOL * p = (POOL*)malloc( size + sizeof(POOL) );
//     p->next = (char*)&p[1];
//     p->end = p->next + size;
//     return p;
// }

// void pool_destroy( POOL *p ) {
//     free(p);
// }

// size_t pool_available( POOL *p ) {
//     return p->end - p->next;
// }

// void * pool_alloc( POOL *p, size_t size ) {
//     if( pool_available(p) < size ) return NULL;
//     void *mem = (void*)p->next;
//     p->next += size;
//     return mem;
// }

/*
 * void *ud points to a POOL
 */
void *
flb_avro_allocator(void *ud, void *ptr, size_t osize, size_t nsize)
{
    // POOL *pool = (POOL *)ud;

        fprintf(stderr, "alloc(%p, %" PRIsz ", %" PRIsz ") => ", ptr, osize, nsize);
        if (nsize == 0) {
            fprintf(stderr, "don't free anything. do that later in the caller\n");
            flb_free(ptr);
            // pool_destroy(pool);
            return NULL;
        } else {
            fprintf(stderr, "realloc:ud:%p:\n", ud);
            return flb_realloc_z(ptr, osize, nsize);
            // ud = flb_realloc_z(ptr, osize, nsize);
            // return pool_alloc(pool, nsize);

        }
}
void *
flb_avro_allocator22(void *ud, void *ptr, size_t osize, size_t nsize)
{
        // AVRO_UNUSED(ud);
        // AVRO_UNUSED(osize);

#if SHOW_ALLOCATIONS
        fprintf(stderr, "alloc(%p, %" PRIsz ", %" PRIsz ") => ", ptr, osize, nsize);
#endif

        if (nsize == 0) {
                size_t  *size = ((size_t *) ptr) - 1;
                if (osize != *size) {
                        fprintf(stderr,
#if SHOW_ALLOCATIONS
                                "ERROR!\n"
#endif
                                "Error freeing %p:\n"
                                "Size passed to avro_free (%" PRIsz ") "
                                "doesn't match size passed to "
                                "avro_malloc (%" PRIsz ")\n",
                                ptr, osize, *size);
                        exit(EXIT_FAILURE);
                }
                free(size);
#if SHOW_ALLOCATIONS
                fprintf(stderr, "NULL\n");
#endif
                return NULL;
        } else {
                size_t  real_size = nsize + sizeof(size_t);
                size_t  *old_size = ptr? ((size_t *) ptr)-1: NULL;
                size_t  *size = (size_t *) realloc(old_size, real_size);
                *size = nsize;
#if SHOW_ALLOCATIONS
                fprintf(stderr, "%p\n", (size+1));
#endif
                return (size + 1);
        }
}

int msgpack2avro(avro_value_t *val, msgpack_object *o)
{
    int ret = FLB_FALSE;

    switch(o->type) {
    case MSGPACK_OBJECT_NIL:
        fprintf(stderr, "DEBUG: got a nil:\n");
        ret = do_avro(avro_value_set_null(val), "failed on nil");
        break;

    case MSGPACK_OBJECT_BOOLEAN:
        fprintf(stderr, "DEBUG: got a bool:%s:\n", (o->via.boolean ? "true" : "false"));
        ret = do_avro(avro_value_set_boolean(val, o->via.boolean), "failed on bool");
        break;

    case MSGPACK_OBJECT_POSITIVE_INTEGER:
        //  for reference src/objectc.c +/msgpack_pack_object
#if defined(PRIu64)
        // msgpack_pack_fix_uint64
        fprintf(stderr, "DEBUG: got a posint: %" PRIu64 "\n", o->via.u64);
        ret = do_avro(avro_value_set_int(val, o->via.u64), "failed on posint");
#else
        if (o.via.u64 > ULONG_MAX)
            fprintf(stderr, "WARNING: over 4294967295");
        else
            fprintf(stderr, "DEBUG: got a posint: %lu\n", (unsigned long)o->via.u64);
            ret = do_avro(avro_value_set_int(val, o->via.u64), "failed on posint");
#endif

        break;

    case MSGPACK_OBJECT_NEGATIVE_INTEGER:
#if defined(PRIi64)
        fprintf(stderr, "DEBUG: got a negint: %" PRIi64 "\n", o->via.i64);
        ret = do_avro(avro_value_set_int(val, o->via.i64), "failed on negint");
#else
        if (o->via.i64 > LONG_MAX)
            fprintf(stderr, "WARNING: over +2147483647");
        else if (o->via.i64 < LONG_MIN)
            fprintf(stderr, "WARNING: under -2147483648");
        else
            fprintf(stderr, "DEBUG: got a negint: %ld\n", (signed long)o->via.i64);
            ret = do_avro(avro_value_set_int(val, o->via.i64), "failed on negint");

#endif
        break;

    case MSGPACK_OBJECT_FLOAT32:
    case MSGPACK_OBJECT_FLOAT64:
        fprintf(stderr, "DEBUG: got a float: %f\n", o->via.f64);
        ret = do_avro(avro_value_set_float(val, o->via.f64), "failed on float");
        break;

    case MSGPACK_OBJECT_STR: 
        {
            fprintf(stderr, "DEBUG: got a string: \"");
            fwrite(o->via.str.ptr, o->via.str.size, 1, stderr);
            fprintf(stderr, "\"\n");
            flb_sds_t key = flb_sds_create_len(o->via.str.ptr, o->via.str.size);
            ret = do_avro(avro_value_set_string(val, key), "failed on string");
            flb_sds_destroy(key);
        }
        break;

    case MSGPACK_OBJECT_BIN:
        fprintf(stderr, "DEBUG: got a binary\n");
        ret = do_avro(avro_value_set_bytes(val, o->via.bin.ptr, o->via.bin.size), "failed on bin");
        break;

    case MSGPACK_OBJECT_EXT:
#if defined(PRIi8)
        fprintf(stderr, "DEBUG: got an ext: %" PRIi8 ")", o->via.ext.type);
#else
        fprintf(stderr, "DEBUG: got an ext: %d)", (int)o->via.ext.type);
#endif
        ret = do_avro(avro_value_set_bytes(val, o->via.bin.ptr, o->via.bin.size), "failed on ext");
        break;

    case MSGPACK_OBJECT_ARRAY: 
        {

            fprintf(stderr, "DEBUG: got a array:size:%u:\n", o->via.array.size);
            if(o->via.array.size != 0) {
                msgpack_object* p = o->via.array.ptr;
                msgpack_object* const pend = o->via.array.ptr + o->via.array.size;
                int i = 0;
                for(; p < pend; ++p) {
                    avro_value_t  element;
                    fprintf(stderr, "DEBUG: processing array\n");
                    if (
                        !do_avro(avro_value_append(val, &element, NULL), "Cannot append to array") ||
                        !do_avro(avro_value_get_by_index(val, i++, &element, NULL), "Cannot get element")) {
                        goto msg2avro_end;
                    }
                    ret = flb_msgpack_to_avro(&element, p);
                    // avro_value_decref(&element);
                }
            }
        } 
        break;

    case MSGPACK_OBJECT_MAP:
        fprintf(stderr, "DEBUG: got a map\n");
        if(o->via.map.size != 0) {
            msgpack_object_kv* p = o->via.map.ptr;
            msgpack_object_kv* const pend = o->via.map.ptr + o->via.map.size;
            size_t i = 0;

            for(; p < pend; ++p) {
                // avro_value_t  *element = flb_calloc(1, sizeof(avro_value_t));
                avro_value_t  element;

                // size_t  new_index;
                // int  is_new = 0;

                flb_sds_t key = flb_sds_create_len(p->key.via.str.ptr, p->key.via.str.size);
                fprintf(stderr, "DEBUG: got key:%s:\n", key);

                // this does not always return 0 for succcess
                if (val == NULL) {
                    fprintf(stderr, "got a null val\n");
                    continue;
                }
                if (avro_value_add(val, key, &element, NULL, NULL) != 0) {
                    fprintf(stderr, "avro_value_add:key:%s:avro error:%s:\n", key, avro_strerror());
                }

                fprintf(stderr, "after first\n");

                // avro_value_add(val, key, &element, &new_index, &is_new);

                if (!do_avro(avro_value_get_by_index(val, i++, &element, NULL), "Cannot get field")) {
                    flb_sds_destroy(key);
                    goto msg2avro_end;
                }
                ret = flb_msgpack_to_avro(&element, &p->val);
                // avro_value_decref(&element);

                flb_sds_destroy(key);

                // avro_value_reset(&element);

            }
        }
        break;

    default:
        // FIXME
#if defined(PRIu64)
        fprintf(stderr, "WARNING: #<UNKNOWN %i %" PRIu64 ">\n", o->type, o->via.u64);
#else
        if (o.via.u64 > ULONG_MAX)
            fprintf(out, "WARNING: #<UNKNOWN %i over 4294967295>", o->type);
        else
            fprintf(out, "WARNING: #<UNKNOWN %i %lu>", o.type, (unsigned long)o->via.u64);
#endif
        // noop
        break;
    }

msg2avro_end:
    return ret;

}

/**
 *  convert msgpack to an avro object.
 *  it will fill the avro value with whatever comes in from the msgpack
 *  instantiate the avro value properly according to avro-c style
 *    - avro_schema_from_json_literal
 *    - avro_generic_class_from_schema
 *    - avro_generic_value_new 
 * 
 *  or use flb_avro_init for the so do the initialization
 * 
 *  refer to avro docs
 *     http://avro.apache.org/docs/current/api/c/index.html#_avro_values
 *
 *  @param  val       An initialized avro value, an instiatied instance of the class to be unpacked.
 *  @param  data      The msgpack_unpacked data.
 *  @return success   FLB_TRUE on success
 */
int flb_msgpack_to_avro(avro_value_t *val, msgpack_object *o)
{
    int ret = -1;

    if (val == NULL || o == NULL) {
        return -1;
    }

    ret = msgpack2avro(val, o);

    return ret;
}

flb_sds_t flb_msgpack_raw_to_avro_sds(const void *in_buf, size_t in_size, const char hexstringxx[], char* schema_json)
{
    size_t off = 0;
    size_t out_size;
    msgpack_unpacked result;
    msgpack_object *root;
    flb_sds_t out_buf;
    avro_value_t avalue;
    avro_writer_t awriter;
    size_t schema_json_len = strlen(schema_json);

    // out_size = in_size * 1.5;
    // out_buf = flb_sds_create_size(out_size);
    // if (!out_buf) {
        // flb_errno();
        // return NULL;
    // }

    // char buf2[512*100];          /* Message value temporary buffer */
    void *buf2;

#define EDEDE 512 * 1000
    buf2 = flb_malloc(EDEDE);
    if (!buf2) {
        flb_errno();
        return NULL;
    }

    // avro_set_allocator(flb_avro_allocator, NULL);
    // avro_value_iface_t  *aclass;
    avro_schema_t aschema;

    avro_value_iface_t *aclass = flb_avro_init(&avalue, schema_json, schema_json_len, &aschema);
    if (aclass == NULL) {
        fprintf(stderr,  "Failed avro init\n");
        return NULL;
    }

    msgpack_unpacked_init(&result);
    msgpack_unpack_next(&result, in_buf, in_size, &off);
    root = &result.data;

    // create the avro object
    // then serialize it into a buffer for the downstream
    if (flb_msgpack_to_avro(&avalue, root) != FLB_TRUE) {
        flb_errno();
        fprintf(stderr,  "Failed msgpack to avro\n");
        return NULL;
    }

    // while (1) {
    //     ret = flb_msgpack_to_avro(&avalue, root);
    //     if (ret <= 0) {
    //         tmp_buf = flb_sds_increase(out_buf, 256);
    //         if (tmp_buf) {
    //             out_buf = tmp_buf;
    //             out_size += 256;
    //         }
    //         else {
    //             flb_errno();
    //             flb_sds_destroy(out_buf);
    //             msgpack_unpacked_destroy(&result);
    //             return NULL;
    //         }
    //     }
    //     else {
    //         break;
    //     }
    // }

    // ./avro/src/avro/io.h:avro_writer_t avro_writer_file(FILE * fp);
    // av = avro_writer_file(stdout);
    // ./avro/src/avro/io.h:avro_writer_t avro_writer_memory(const char *buf, int64_t len);
    fprintf(stderr,  "before avro_writer_memory\n");
    //  write one bye of \0
    //  write 16 bytes schemaid where the schemaid is hex for the written bytes
    // av = avro_writer_memory(buf2, sizeof(buf2));
    
    awriter = avro_writer_memory(buf2, EDEDE);
    if (awriter == NULL) {
            fprintf(stderr,  "Unable to init avro writer\n");
            return NULL;
    }

    // write the magic byte
    // int avro_write(avro_writer_t writer, void *buf, int64_t len)
    int rval;
    // const char dddd[] = {'\0'};
    rval = avro_write(awriter, "\0", 1);
    // rval = avro_write(awriter, dddd, 1);
    // avro_write(awriter, "\0", 1);
    if (rval != 0) {
            fprintf(stderr,  "Unable to write magic byte\n");
            return NULL;
    }

    // write the schemaid
    const char hexstring[] = "34530c546683be367ddde8b7734b8af3", *pos = hexstring;
    // const char *pos = hexstring;
    unsigned char val[16];
    size_t count;
    for (count = 0; count < sizeof val/sizeof *val; count++) {
            sscanf(pos, "%2hhx", &val[count]);
            pos += 2;
    }
    
    // write it into a buffer which can be passed to librdkafka
    rval = avro_write(awriter, val, 16);
    if (rval != 0) {
            fprintf(stderr,  "Unable to write schemaid\n");
            return NULL;
    }

	if (avro_value_write(awriter, &avalue)) {
		fprintf(stderr,
			"Unable to write avro value to memory buffer\nMessage: %s\n", avro_strerror());
		return NULL;
	}
    // rval = avro_write(awriter, avalue, avro_writer_tell(avalue));
    // if (rval != 0) {
            // fprintf(stderr,  "Unable to write schemaid\n");
            // return NULL;
    // }
    // null terminate it
    // since the other use-cases are strings
    // its safer to pretend this is a string
    // rval = avro_write(awriter, "\0", 1);
    // if (rval != 0) {
    //         fprintf(stderr,  "Unable to null terminate the buffer\n");
    //         return NULL;
    // }

    // fprintf(stderr,  "before add_person\n");

    // add_person(awriter, "Super", "Man", "123456", 31);

    fprintf(stderr,  "before avro_writer_flush\n");

    int64_t bytes_written = avro_writer_tell(awriter);

    avro_writer_flush(awriter);

    // by here the entire object should be fully serialized into the sds buffer
    msgpack_unpacked_destroy(&result);
    avro_writer_free(awriter);
    // avro_generic_value_free(&avalue);
    // avro_free()
    // avro_free(&avalue, 0);
    avro_free(aclass, 0);
 
    fprintf(stderr,  "after memory free\n");

    // flb_sds_len_set(out_buf, ret);

    // strictly pretending its a string here
    // since it was strictly null terminated above
    // out_buf = flb_sds_create(buf2);
    out_buf = flb_sds_create_len(buf2, bytes_written + 1);

    flb_free(buf2);

    return out_buf;

}