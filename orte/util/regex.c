/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2006 The University of Tennessee and The University
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
 */
#include "orte_config.h"
#include "orte/types.h"
#include "orte/constants.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "opal/util/argv.h"

#include "orte/mca/errmgr/errmgr.h"
#include "orte/util/show_help.h"
#include "orte/util/name_fns.h"
#include "orte/runtime/orte_globals.h"

#include "orte/util/regex.h"

#define ORTE_MAX_NODE_PREFIX        50

static int regex_parse_node_ranges(char *base, char *ranges, char ***names);
static int regex_parse_node_range(char *base, char *range, char suffix, char ***names);


int orte_regex_extract_node_names(char *regexp, char ***names)
{
    int i, j, len, ret;
    char *base;
    char *orig;
    bool found_range = false;
    bool more_to_come = false;
    
    if (NULL == regexp) {
        *names = NULL;
        return ORTE_SUCCESS;
    }
    
    orig = base = strdup(regexp);
    if (NULL == base) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    
    OPAL_OUTPUT_VERBOSE((1, orte_debug_output,
                         "%s regex:extract:nodenames: checking nodelist: %s",
                         ORTE_NAME_PRINT(ORTE_PROC_MY_NAME),
                         regexp));
    
    do {
        /* Find the base */
        len = strlen(base);
        for (i = 0; i <= len; ++i) {
            if (base[i] == '[') {
                /* we found a range. this gets dealt with below */
                base[i] = '\0';
                found_range = true;
                break;
            }
            if (base[i] == ',') {
                /* we found a singleton node, and there are more to come */
                base[i] = '\0';
                found_range = false;
                more_to_come = true;
                break;
            }
            if (base[i] == '\0') {
                /* we found a singleton node */
                found_range = false;
                more_to_come = false;
                break;
            }
        }
        if(i == 0) {
            /* we found a special character at the beginning of the string */
            orte_show_help("help-regex.txt", "regex:special-char", true, regexp);
            free(orig);
            return ORTE_ERR_BAD_PARAM;
        }
        
        if (found_range) {
            /* If we found a range, now find the end of the range */
            for (j = i; j < len; ++j) {
                if (base[j] == ']') {
                    base[j] = '\0';
                    break;
                }
            }
            if (j >= len) {
                /* we didn't find the end of the range */
                orte_show_help("help-regex.txt", "regex:end-range-missing", true, regexp);
                free(orig);
                return ORTE_ERR_BAD_PARAM;
            }
            
            ret = regex_parse_node_ranges(base, base + i + 1, names);
            if(ORTE_SUCCESS != ret) {
                orte_show_help("help-regex.txt", "regex:bad-value", true, regexp);
                free(orig);
                return ret;
            }    
            if(base[j + 1] == ',') {
                more_to_come = true;
                base = &base[j + 2];
            } else {
                more_to_come = false;
            }
        } else {
            /* If we didn't find a range, just add the node */
            
            OPAL_OUTPUT_VERBOSE((1, orte_debug_output,
                                 "%s regex:extract:nodenames: found node: %s",
                                 ORTE_NAME_PRINT(ORTE_PROC_MY_NAME), base));
            
            if(ORTE_SUCCESS != (ret = opal_argv_append_nosize(names, base))) {
                ORTE_ERROR_LOG(ret);
                free(orig);
                return ret;
            }
            /* set base equal to the (possible) next base to look at */
            base = &base[i + 1];
        }
    } while(more_to_come);
    
    free(orig);
    
    /* All done */
    return ret;
}


/*
 * Parse one or more ranges in a set
 *
 * @param base     The base text of the node name
 * @param *ranges  A pointer to a range. This can contain multiple ranges
 *                 (i.e. "1-3,10" or "5" or "9,0100-0130,250") 
 * @param ***names An argv array to add the newly discovered nodes to
 */
static int regex_parse_node_ranges(char *base, char *ranges, char ***names)
{
    int i, len, ret;
    char *start, *orig;
    
    /* Look for commas, the separator between ranges */
    
    len = strlen(ranges);
    for (orig = start = ranges, i = 0; i < len; ++i) {
        if (',' == ranges[i]) {
            ranges[i] = '\0';
            ret = regex_parse_node_range(base, start, '\0', names);
            if (ORTE_SUCCESS != ret) {
                ORTE_ERROR_LOG(ret);
                return ret;
            }
            start = ranges + i + 1;
        }
    }
    
    /* Pick up the last range, if it exists */
    
    if (start < orig + len) {
        
        OPAL_OUTPUT_VERBOSE((1, orte_debug_output,
                             "%s regex:parse:ranges: parse range %s (2)",
                             ORTE_NAME_PRINT(ORTE_PROC_MY_NAME), start));
        
        ret = regex_parse_node_range(base, start, '\0', names);
        if (ORTE_SUCCESS != ret) {
            ORTE_ERROR_LOG(ret);
            return ret;
        }
    }
    
    /* All done */
    return ORTE_SUCCESS;
}


/*
 * Parse a single range in a set and add the full names of the nodes
 * found to the names argv
 *
 * @param base     The base text of the node name
 * @param *ranges  A pointer to a single range. (i.e. "1-3" or "5") 
 * @param ***names An argv array to add the newly discovered nodes to
 */
static int regex_parse_node_range(char *base, char *range, char suffix, char ***names)
{
    char *str, temp1[BUFSIZ];
    size_t i, j, start, end;
    size_t base_len, len, num_len;
    size_t str_start, str_end;
    size_t num_str_len;
    bool found;
    int ret;
    
    len = strlen(range);
    base_len = strlen(base);
    /* Silence compiler warnings; start and end are always assigned
     properly, below */
    start = end = 0;
    
    /* Look for the beginning of the first number */
    
    for (found = false, i = 0; i < len; ++i) {
        if (isdigit((int) range[i])) {
            if (!found) {
                str_start = i;
                start = atoi(range + i);
                found = true;
                break;
            }
        }
    }
    if (!found) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    
    /* Look for the end of the first number */
    
    for (found = false, num_str_len = 0; i < len; ++i, ++num_str_len) {
        if (!isdigit((int) range[i])) {
            break;
        }
    }
    
    /* Was there no range, just a single number? */
    
    if (i >= len) {
        str_end = len;
        end = start;
        found = true;
    }
    
    /* Nope, there was a range.  Look for the beginning of the second
     number */
    
    else {
        str_end = i - 1;
        for (; i < len; ++i) {
            if (isdigit((int) range[i])) {
                end = atoi(range + i);
                found = true;
                break;
            }
        }
    }
    if (!found) {
        ORTE_ERROR_LOG(ORTE_ERR_NOT_FOUND);
        return ORTE_ERR_NOT_FOUND;
    }
    
    /* Make strings for all values in the range */
    
    len = base_len + num_str_len + 32;
    str = (char *) malloc(len);
    if (NULL == str) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    strcpy(str, base);
    for (i = start; i <= end; ++i) {
        str[base_len] = '\0';
        snprintf(temp1, BUFSIZ - 1, "%lu", (long) i);
        
        /* Do we need zero padding? */
        
        if ((num_len = strlen(temp1)) < num_str_len) {
            for (j = base_len; j < base_len + (num_str_len - num_len); ++j) {
                str[j] = '0';
            }
            str[j] = '\0';
        }
        strcat(str, temp1);
        /* if there is a suffix, add it */
        if ('\0' != suffix) {
            num_len = strlen(str);
            str[num_len] = suffix;
            str[num_len+1] = '\0';
        }
        ret = opal_argv_append_nosize(names, str);
        if(ORTE_SUCCESS != ret) {
            ORTE_ERROR_LOG(ret);
            free(str);
            return ret;
        }
    }
    free(str);
    
    /* All done */
    return ORTE_SUCCESS;
}

/* Compute the #procs on each node given a regex of form
 * "#procs(x#nodes),#procs(x#nodes). In other words, an
 * expression of "4(x30) will be interpreted to mean four
 * procs on each of the next 30 nodes.
 */
int orte_regex_extract_ppn(int num_nodes, char *regexp, int **ppn)
{
    int *tmp;
    char *begptr, *endptr, *orig;
    int i, j, count, reps;
    
    /* init null answer */
    *ppn = NULL;
    
    tmp = (int *) malloc(sizeof(int) * num_nodes);
    if (NULL == tmp) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    memset(tmp, 0, sizeof(int) * num_nodes);
    
    orig = begptr = strdup(regexp);
    if (NULL == begptr) {
        ORTE_ERROR_LOG(ORTE_ERR_OUT_OF_RESOURCE);
        free(tmp);
        return ORTE_ERR_OUT_OF_RESOURCE;
    }
    
    j = 0;
    while (begptr) {
        count = strtol(begptr, &endptr, 10);
        if ((endptr[0] == '(') && (endptr[1] == 'x')) {
            reps = strtol((endptr+2), &endptr, 10);
            if (endptr[0] == ')') {
                endptr++;
            }
        } else {
            reps = 1;
        }
        
        for (i = 0; i < reps && j < num_nodes; i++) {
            tmp[j++] = count;
        }
        
        if (*endptr == ',') {
            begptr = endptr + 1;
        } else if (*endptr == '\0' || j >= num_nodes) {
            break;
        } else {
            orte_show_help("help-regex.txt", "regex:bad-ppn", true, regexp);
            ORTE_ERROR_LOG(ORTE_ERR_BAD_PARAM);
            free(tmp);
            free(orig);
            return ORTE_ERR_BAD_PARAM;
        }
    }
    
    free(orig);
    
    /* return values */
    *ppn = tmp;
    
    return ORTE_SUCCESS;
}

static void compute_vpids(orte_node_t *node, orte_jobid_t jobid,
                          orte_vpid_t *start_vpid, orte_vpid_t *end_vpid,
                          int32_t *ppn, orte_node_rank_t *nrank)
{
    int32_t nppn, k;
    orte_proc_t *proc, *start_proc, *end_proc;
    
    nppn = 0;
    start_proc = NULL;
    end_proc = NULL;
    for (k=0; k < node->procs->size; k++) {
        if (NULL == (proc = (orte_proc_t*)opal_pointer_array_get_item(node->procs, k)) ||
            proc->name.jobid != jobid) {
            continue;
        }
        nppn++;
        if (NULL == start_proc) {
            start_proc = proc;
        } else if (NULL == end_proc) {
            end_proc = proc;
        }
    }
    
    *ppn = nppn;
    if (NULL == start_proc) {
        /* nobody was mapped to this node */
        *start_vpid = ORTE_VPID_INVALID;
        *nrank = ORTE_NODE_RANK_INVALID;
    } else {
        *start_vpid = start_proc->name.vpid;
        *nrank = start_proc->node_rank;
    }
    if (NULL == end_proc) {
        /* could have been only one proc mapped, or none */
        *end_vpid = ORTE_VPID_INVALID;
    } else {
        *end_vpid = end_proc->name.vpid;
    }
}

static void start_sequence(orte_jobid_t jobid, orte_node_t *node,
                           orte_regex_node_t *ndreg, char suffix, int32_t nodenum)
{
    int32_t j, ppn;
    orte_vpid_t start_vpid, end_vpid;
    orte_node_rank_t nrank;
    
    opal_value_array_append_item(&ndreg->suffix, &suffix);
    opal_value_array_append_item(&ndreg->nodes, &nodenum);
    j = 0;
    opal_value_array_append_item(&ndreg->cnt, &j);
    compute_vpids(node, jobid, &start_vpid, &end_vpid, &ppn, &nrank);
    opal_value_array_append_item(&ndreg->starting_vpid, &start_vpid);
    opal_value_array_append_item(&ndreg->ppn, &ppn);
    opal_value_array_append_item(&ndreg->nrank, &nrank);
}

char* orte_regex_encode_maps(orte_job_t *jdata)
{
    orte_node_t *node;
    orte_regex_node_t *ndreg;
    int32_t nodenum, i, n;
    bool found, fullname;
    opal_list_t nodelist;
    int len;
    char prefix[ORTE_MAX_NODE_PREFIX];
    int startnum;
    opal_list_item_t *item;
    char **regexargs = NULL, *tmp;
    int32_t num_nodes, start, cnt, ppn, nppn;
    orte_vpid_t vpid_start, start_vpid, end_vpid, base;
    char *regexp = NULL;
    bool byslot;
    orte_node_rank_t node_rank, nrank;
    char suffix, sfx;
    
    /* this is only supported with regular maps - i.e., when
     * the mapping is byslot or bynode. Irregular maps cannot
     * be expressed in a regular expression
     */
    if (jdata->map->policy & ORTE_RMAPS_BYUSER) {
        return NULL;
    }
    
    /* determine the mapping policy */
    byslot = true;
    if (jdata->map->policy & ORTE_RMAPS_BYNODE) {
        byslot = false;
    }
    
    /* setup the list of nodes with same prefixes */
    OBJ_CONSTRUCT(&nodelist, opal_list_t);
    
    /* cycle through the node pool */
    for (n=0; n < orte_node_pool->size; n++) {
        if (NULL == (node = (orte_node_t*)opal_pointer_array_get_item(orte_node_pool, n))) {
            continue;
        }
        /* determine this node's prefix by looking for first non-alpha char */
        fullname = false;
        len = strlen(node->name);
        startnum = -1;
        memset(prefix, 0, ORTE_MAX_NODE_PREFIX);
        suffix = '\0';
        for (i=0; i < len; i++) {
            if (!isalpha(node->name[i])) {
                /* found a non-alpha char */
                if (!isdigit(node->name[i])) {
                    /* if it is anything but a digit, we just use
                     * the entire name, which by definition is unique
                     * by the way we created the node pool
                     */
                    fullname = true;
                    break;
                }
                if ('0' == node->name[i]) {
                    /* if the digit is 0, then add it to the prefix */
                    prefix[i] = node->name[i];
                    continue;
                }
                /* okay, this defines end of the prefix */
                startnum = i;
                break;
            }
            prefix[i] = node->name[i];
        }
        if (fullname || startnum < 0) {
            ndreg = OBJ_NEW(orte_regex_node_t);
            ndreg->prefix = strdup(node->name);
            start_sequence(jdata->jobid, node, ndreg, suffix, -1);
            opal_list_append(&nodelist, &ndreg->super);
            continue;
        }
        /* search for a suffix */
        if (isalpha(node->name[len-1])) {
            suffix = node->name[len-1];
        }
        nodenum = strtol(&node->name[startnum], NULL, 10);
        /* is this prefix already on our list? */
        found = false;
        for (item = opal_list_get_first(&nodelist);
             !found && item != opal_list_get_end(&nodelist);
             item = opal_list_get_next(item)) {
            ndreg = (orte_regex_node_t*)item;
            if (0 == strcmp(prefix, ndreg->prefix)) {
                /* yes - flag it */
                found = true;
                /* see if we have a range or a break in the list - we
                 * break the list if one of the following conditions occurs:
                 *
                 * 1. the node number is out of sequence
                 *
                 * 2. the vpid of the first proc on the node is out
                 *    of sequence - i.e., does not equal the vpid of
                 *    the first proc on the first node + step if bynode,
                 *    or the last proc on the prior node + 1 if byslot
                 *
                 * 3. the starting node rank on the node is out of sequence
                 */
                num_nodes = opal_value_array_get_size(&ndreg->nodes)-1;
                start = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->nodes, int32_t, num_nodes);
                cnt = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->cnt, int32_t, num_nodes);
                sfx = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->suffix, char, num_nodes);
                if (suffix != sfx) {
                    /* break in suffix - start new range */
                    start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
                } else if (nodenum != cnt+start+1) {
                    /* have a break in the node sequence - start new range */
                    start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
                } else {
                    /* cycle through the procs on this node and see if the vpids
                     * for this jobid break the sequencing
                     */
                    vpid_start = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->starting_vpid, orte_vpid_t, num_nodes);
                    ppn = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->ppn, int32_t, num_nodes);
                    nrank = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->nrank, orte_node_rank_t, num_nodes);
                    compute_vpids(node, jdata->jobid, &start_vpid, &end_vpid, &nppn, &node_rank);
                    /* if the ppn doesn't match, then that breaks the sequence */
                    if (nppn != ppn) {
                        start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
                        break;
                    }
                    /* if the starting node rank doesn't match, then that breaks the sequence */
                    if (nrank != node_rank) {
                        start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
                        break;
                    }
                    /* if the vpids don't align correctly, then that breaks the sequence */
                    if (byslot) {
                        base = vpid_start + (ppn * (cnt+1));
                        if (start_vpid != base) {
                            /* break sequence */
                            start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
                            break;
                        }
                    } else {
                        if (start_vpid != (vpid_start + 1)) {
                            /* break sequence */
                            start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
                            break;
                        }
                    }
                    /* otherwise, if everything matches, just increment the cnt */
                    OPAL_VALUE_ARRAY_SET_ITEM(&ndreg->cnt, int32_t, num_nodes, cnt+1);
                }
            }
        }
        if (!found) {
            /* need to add it */
            ndreg = OBJ_NEW(orte_regex_node_t);
            ndreg->prefix = strdup(prefix);
            start_sequence(jdata->jobid, node, ndreg, suffix, nodenum);
            opal_list_append(&nodelist, &ndreg->super);
        }
    }
    
    /* the regular expression begins with the jobid */
    asprintf(&tmp, "LJID=%s", ORTE_LOCAL_JOBID_PRINT(jdata->jobid));
    opal_argv_append_nosize(&regexargs, tmp);
    free(tmp);
    
    /* next comes the starting daemon vpid */
    asprintf(&tmp, "DVPID=%s", ORTE_VPID_PRINT(jdata->map->daemon_vpid_start));
    opal_argv_append_nosize(&regexargs, tmp);
    free(tmp);
    
    /* begin constructing the regular expression for each prefix */
    for (item = opal_list_get_first(&nodelist);
         item != opal_list_get_end(&nodelist);
         item = opal_list_get_next(item)) {
        ndreg = (orte_regex_node_t*)item;
        
        /* how many values are in the array? */
        num_nodes = opal_value_array_get_size(&ndreg->nodes);
        if (0 == num_nodes) {
            /* solitary node */
            asprintf(&tmp, "%s", ndreg->prefix);
            opal_argv_append_nosize(&regexargs, tmp);
            free(tmp);
            continue;
        }
        /* build the regexargs array */
        for (i=0; i < num_nodes; i++) {
            /* get the index and the cnt */
            sfx = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->suffix, char, i);
            start = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->nodes, int32_t, i);
            cnt = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->cnt, int32_t, i);
            vpid_start = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->starting_vpid, orte_vpid_t, i);
            ppn = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->ppn, int32_t, i);
            nrank = OPAL_VALUE_ARRAY_GET_ITEM(&ndreg->nrank, orte_node_rank_t, i);
            /* if we have a range, construct it that way */
            if (0 < cnt) {
                if (ORTE_VPID_INVALID == vpid_start) {
                    /* no procs from this job on these nodes */
                    if ('\0' == sfx) {
                        asprintf(&tmp, "%s[%d-%d]", ndreg->prefix, start, start+cnt);
                    } else {
                        asprintf(&tmp, "%s[%d-%d]%c", ndreg->prefix, start, start+cnt, sfx);
                    }
                } else {
                    if ('\0' == sfx) {
                        asprintf(&tmp, "%s[%d-%d](%sx%d:%d:%d)", ndreg->prefix, start, start+cnt,
                                 ORTE_VPID_PRINT(vpid_start), ppn,
                                 (byslot) ? 1 : (int)jdata->map->num_nodes, (int)nrank);
                    } else {
                        asprintf(&tmp, "%s[%d-%d]%c(%sx%d:%d:%d)", ndreg->prefix, start, start+cnt,
                                 sfx, ORTE_VPID_PRINT(vpid_start), ppn,
                                 (byslot) ? 1 : (int)jdata->map->num_nodes, (int)nrank);
                    }
                }
            } else {
                /* single node - could be due to a break in the numbering, suffix, and/or vpids,
                 * or because it was a fullname node with no numbering in it
                 */
                if (ORTE_VPID_INVALID == vpid_start) {
                    /* no procs from this job on this node */
                    if (start < 0) {
                        /* fullname node */
                        if ('\0' == sfx) {
                            asprintf(&tmp, "%s", ndreg->prefix);
                        } else {
                            asprintf(&tmp, "%s%c", ndreg->prefix, sfx);
                        }
                    } else {
                        if ('\0' == sfx) {
                            asprintf(&tmp, "%s%d", ndreg->prefix, start);
                        } else {
                            asprintf(&tmp, "%s%d%c", ndreg->prefix, start, sfx);
                        }
                    }
                } else {
                    if (start < 0) {
                        if ('\0' == sfx) {
                            asprintf(&tmp, "%s(%sx%d:%d:%d)", ndreg->prefix,
                                     ORTE_VPID_PRINT(vpid_start), ppn,
                                     (byslot) ? 1 : (int)jdata->map->num_nodes, (int)nrank);
                        } else {
                            asprintf(&tmp, "%s%c(%sx%d:%d:%d)", ndreg->prefix, sfx,
                                     ORTE_VPID_PRINT(vpid_start), ppn,
                                     (byslot) ? 1 : (int)jdata->map->num_nodes, (int)nrank);
                        }
                    } else {
                        if ('\0' == sfx) {
                        asprintf(&tmp, "%s%d(%sx%d:%d:%d)", ndreg->prefix, start,
                                 ORTE_VPID_PRINT(vpid_start), ppn,
                                 (byslot) ? 1 : (int)jdata->map->num_nodes, (int)nrank);
                        } else {
                            asprintf(&tmp, "%s%d%c(%sx%d:%d:%d)", ndreg->prefix, start, sfx,
                                     ORTE_VPID_PRINT(vpid_start), ppn,
                                     (byslot) ? 1 : (int)jdata->map->num_nodes, (int)nrank);
                        }
                    }
                }
            }
            opal_argv_append_nosize(&regexargs, tmp);
            free(tmp);
        }
    }
    
    /* assemble final result */
    regexp = opal_argv_join(regexargs, ',');
    /* cleanup */
    opal_argv_free(regexargs);

    while (NULL != (item = opal_list_remove_first(&nodelist))) {
        OBJ_RELEASE(item);
    }
    OBJ_DESTRUCT(&nodelist);
    
    return regexp;
}

static int parse_node_range(char *orig, char ***names, orte_vpid_t *vpid_start,
                            int *ppn, int *step, int *nrank)
{
    char *base, *ptr, *ptr2, *next, suffix;
    int i, j, len, rc=ORTE_SUCCESS;
    bool found_range;

    /* protect input */
    base = strdup(orig);
    
    /* default to no procs */
    *vpid_start = ORTE_VPID_INVALID;
    
    /* start by searching for ranges and proc specifications */
    len = strlen(base);
    ptr = NULL;
    for (i = 0; i <= len; ++i) {
        if (base[i] == '[') {
            /* we found a range. this gets dealt with below */
            base[i] = '\0';
            found_range = true;
            break;
        }
        if (base[i] == '\0') {
            /* we found a singleton node - no procs on it */
            base[i] = '\0';
            found_range = false;
            break;
        }
        if (base[i] == '(') {
            /* we found a singleton node that has procs on it */
            base[i] = '\0';
            found_range = false;
            ptr = &base[i+1];
            break;
        }
    }
    if (i == 0) {
        /* we found a special character at the beginning of the string */
        orte_show_help("help-regex.txt", "regex:special-char", true, orig);
        rc = ORTE_ERR_BAD_PARAM;
        goto cleanup;
    }
    
    if (found_range) {
        /* If we found a range, now find the end of the range */
        for (j = i; j < len; ++j) {
            if (base[j] == ']') {
                base[j] = '\0';
                if (j < len-2) {
                    if (base[j+1] == '(') {
                        /* procs are in this range and there is no suffix */
                        ptr = &base[j+2];
                    } else {
                        /* we must have a suffix */
                        suffix = base[j+1];
                        if (j < len-3 && base[j+2] == '(') {
                            /* we also have procs in this range */
                            ptr = &base[j+3];
                        }
                    }
                }
                break;
            }
        }
        if (j >= len) {
            /* we didn't find the end of the range */
            orte_show_help("help-regex.txt", "regex:end-range-missing", true, orig);
            rc = ORTE_ERR_BAD_PARAM;
            goto cleanup;
        }
        
        rc = regex_parse_node_range(base, base + i + 1, suffix, names);
        if(ORTE_SUCCESS != rc) {
            orte_show_help("help-regex.txt", "regex:bad-value", true, orig);
            rc = ORTE_ERR_BAD_PARAM;
            goto cleanup;
        }    
    } else {
        /* If we didn't find a range, just add the node */
        
        OPAL_OUTPUT_VERBOSE((1, orte_debug_output,
                             "%s regex:extract:nodenames: found node: %s",
                             ORTE_NAME_PRINT(ORTE_PROC_MY_NAME), base));
        
        if(ORTE_SUCCESS != (rc = opal_argv_append_nosize(names, base))) {
            ORTE_ERROR_LOG(rc);
            rc = ORTE_ERR_BAD_PARAM;
            goto cleanup;
        }
    }
    
    if (NULL != ptr) {  /* we have procs on these nodes */
        /* find the end of the description */
        ptr2 = strchr(ptr, ')');
        if (NULL == ptr2) {
            /* malformed */
            orte_show_help("help-regex.txt", "regex:bad-value", true, ptr);
            return ORTE_ERROR;
        }
        *ptr2 = '\0';
        
        /* the proc description is in the format:
         * starting-vpidxppn:step:starting-node-rank
         * where step=step between vpids
         */
        
        /* start by extracting the starting vpid */
        if (NULL == (ptr2 = strchr(ptr, 'x'))) {
            /* malformed */
            orte_show_help("help-regex.txt", "regex:bad-value", true, ptr);
            return ORTE_ERROR;
        }
        *ptr2 = '\0';
        orte_util_convert_string_to_vpid(vpid_start, ptr);
        
        /* get ppn */
        next = ptr2 + 1;
        if (NULL == (ptr2 = strchr(next, ':'))) {
            /* malformed */
            orte_show_help("help-regex.txt", "regex:bad-value", true, next);
            return ORTE_ERROR;
        }
        *ptr2 = '\0';
        *ppn = strtol(next, NULL, 10);
        
        /* get step */
        next = ptr2 + 1;
        if (NULL == (ptr2 = strchr(next, ':'))) {
            /* malformed */
            orte_show_help("help-regex.txt", "regex:bad-value", true, next);
            return ORTE_ERROR;
        }
        *ptr2 = '\0';
        *step = strtol(next, NULL, 10);
        
        /* get the starting node rank */
        next = ptr2 + 1;
        *nrank = strtol(next, NULL, 10);
    }
    
cleanup:
    free(base);
    return rc;
}

int orte_regex_decode_maps(char *regexp)
{
    char **seqs, *ptr, **names;
    int i, j, k, n, rc;
    int ppn, step, start_nrank, nrank;
    int32_t tmp32;
    orte_vpid_t daemon_vpid, vpid;
    orte_jobid_t jobid;
    orte_nid_t *nid;
    orte_jmap_t *jmap;
    orte_pmap_t *pmap;
    bool found;
    
    /* if regexp is NULL, then nothing to parse */
    if (NULL == regexp) {
        return ORTE_ERR_SILENT;
    }
    
    /* break the regexp into its component parts - this is trivial
     * because they are all separated by commas!
     */
    seqs = opal_argv_split(regexp, ',');
    
    /* we need to have at least three elements or something is wrong */
    if (opal_argv_count(seqs) < 3) {
        opal_argv_free(seqs);
        return ORTE_ERROR;
    }
    
    /* the first entry is the local jobid, so we extract that and
     * convert it into a global jobid
     */
    ptr = strchr(seqs[0], '=');
    if (NULL == ptr) {
        opal_argv_free(seqs);
        return ORTE_ERROR;
    }
    ptr++;
    tmp32 = strtol(ptr, NULL, 10);
    jobid = ORTE_CONSTRUCT_LOCAL_JOBID(ORTE_PROC_MY_NAME->jobid, tmp32);
    
    /* do we already have a jmap entry for this job? */
    found = false;
    for (i=0; i < orte_jobmap.size; i++) {
        if (NULL == (jmap = (orte_jmap_t*)opal_pointer_array_get_item(&orte_jobmap, i))) {
            continue;
        }
        if (jmap->job == jobid) {
            /* got it */
            found = true;
            break;
        }
    }
    if (!found) {
        /* don't already have it - add it */
        jmap = OBJ_NEW(orte_jmap_t);
        jmap->job = jobid;
        opal_pointer_array_add(&orte_jobmap, jmap);
    }
    
    /* the second entry is the starting daemon vpid for the job being launched */
    ptr = strchr(seqs[0], '=');
    if (NULL == ptr) {
        opal_argv_free(seqs);
        return ORTE_ERROR;
    }
    ptr++;
    /* use the standard vpid conversion routine to get the value - don't attempt
     * to directly convert it with strtol as the value could be INVALID
     */
    if (ORTE_SUCCESS != (rc = orte_util_convert_string_to_vpid(&daemon_vpid, ptr))) {
        ORTE_ERROR_LOG(rc);
        opal_argv_free(seqs);
        return rc;
    }
    
    /* the remaining entries contain the name of the nodes in the system, how
     * many procs (if any) on each of those nodes, the starting vpid of the
     * procs on those nodes, and the starting node rank for the procs on
     * each node
     */
    names = NULL;
    for (n=2; n < opal_argv_count(seqs); n++) {
        /* parse the node entry to get a list of all node names in it */
        if (ORTE_SUCCESS != (rc = parse_node_range(seqs[n], &names, &vpid, &ppn, &step, &start_nrank))) {
            ORTE_ERROR_LOG(rc);
            opal_argv_free(seqs);
            return rc;
        }
        for (i=0; i < opal_argv_count(names); i++) {
            /* is this name already in our nidmap? */
            found = false;
            for (j=0; j < orte_nidmap.size; j++) {
                if (NULL == (nid = (orte_nid_t*)opal_pointer_array_get_item(&orte_nidmap, j))) {
                    continue;
                }
                if (0 == strcmp(nid->name, names[i])) {
                    /* yep - we have it */
                    found = true;
                    break;
                }
            }
            if (!found) {
                /* must not already have it - create one */
                nid = OBJ_NEW(orte_nid_t);
                nid->name = strdup(names[i]);
                nid->index = opal_pointer_array_add(&orte_nidmap, nid);
            }
            /* are there any procs on this node? */
            if (ORTE_VPID_INVALID != vpid) {
                /* yep - add a daemon if we don't already one, otherwise
                 * this is just adding procs to an existing daemon
                 */
                if (ORTE_VPID_INVALID != daemon_vpid &&
                    ORTE_VPID_INVALID == nid->daemon) {
                    /* no daemon assigned yet - add it */
                    nid->daemon = daemon_vpid;
                    daemon_vpid++;
                }
                /* cycle through the ppn, adding a pmap
                 * for each new rank
                 */
                nrank = start_nrank;
                for (k=0; k < ppn; k++) {
                    pmap = OBJ_NEW(orte_pmap_t);
                    pmap->node = nid->index;
                    pmap->local_rank = k;
                    pmap->node_rank = nrank++;
                    jmap->num_procs++;
                    opal_pointer_array_set_item(&jmap->pmap, vpid, pmap);
                    vpid += step;
                }
            }
        }
        opal_argv_free(names);
        names = NULL;
    }
    opal_argv_free(seqs);
    return ORTE_SUCCESS;
}
