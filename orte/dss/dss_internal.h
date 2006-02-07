/* -*- C -*-
 *
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 */
#ifndef ORTE_DSS_INTERNAL_H_
#define ORTE_DSS_INTERNAL_H_

#include "orte_config.h"

#include "include/orte_constants.h"
#include "class/orte_pointer_array.h"

#include "dss/dss.h"

#if HAVE_STRING_H
#    if !defined(STDC_HEADERS) && HAVE_MEMORY_H
#        include <memory.h>
#    endif
#    include <string.h>
#endif

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

/*
 * DEFINE THE DEFAULT PAGE SIZE FOR THE DSS BUFFERS - IN KILOBYTES
 */
#define ORTE_DSS_DEFAULT_PAGE_SIZE  1

/**
 * Internal struct used for holding registered dss functions
 */
struct orte_dss_type_info_t {
    opal_object_t super;
    /* type identifier */
    orte_data_type_t odti_type;
    /** Debugging string name */
    char *odti_name;
    /** Pack function */
    orte_dss_pack_fn_t odti_pack_fn;
    /** Unpack function */
    orte_dss_unpack_fn_t odti_unpack_fn;
    /** copy function */
    orte_dss_copy_fn_t odti_copy_fn;
    /** compare function */
    orte_dss_compare_fn_t odti_compare_fn;
    /** size function */
    orte_dss_size_fn_t odti_size_fn;
    /** print function */
    orte_dss_print_fn_t odti_print_fn;
    /** Release function */
    orte_dss_release_fn_t odti_release_fn;
    /** flag to indicate structured data */
    bool odti_structured;
};
/**
 * Convenience typedef
 */
typedef struct orte_dss_type_info_t orte_dss_type_info_t;
OBJ_CLASS_DECLARATION(orte_dss_type_info_t);

/*
 * globals needed within dss
 */
extern bool orte_dss_initialized;
extern bool orte_dss_debug;
extern int orte_dss_verbose;
extern int orte_dss_page_size;
extern orte_pointer_array_t *orte_dss_types;
extern orte_data_type_t orte_dss_num_reg_types;

    /*
     * Implementations of API functions
     */
    int orte_dss_set(orte_data_value_t *value, void *new_value, orte_data_type_t type);

    int orte_dss_get(void **data, orte_data_value_t *value, orte_data_type_t type);

    int orte_dss_arith(orte_data_value_t *value, void *operand, orte_dss_arith_op_t operation, orte_data_type_t type);

    int orte_dss_increment(orte_data_value_t *value);

    int orte_dss_decrement(orte_data_value_t *value);

    int orte_dss_pack(orte_buffer_t *buffer, void *src,
                      size_t num_vals,
                      orte_data_type_t type);
    int orte_dss_unpack(orte_buffer_t *buffer, void *dest,
                        size_t *max_num_vals,
                        orte_data_type_t type);

    int orte_dss_copy(void **dest, void *src, orte_data_type_t type);

    int orte_dss_compare(void *value1, void *value2,
                         orte_data_type_t type);

    int orte_dss_print(char **output, char *prefix, void *src, orte_data_type_t type);

    int orte_dss_size(size_t *size, void *src, orte_data_type_t type);

    int orte_dss_peek(orte_buffer_t *buffer, orte_data_type_t *type,
                      size_t *number);

    int orte_dss_peek_type(orte_buffer_t *buffer, orte_data_type_t *type);

    int orte_dss_unload(orte_buffer_t *buffer, void **payload,
                        size_t *bytes_used);
    int orte_dss_load(orte_buffer_t *buffer, void *payload, size_t bytes_used);

    int orte_dss_register(orte_dss_pack_fn_t pack_fn,
                          orte_dss_unpack_fn_t unpack_fn,
                          orte_dss_copy_fn_t copy_fn,
                          orte_dss_compare_fn_t compare_fn,
                          orte_dss_size_fn_t size_fn,
                          orte_dss_print_fn_t print_fn,
                          orte_dss_release_fn_t release_fn,
                          bool structured,
                          const char *name, orte_data_type_t *type);

    void orte_dss_release(orte_data_value_t *value);

    char *orte_dss_lookup_data_type(orte_data_type_t type);

    void orte_dss_dump_data_types(int output);

    /*
     * Non-API functions
     */
    int orte_dss_pack_buffer(orte_buffer_t *buffer, void *src, size_t num_vals,
                  orte_data_type_t type);

    int orte_dss_unpack_buffer(orte_buffer_t *buffer, void *dst, size_t *num_vals,
                    orte_data_type_t type);

    /*
     * Internal pack functions
     */

    int orte_dss_pack_null(orte_buffer_t *buffer, void *src,
                           size_t num_vals, orte_data_type_t type);
    int orte_dss_pack_byte(orte_buffer_t *buffer, void *src,
                           size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_bool(orte_buffer_t *buffer, void *src,
                           size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_int(orte_buffer_t *buffer, void *src,
                          size_t num_vals, orte_data_type_t type);
    int orte_dss_pack_int16(orte_buffer_t *buffer, void *src,
                            size_t num_vals, orte_data_type_t type);
    int orte_dss_pack_int32(orte_buffer_t *buffer, void *src,
                            size_t num_vals, orte_data_type_t type);
    int orte_dss_pack_int64(orte_buffer_t *buffer, void *src,
                            size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_sizet(orte_buffer_t *buffer, void *src,
                            size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_pid(orte_buffer_t *buffer, void *src,
                          size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_string(orte_buffer_t *buffer, void *src,
                             size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_data_type(orte_buffer_t *buffer, void *src,
                           size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_daemon_cmd(orte_buffer_t *buffer, void *src,
                                 size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_data_value(orte_buffer_t *buffer, void *src,
                           size_t num_vals, orte_data_type_t type);

    int orte_dss_pack_byte_object(orte_buffer_t *buffer, void *src,
                           size_t num_vals, orte_data_type_t type);

    /*
     * Internal unpack functions
     */

    int orte_dss_unpack_null(orte_buffer_t *buffer, void *dest,
                             size_t *num_vals, orte_data_type_t type);
    int orte_dss_unpack_byte(orte_buffer_t *buffer, void *dest,
                             size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_bool(orte_buffer_t *buffer, void *dest,
                             size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_int(orte_buffer_t *buffer, void *dest,
                            size_t *num_vals, orte_data_type_t type);
    int orte_dss_unpack_int16(orte_buffer_t *buffer, void *dest,
                              size_t *num_vals, orte_data_type_t type);
    int orte_dss_unpack_int32(orte_buffer_t *buffer, void *dest,
                              size_t *num_vals, orte_data_type_t type);
    int orte_dss_unpack_int64(orte_buffer_t *buffer, void *dest,
                              size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_sizet(orte_buffer_t *buffer, void *dest,
                              size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_pid(orte_buffer_t *buffer, void *dest,
                            size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_string(orte_buffer_t *buffer, void *dest,
                               size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_data_type(orte_buffer_t *buffer, void *dest,
                             size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_daemon_cmd(orte_buffer_t *buffer, void *dest,
                                   size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_data_value(orte_buffer_t *buffer, void *dest,
                             size_t *num_vals, orte_data_type_t type);

    int orte_dss_unpack_byte_object(orte_buffer_t *buffer, void *dest,
                             size_t *num_vals, orte_data_type_t type);

    /*
     * Internal copy functions
     */

    int orte_dss_std_copy(void **dest, void *src, orte_data_type_t type);

    int orte_dss_copy_null(char **dest, char *src, orte_data_type_t type);

    int orte_dss_copy_string(char **dest, char *src, orte_data_type_t type);

    int orte_dss_copy_byte_object(orte_byte_object_t **dest, orte_byte_object_t *src,
                                  orte_data_type_t type);

    int orte_dss_copy_data_value(orte_data_value_t **dest, orte_data_value_t *src,
                                  orte_data_type_t type);
    /*
     * Internal compare functions
     */

    int orte_dss_compare_bool(bool *value1, bool *value2, orte_data_type_t type);

    int orte_dss_compare_int(int *value1, int *value2, orte_data_type_t type);
    int orte_dss_compare_uint(uint *value1, uint *value2, orte_data_type_t type);

    int orte_dss_compare_size(size_t *value1, size_t *value2, orte_data_type_t type);

    int orte_dss_compare_pid(pid_t *value1, pid_t *value2, orte_data_type_t type);

    int orte_dss_compare_byte(char *value1, char *value2, orte_data_type_t type);
    int orte_dss_compare_char(char *value1, char *value2, orte_data_type_t type);
    int orte_dss_compare_int8(int8_t *value1, int8_t *value2, orte_data_type_t type);
    int orte_dss_compare_uint8(uint8_t *value1, uint8_t *value2, orte_data_type_t type);

    int orte_dss_compare_int16(int16_t *value1, int16_t *value2, orte_data_type_t type);
    int orte_dss_compare_uint16(uint16_t *value1, uint16_t *value2, orte_data_type_t type);

    int orte_dss_compare_int32(int32_t *value1, int32_t *value2, orte_data_type_t type);
    int orte_dss_compare_uint32(uint32_t *value1, uint32_t *value2, orte_data_type_t type);

    int orte_dss_compare_int64(int64_t *value1, int64_t *value2, orte_data_type_t type);
    int orte_dss_compare_uint64(uint64_t *value1, uint64_t *value2, orte_data_type_t type);

    int orte_dss_compare_null(char *value1, char *value2, orte_data_type_t type);

    int orte_dss_compare_string(char *value1, char *value2, orte_data_type_t type);

    int orte_dss_compare_dt(orte_data_type_t *value1, orte_data_type_t *value2, orte_data_type_t type);

    int orte_dss_compare_daemon_cmd(orte_daemon_cmd_flag_t *value1, orte_daemon_cmd_flag_t *value2, orte_data_type_t type);

    int orte_dss_compare_data_value(orte_data_value_t *value1, orte_data_value_t *value2, orte_data_type_t type);

    int orte_dss_compare_byte_object(orte_byte_object_t *value1, orte_byte_object_t *value2, orte_data_type_t type);

    /*
    * Internal size functions
    */
    int orte_dss_std_size(size_t *size, void *src, orte_data_type_t type);

    int orte_dss_size_string(size_t *size, char *src, orte_data_type_t type);

    int orte_dss_size_data_value(size_t *size, orte_data_value_t *src, orte_data_type_t type);

    int orte_dss_size_byte_object(size_t *size, orte_byte_object_t *src, orte_data_type_t type);

    /*
    * Internal print functions
    */
    int orte_dss_print_byte(char **output, char *prefix, uint8_t *src, orte_data_type_t type);

    int orte_dss_print_string(char **output, char *prefix, char *src, orte_data_type_t type);

    int orte_dss_print_size(char **output, char *prefix, size_t *src, orte_data_type_t type);
    int orte_dss_print_pid(char **output, char *prefix, pid_t *src, orte_data_type_t type);
    int orte_dss_print_bool(char **output, char *prefix, bool *src, orte_data_type_t type);
    int orte_dss_print_int(char **output, char *prefix, int *src, orte_data_type_t type);
    int orte_dss_print_uint(char **output, char *prefix, int *src, orte_data_type_t type);
    int orte_dss_print_uint8(char **output, char *prefix, uint8_t *src, orte_data_type_t type);
    int orte_dss_print_uint16(char **output, char *prefix, uint16_t *src, orte_data_type_t type);
    int orte_dss_print_uint32(char **output, char *prefix, uint32_t *src, orte_data_type_t type);
    int orte_dss_print_int8(char **output, char *prefix, int8_t *src, orte_data_type_t type);
    int orte_dss_print_int16(char **output, char *prefix, int16_t *src, orte_data_type_t type);
    int orte_dss_print_int32(char **output, char *prefix, int32_t *src, orte_data_type_t type);
#ifdef HAVE_INT64_T
    int orte_dss_print_uint64(char **output, char *prefix, uint64_t *src, orte_data_type_t type);
    int orte_dss_print_int64(char **output, char *prefix, int64_t *src, orte_data_type_t type);
#else
    int orte_dss_print_uint64(char **output, char *prefix, void *src, orte_data_type_t type);
    int orte_dss_print_int64(char **output, char *prefix, void *src, orte_data_type_t type);
#endif
    int orte_dss_print_null(char **output, char *prefix, void *src, orte_data_type_t type);
    int orte_dss_print_data_type(char **output, char *prefix, orte_data_type_t *src, orte_data_type_t type);
    int orte_dss_print_daemon_cmd(char **output, char *prefix, orte_daemon_cmd_flag_t *src, orte_data_type_t type);
    int orte_dss_print_data_value(char **output, char *prefix, orte_data_value_t *src, orte_data_type_t type);
    int orte_dss_print_byte_object(char **output, char *prefix, orte_byte_object_t *src, orte_data_type_t type);


    /*
    * Internal release functions
    */
    void orte_dss_std_release(orte_data_value_t *value);

    void orte_dss_std_obj_release(orte_data_value_t *value);

    void orte_dss_release_byte_object(orte_data_value_t *value);

    /*
     * Internal helper functions
     */

    char* orte_dss_buffer_extend(orte_buffer_t *bptr, size_t bytes_to_add);

    bool orte_dss_too_small(orte_buffer_t *buffer, size_t bytes_reqd);

    int orte_dss_store_data_type(orte_buffer_t *buffer, orte_data_type_t type);

    int orte_dss_get_data_type(orte_buffer_t *buffer, orte_data_type_t *type);

    orte_dss_type_info_t* orte_dss_find_type(orte_data_type_t type);

#if defined(c_plusplus) || defined(__cplusplus)
}
#endif
#endif
