/*
 * Copyright 2008-2019 Aerospike, Inc.
 *
 * Portions may be licensed to Aerospike, Inc. under one or more contributor
 * license agreements.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
#pragma once

#include <aerospike/as_atomic.h>
#include <aerospike/as_std.h>
#include <aerospike/as_status.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * MACROS
 *****************************************************************************/

/**
 * Maximum namespace size including null byte.  Effective maximum length is 31.
 */
#define AS_MAX_NAMESPACE_SIZE 32

/******************************************************************************
 * TYPES
 *****************************************************************************/
struct as_node_s;
struct as_cluster_s;
struct as_error_s;
struct as_key_s;

/**
 * @private
 * Map of namespace data partitions to nodes.
 *
 * TODO - not ideal for replication factor > 2.
 */
typedef struct as_partition_s {
	struct as_node_s* master;
	struct as_node_s* prole;
	uint32_t regime;
} as_partition;

/**
 * @private
 * Map of namespace to data partitions.
 */
typedef struct as_partition_table_s {
	uint32_t ref_count;
	uint32_t size;
	char ns[AS_MAX_NAMESPACE_SIZE];
	bool sc_mode;
	char pad[7];
	as_partition partitions[];
} as_partition_table;

/**
 * @private
 * Reference counted array of partition table pointers.
 */
typedef struct as_partition_tables_s {
	uint32_t ref_count;
	uint32_t size;
	as_partition_table* array[];
} as_partition_tables;

/**
 * @private
 * Partition info.
 */
typedef struct as_partition_info_s {
	const char* ns;
	void* partition;  // as_partition or as_shm_partition.
	uint32_t partition_id;
	bool sc_mode;
} as_partition_info;

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @private
 * Create reference counted structure containing partition tables.
 */
as_partition_tables*
as_partition_tables_create(uint32_t capacity);

/**
 * @private
 * Destroy partition tables.
 */
void
as_partition_tables_destroy(as_partition_tables* tables);

/**
 * @private
 * Release reference counted access to partition tables.
 */
static inline void
as_partition_tables_release(as_partition_tables* tables)
{
	if (as_aaf_uint32(&tables->ref_count, -1) == 0) {
		as_partition_tables_destroy(tables);
	}
}

/**
 * @private
 * Destroy and release memory for partition table.
 */
void
as_partition_table_destroy(as_partition_table* table);

/**
 * @private
 * Get partition table given namespace.
 */
as_partition_table*
as_partition_tables_get(as_partition_tables* tables, const char* ns);

/**
 * @private
 * Is node referenced in any partition table.
 */
bool
as_partition_tables_find_node(as_partition_tables* tables, struct as_node_s* node);
	
/**
 * @private
 * Return partition ID given digest.
 */
static inline uint32_t
as_partition_getid(const uint8_t* digest, uint32_t n_partitions)
{
	return (*(uint16_t*)digest) & (n_partitions - 1);
}

/**
 * @private
 * Initialize partition info given key.  If this function succeeds and not using shared memory,
 * as_partition_tables_release() must be called when done with partition.
 */
as_status
as_partition_info_init(
	as_partition_info* pi, struct as_cluster_s* cluster, struct as_error_s* err,
	const struct as_key_s* key
	);

#ifdef __cplusplus
} // end extern "C"
#endif
