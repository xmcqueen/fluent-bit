/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <string.h>
#include <fluent-bit/flb_mem.h>
#include <fluent-bit/flb_avro.h>

#include "flb_tests_internal.h"

/* AVRO iteration tests */
#define AVRO_SINGLE_MAP1 FLB_TESTS_DATA_PATH "/data/avro/json_single_map_001.json"
#define AVRO_SINGLE_MAPX FLB_TESTS_DATA_PATH "/data/avro/json_single_map_00x.json"
#define AVRO_REC_REC_MAP FLB_TESTS_DATA_PATH "/data/avro/map_in_record_in_record.json"
#define AVRO_LIST_REC_REC_MAP FLB_TESTS_DATA_PATH "/data/avro/map_in_record_in_record_in_list.json"
#define AVRO_TIGHT_SCHEMA FLB_TESTS_DATA_PATH "/data/avro/tight_schema.json"
// #define AVRO_MULTILINE_JSON FLB_TESTS_DATA_PATH "/data/avro/multiline.json"
#define AVRO_MULTILINE_JSON FLB_TESTS_DATA_PATH "/data/avro/live-sample.json"

const char  JSON_SINGLE_MAP_001_SCHEMA[] =
"{\"type\":\"record\",\
  \"name\":\"Map001\",\
  \"fields\":[\
     {\"name\": \"key001\", \"type\": \"int\"},\
     {\"name\": \"key002\", \"type\": \"float\"},\
     {\"name\": \"key003\", \"type\": \"string\"},\
     {\"name\": \"key004\", \"type\":\
        {\"type\": \"array\", \"items\":\
             {\"type\": \"map\",\"values\": \"int\"}}}]}";

const char  JSON_SINGLE_MAP_00X_SCHEMA[] =
"{\"type\":\"record\",\
  \"name\":\"Map001\",\
  \"fields\":[\
     {\"name\": \"key001\", \"type\": \"int\"},\
     {\"name\": \"key002\", \"type\": \"float\"},\
     {\"name\": \"key003\", \"type\": \"string\"},\
     {\"name\": \"key004\", \"type\":\
        {\"type\": \"map\",\"values\": \"string\"}}]}";

const char  JSON_REC_REC_MAP_SCHEMA[] =
"{\"type\":\"record\",\
  \"name\":\"RecRecMap\",\
  \"fields\":[\
     {\"name\": \"log\", \"type\": \"string\"},\
     {\"name\": \"kubernetes\", \"type\":\
        {\"type\":\"record\",\
         \"name\":\"Map001\",\
         \"fields\":[\
            {\"name\": \"key001\", \"type\": \"int\"},\
            {\"name\": \"key002\", \"type\": \"float\"},\
            {\"name\": \"key003\", \"type\": \"string\"},\
            {\"name\": \"key004\", \"type\":\
               {\"type\": \"map\",\"values\": \"string\"}}]}}]}";

const char  JSON_LIST_REC_REC_MAP_SCHEMA[] =
"{\"name\": \"qavrov2\",\
\"type\": \"array\",\
\"namespace\": \"com.x.kubernetes.docker\",\
\"items\":\
{\"type\":\"record\",\
  \"name\":\"ListRecRecMap\",\
  \"fields\":[\
     {\"name\": \"log\", \"type\": \"string\"},\
     {\"name\": \"kubernetes\", \"type\":\
        {\"type\":\"record\",\
         \"name\":\"Map001\",\
         \"fields\":[\
            {\"name\": \"key001\", \"type\": \"int\"},\
            {\"name\": \"key002\", \"type\": \"float\"},\
            {\"name\": \"key003\", \"type\": \"string\"},\
            {\"name\": \"key004\", \"type\":\
               {\"type\": \"map\",\"values\": \"string\"}}]}}]}}";

const char QQQq[] = "{\"name\": \"qavrov2\",\"type\": \"array\",\"namespace\": \"com.x.kubernetes.docker\",\"items\":{\"name\":\"qavrov2_record\",\"type\":\"record\",\"fields\":[{\"name\":\"date\",\"type\":\"float\"},{\"name\":\"log\",\"type\":\"string\"},{\"name\":\"kubernetes\",\"type\":{\"name\":\"krec\",\"type\":\"record\",\"fields\":[{\"name\":\"pod_name\",\"type\":\"string\"},{\"name\":\"namespace_name\",\"type\":\"string\"},{\"name\":\"pod_id\",\"type\":\"string\"},{\"name\":\"host\",\"type\":\"string\"},{\"name\":\"container_name\",\"type\":\"string\"},{\"name\":\"docker_id\",\"type\":\"string\"},{\"name\":\"container_hash\",\"type\":\"string\"},{\"name\":\"container_image\",\"type\":\"string\"},{\"name\":\"labels\",\"type\":{\"type\":\"map\",\"values\":\"string\"}},{\"name\":\"annotations\",\"type\":{\"type\":\"map\",\"values\":\"string\"}}]}}]}}";
// const char QQQ[] = "{\"name\":\"qavrov2\",\"type\":\"array\",\"namespace\":\"com.x.kubernetes.docker\",\"items\":{\"name\":\"qavrov2_record\",\"type\":\"record\",\"fields\":[{\"name\":\"log\",\"type\":\"string\"},{\"name\":\"capture\",\"type\":\"string\"},{\"name\":\"kubernetes\",\"type\":{\"name\":\"krec\",\"type\":\"record\",\"fields\":[{\"name\":\"pod_name\",\"type\":\"string\"},{\"name\":\"namespace_name\",\"type\":\"string\"},{\"name\":\"pod_id\",\"type\":\"string\"},{\"name\":\"labels\",\"type\":{\"type\":\"map\",\"values\":\"string\"}},{\"name\":\"annotations\",\"type\":{\"type\":\"map\",\"values\":\"string\"}},{\"name\":\"host\",\"type\":\"string\"},{\"name\":\"container_name\",\"type\":\"string\"},{\"name\":\"docker_id\",\"type\":\"string\"},{\"name\":\"container_hash\",\"type\":\"string\"},{\"name\":\"container_image\",\"type\":\"string\"}]}}]}}";
const char QQQ[] = "{\"name\":\"qavrov2_record\",\"type\":\"record\",\"fields\":[{\"name\":\"log\",\"type\":\"string\"},{\"name\":\"capture\",\"type\":\"string\"},{\"name\":\"kubernetes\",\"type\":{\"name\":\"krec\",\"type\":\"record\",\"fields\":[{\"name\":\"pod_name\",\"type\":\"string\"},{\"name\":\"namespace_name\",\"type\":\"string\"},{\"name\":\"pod_id\",\"type\":\"string\"},{\"name\":\"labels\",\"type\":{\"type\":\"map\",\"values\":\"string\"}},{\"name\":\"annotations\",\"type\":{\"type\":\"map\",\"values\":\"string\"}},{\"name\":\"host\",\"type\":\"string\"},{\"name\":\"container_name\",\"type\":\"string\"},{\"name\":\"docker_id\",\"type\":\"string\"},{\"name\":\"container_hash\",\"type\":\"string\"},{\"name\":\"container_image\",\"type\":\"string\"}]}}]}";

/* Unpack msgpack per avro schema */
void test_unpack_to_avro()
{
    int root_type;
    size_t len;
    char *data;
    char *out_buf;
    size_t out_size;

    avro_value_t  aobject;

    // AVRO_POOL *ppp = avro_pool_create(1024 * 1024);
    // Memory_Pool mp;
    // mp_init(&mp, 2048, 2048);
    // mp_init(&mp, 4096, 4096);

    // avro_set_allocator(flb_avro_allocatorqqq, (void *)ppp);
    // avro_set_allocator(flb_avro_allocatorqqq, (void *)&mp);
    avro_schema_t aschema;
    avro_value_iface_t  *aclass = NULL;

    aclass = flb_avro_init(&aobject, (char *)JSON_SINGLE_MAP_001_SCHEMA, strlen(JSON_SINGLE_MAP_001_SCHEMA), &aschema);
    TEST_CHECK(aclass != NULL);

    data = mk_file_to_buffer(AVRO_SINGLE_MAP1);
    TEST_CHECK(data != NULL);

    len = strlen(data);

    TEST_CHECK(flb_pack_json(data, len, &out_buf, &out_size, &root_type) == 0);

    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);
    TEST_CHECK(msgpack_unpack_next(&msg, out_buf, out_size, NULL) == MSGPACK_UNPACK_SUCCESS);

    msgpack_object_print(stderr, msg.data);
    flb_msgpack_to_avro(&aobject, &msg.data);

    avro_value_t test_value;
    TEST_CHECK(avro_value_get_by_name(&aobject, "key001", &test_value, NULL) == 0);

    int val001 = 0;
    avro_value_get_int(&test_value, &val001);
    TEST_CHECK(val001 == 123456789);

    TEST_CHECK(avro_value_get_by_name(&aobject, "key002", &test_value, NULL) == 0);

    float val002 = 0.0f;
    // for some reason its rounding to this value
    float val002_actual = 0.999888f;
    avro_value_get_float(&test_value, &val002);
    char str1[80];
    char str2[80];
    sprintf(str1, "%f", val002);
    sprintf(str2, "%f", val002_actual);
    fprintf(stderr, "val002:%s:\n", str1);
    fprintf(stderr, "val002_actual:%s:\n", str2);
    TEST_CHECK((strcmp(str1, str2) == 0));

    TEST_CHECK(avro_value_get_by_name(&aobject, "key003", &test_value, NULL) == 0);
    char *val003 = NULL;
    size_t val003_size = 0;
    avro_value_get_string(&test_value, &val003, &val003_size);
    fprintf(stderr, "val003_size:%zu:\n", val003_size);

    TEST_CHECK((strcmp(val003, "abcdefghijk") == 0));
    TEST_CHECK(val003_size == 11);

    TEST_CHECK(avro_value_get_by_name(&aobject, "key004", &test_value, NULL) == 0);

    size_t asize = 0;
    avro_value_get_size(&test_value, &asize);
    fprintf(stderr, "asize:%zu:\n", asize);

    TEST_CHECK(asize == 2);

    // check the first map
    avro_value_t k8sRecord;
    TEST_CHECK(avro_value_get_by_index(&test_value, 0, &k8sRecord, NULL) == 0);

    size_t msize = 0;
    avro_value_get_size(&k8sRecord, &msize);
    fprintf(stderr, "msize:%zu:\n", msize);

    TEST_CHECK(msize == 2);

    avro_value_t obj_test;
    const char  *actual_key = NULL;
    int  actual = 0;

    // check the first item in the map
    TEST_CHECK(avro_value_get_by_index(&k8sRecord, 0, &obj_test, &actual_key) == 0);
        fprintf(stderr, "actual_key:%s:\n", actual_key);

    TEST_CHECK(strcmp(actual_key, "a") == 0);
    TEST_CHECK(avro_value_get_int(&obj_test, &actual) == 0);
    TEST_CHECK(actual == 1);

    // check the second item in the map
    TEST_CHECK(avro_value_get_by_index(&k8sRecord, 1, &obj_test, &actual_key) == 0);
            fprintf(stderr, "actual_key:%s:\n", actual_key);

    TEST_CHECK(strcmp(actual_key, "b") == 0);
    TEST_CHECK(avro_value_get_int(&obj_test, &actual) == 0);
    TEST_CHECK(actual == 2);

    // check the second map
    TEST_CHECK(avro_value_get_by_index(&test_value, 1, &k8sRecord, NULL) == 0);

    avro_value_get_size(&k8sRecord, &msize);
    fprintf(stderr, "msize:%zu:\n", msize);

    TEST_CHECK(msize == 2);


    avro_value_decref(&aobject);
	avro_value_iface_decref(aclass);
    avro_schema_decref(aschema);
    msgpack_unpacked_destroy(&msg);
    flb_free(data);
    flb_free(out_buf);
}

void test_parse_tight_schema()
{
    int root_type;
    size_t len;
    char *out_buf;
    size_t out_size;

    avro_value_t  aobject;

    avro_value_iface_t  *aclass = NULL;
    avro_schema_t aschema;

    aclass = flb_avro_init(&aobject, (char *)QQQ, strlen(QQQ), &aschema);
    TEST_CHECK(aclass != NULL);

    // get the json
    char *data = mk_file_to_buffer(AVRO_MULTILINE_JSON);
    TEST_CHECK(data != NULL);

    len = strlen(data);

    TEST_CHECK(flb_pack_json(data, len, &out_buf, &out_size, &root_type) == 0);

    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);
    TEST_CHECK(msgpack_unpack_next(&msg, out_buf, out_size, NULL) == MSGPACK_UNPACK_SUCCESS);

    msgpack_object_print(stderr, msg.data);

    flb_msgpack_to_avro(&aobject, &msg.data);

    avro_value_t log0;
    TEST_CHECK(avro_value_get_by_name(&aobject, "log", &log0, NULL) == 0);

    size_t size1 = 0;
    const char  *log_line = NULL;
    TEST_CHECK(avro_value_get_string(&log0, &log_line, &size1) == 0);
    char *pre = "2020-08-21T15:49:48.154291375Z";
    TEST_CHECK((strncmp(pre, log_line, strlen(pre)) == 0));
    fprintf(stderr, "log_line len:%zu:\n", strlen(log_line));

    avro_value_t kubernetes0;
    TEST_CHECK(avro_value_get_by_name(&aobject, "kubernetes", &kubernetes0, NULL) == 0);

    avro_value_get_size(&kubernetes0, &size1);
    fprintf(stderr, "asize:%zu:\n", size1);
    TEST_CHECK(size1 == 10);

    avro_value_t pn;
    TEST_CHECK(avro_value_get_by_name(&kubernetes0, "pod_name", &pn, NULL) == 0);

    char *pod_name = NULL;
    size_t pod_name_size = 0;
    TEST_CHECK(avro_value_get_string(&pn, &pod_name, &pod_name_size) == 0);
    TEST_CHECK(strcmp(pod_name, "yali-bert-completion-tensorboard-6786c9c8-wj25m") == 0);

    avro_value_t nn;
    TEST_CHECK(avro_value_get_by_name(&kubernetes0, "namespace_name", &nn, NULL) == 0);

    char *namespace_name = NULL;
    size_t namespace_name_size = 0;
    TEST_CHECK(avro_value_get_string(&nn, &namespace_name, &namespace_name_size) == 0);
    TEST_CHECK(strcmp(namespace_name, "k8s-pilot") == 0);

    avro_value_t mapX;
    TEST_CHECK(avro_value_get_by_name(&kubernetes0, "annotations", &mapX, NULL) == 0);

    avro_value_get_size(&mapX, &size1);
    fprintf(stderr, "asize:%zu:\n", size1);

    TEST_CHECK(size1 == 5);

    // check the first item in the map
    avro_value_t doas;
    TEST_CHECK(avro_value_get_by_name(&mapX, "doAs", &doas, NULL) == 0);
    char *doaser = NULL;
    size_t doaser_size;
    TEST_CHECK(avro_value_get_string(&doas, &doaser, &doaser_size) == 0);
    TEST_CHECK((strcmp(doaser, "stdemb") == 0));

    avro_value_decref(&aobject);
	avro_value_iface_decref(aclass);
    avro_schema_decref(aschema);
    msgpack_unpacked_destroy(&msg);
    flb_free(data);
    flb_free(out_buf);

}

void test_unpack_to_avrox()
{
    int root_type;
    size_t len;
    char *data;
    char *out_buf;
    size_t out_size;

    avro_value_t  aobject;

    Memory_Pool mp;
    mp_init(&mp, 4096, 4096);

    avro_set_allocator(flb_avro_allocatorqqq, (void *)&mp);
    avro_schema_t aschema;

    flb_avro_init(&aobject, (char *)JSON_SINGLE_MAP_00X_SCHEMA, strlen(JSON_SINGLE_MAP_00X_SCHEMA), &aschema);

    data = mk_file_to_buffer(AVRO_SINGLE_MAPX);
    TEST_CHECK(data != NULL);

    len = strlen(data);

    TEST_CHECK(flb_pack_json(data, len, &out_buf, &out_size, &root_type) == 0);

    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);
    TEST_CHECK(msgpack_unpack_next(&msg, out_buf, out_size, NULL) == MSGPACK_UNPACK_SUCCESS);

    msgpack_object_print(stderr, msg.data);
    flb_msgpack_to_avro(&aobject, &msg.data);

    avro_value_t test_value;
    TEST_CHECK(avro_value_get_by_name(&aobject, "key001", &test_value, NULL) == 0);

    avro_value_t mapX;
    TEST_CHECK(avro_value_get_by_name(&aobject, "key004", &mapX, NULL) == 0);

    size_t asize = 0;
    avro_value_get_size(&mapX, &asize);
    fprintf(stderr, "asize:%zu:\n", asize);

    TEST_CHECK(asize == 2);

    avro_value_t obj_test;
    const char  *actual_key = NULL;

    // check the first item in the map
    TEST_CHECK(avro_value_get_by_index(&mapX, 0, &obj_test, &actual_key) == 0);
    fprintf(stderr, "actual_key:%s:\n", actual_key);

    size_t size1 = 0;
    TEST_CHECK(strcmp(actual_key, "a") == 0);
    TEST_CHECK(avro_value_get_string(&obj_test, &actual_key, &size1) == 0);
    TEST_CHECK((strcmp(actual_key, "ede") == 0));


    // check the second item in the map
    TEST_CHECK(avro_value_get_by_index(&mapX, 1, &obj_test, &actual_key) == 0);
    fprintf(stderr, "actual_key:%s:\n", actual_key);

    size_t size2 = 0;
    TEST_CHECK(strcmp(actual_key, "b") == 0);
    TEST_CHECK(avro_value_get_string(&obj_test, &actual_key, &size2) == 0);
    TEST_CHECK((strcmp(actual_key, "dkjdkj") == 0));

    mp_destroy(&mp);
    msgpack_unpacked_destroy(&msg);
    flb_free(data);
    flb_free(out_buf);
}

void test_rec_rec_map()
{
    int root_type;
    size_t len;
    char *data;
    char *out_buf;
    size_t out_size;

    avro_value_t  aobject;

    Memory_Pool mp;
    mp_init(&mp, 4096, 4096);

    avro_set_allocator(flb_avro_allocatorqqq, (void *)&mp);
    avro_schema_t aschema;

    // flb_avro_init(&aobject, (char *)JSON_REC_REC_MAP_SCHEMA, strlen(JSON_REC_REC_MAP_SCHEMA), &aschema);
    flb_avro_init(&aobject, (char *)JSON_LIST_REC_REC_MAP_SCHEMA, strlen(JSON_LIST_REC_REC_MAP_SCHEMA), &aschema);

    // data = mk_file_to_buffer(AVRO_REC_REC_MAP);
    data = mk_file_to_buffer(AVRO_LIST_REC_REC_MAP);
    TEST_CHECK(data != NULL);

    len = strlen(data);

    TEST_CHECK(flb_pack_json(data, len, &out_buf, &out_size, &root_type) == 0);

    msgpack_unpacked msg;
    msgpack_unpacked_init(&msg);
    TEST_CHECK(msgpack_unpack_next(&msg, out_buf, out_size, NULL) == MSGPACK_UNPACK_SUCCESS);

    msgpack_object_print(stderr, msg.data);
    flb_msgpack_to_avro(&aobject, &msg.data);


    size_t asize = 0;
    avro_value_get_size(&aobject, &asize);
    fprintf(stderr, "asize:%zu:\n", asize);

    TEST_CHECK(asize == 1);

    avro_value_t fbRecord;
    TEST_CHECK(avro_value_get_by_index(&aobject, 0, &fbRecord, NULL) == 0);

    avro_value_t log0;
    TEST_CHECK(avro_value_get_by_name(&fbRecord, "log", &log0, NULL) == 0);

    size_t size1 = 0;
    const char  *log_line = NULL;
    TEST_CHECK(avro_value_get_string(&log0, &log_line, &size1) == 0);
    TEST_CHECK((strcmp(log_line, "log line") == 0));

    avro_value_t kubernetes0;
    TEST_CHECK(avro_value_get_by_name(&fbRecord, "kubernetes", &kubernetes0, NULL) == 0);

    avro_value_get_size(&kubernetes0, &size1);
    fprintf(stderr, "asize:%zu:\n", size1);
    TEST_CHECK(size1 == 4);

    avro_value_t test_value;
    TEST_CHECK(avro_value_get_by_name(&kubernetes0, "key001", &test_value, NULL) == 0);

    avro_value_t mapX;
    TEST_CHECK(avro_value_get_by_name(&kubernetes0, "key004", &mapX, NULL) == 0);

    avro_value_get_size(&mapX, &size1);
    fprintf(stderr, "asize:%zu:\n", size1);

    TEST_CHECK(size1 == 2);

    avro_value_t obj_test;
    const char  *actual_key = NULL;

    // check the first item in the map
    TEST_CHECK(avro_value_get_by_index(&mapX, 0, &obj_test, &actual_key) == 0);
    fprintf(stderr, "actual_key:%s:\n", actual_key);

    TEST_CHECK(strcmp(actual_key, "a") == 0);
    TEST_CHECK(avro_value_get_string(&obj_test, &actual_key, &size1) == 0);
    TEST_CHECK((strcmp(actual_key, "ede") == 0));

    // check the second item in the map
    TEST_CHECK(avro_value_get_by_index(&mapX, 1, &obj_test, &actual_key) == 0);
    fprintf(stderr, "actual_key:%s:\n", actual_key);

    TEST_CHECK(strcmp(actual_key, "b") == 0);
    TEST_CHECK(avro_value_get_string(&obj_test, &actual_key, &size1) == 0);
    TEST_CHECK((strcmp(actual_key, "dkjdkj") == 0));

    mp_destroy(&mp);
    msgpack_unpacked_destroy(&msg);
    flb_free(data);
    flb_free(out_buf);
}

TEST_LIST = {
    /* Avro */
    { "msgpack_to_avro_basic", test_unpack_to_avro},
    { "avro_tight_schema", test_parse_tight_schema},
    // { "msgpack_to_avrox", test_unpack_to_avrox},
    // { "rec_rec_map", test_rec_rec_map},
    { 0 }
};
