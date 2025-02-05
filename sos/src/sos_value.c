/*
 * Copyright (c) 2014-2015 Open Grid Computing, Inc. All rights reserved.
 * Copyright (c) 2014-2015 Sandia Corporation. All rights reserved.
 *
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the U.S. Government.
 * Export of this program may require a license from the United States
 * Government.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the BSD-type
 * license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *      Neither the name of Sandia nor the names of any contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *      Neither the name of Open Grid Computing nor the names of any
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *      Modified source versions must be plainly marked as such, and
 *      must not be misrepresented as being the original software.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: Tom Tucker tom at ogc dot us
 */
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sos/sos.h>
#include <errno.h>
#include <assert.h>
#include "sos_priv.h"

typedef int (*cmp_fn_t)(sos_value_t a, sos_value_t b, size_t size);

static int INT16_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.int16_ < b->data->prim.int16_)
		return -1;
	if (a->data->prim.int16_ > b->data->prim.int16_)
		return 1;
	return 0;
}

static int INT32_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.int32_ < b->data->prim.int32_)
		return -1;
	if (a->data->prim.int32_ > b->data->prim.int32_)
		return 1;
	return 0;
}

static int INT64_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.int64_ < b->data->prim.int64_)
		return -1;
	if (a->data->prim.int64_ > b->data->prim.int64_)
		return 1;
	return 0;
}

static int UINT16_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.uint16_ < b->data->prim.uint16_)
		return -1;
	if (a->data->prim.uint16_ > b->data->prim.uint16_)
		return 1;
	return 0;
}

static int UINT32_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.uint32_ < b->data->prim.uint32_)
		return -1;
	if (a->data->prim.uint32_ > b->data->prim.uint32_)
		return 1;
	return 0;
}

static int UINT64_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.uint64_ < b->data->prim.uint64_)
		return -1;
	if (a->data->prim.uint64_ > b->data->prim.uint64_)
		return 1;
	return 0;
}

static int FLOAT_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	float res = a->data->prim.float_ - b->data->prim.float_;
	if (res > 0)
		return 1;
	else if (res < 0)
		return -1;
	return 0;
}

static int DOUBLE_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	double res = a->data->prim.double_ - b->data->prim.double_;
	if (res > 0)
		return 1;
	else if (res < 0)
		return -1;
	return 0;
}

static int LONG_DOUBLE_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.long_double_ > b->data->prim.long_double_)
		return 1;
	if (a->data->prim.long_double_ < b->data->prim.long_double_)
		return -1;
	return 0;
}

static int TIMESTAMP_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	if (a->data->prim.timestamp_.tv.tv_sec < b->data->prim.timestamp_.tv.tv_sec)
		return -1;
	if (a->data->prim.timestamp_.tv.tv_sec > b->data->prim.timestamp_.tv.tv_sec)
		return 1;
	if (a->data->prim.timestamp_.tv.tv_usec < b->data->prim.timestamp_.tv.tv_usec)
		return -1;
	if (a->data->prim.timestamp_.tv.tv_usec > b->data->prim.timestamp_.tv.tv_usec)
		return 1;
	return 0;
}

static int OBJ_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	return memcmp(a->data->prim.ref_.idx_data.bytes, b->data->prim.ref_.idx_data.bytes, sizeof(ods_idx_data_t));
}

static int STRUCT_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	return memcmp(a->data->prim.struc_, b->data->prim.struc_, size);
}

static int BYTE_ARRAY_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	int cmp_len = sos_array_count(a);
	if (cmp_len > sos_array_count(b))
		cmp_len = sos_array_count(b);
	int res = memcmp(sos_array_data(a, byte_), sos_array_data(b, byte_), cmp_len);
	if (res == 0)
		return sos_array_count(a) - sos_array_count(b);
	return res;
}

static int CHAR_ARRAY_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	return BYTE_ARRAY_cmp(a, b, size);
}

static int OBJ_ARRAY_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	size_t a_len = sos_array_count(a);
	size_t b_len = sos_array_count(b);
	int i, res;
	size_t cmp_len = (a_len < b_len ? a_len : b_len);
	for (i = 0; i < cmp_len; i++) {
		res = memcmp(sos_array_data(a, ref_)[i].idx_data.bytes,
			     sos_array_data(b, ref_)[i].idx_data.bytes,
			     sizeof(sos_obj_ref_t));
		if (res)
			return res;
	}
	return a_len - b_len;
}

#define ARRAY_CMP(_n_, _t_, _f_)					\
static int _n_ ## _cmp(sos_value_t a, sos_value_t b, size_t size) \
{								\
	size_t a_len = sos_array_count(a);	\
	size_t b_len = sos_array_count(b);	\
	int i;							\
	size_t cmp_len = (a_len < b_len ? a_len : b_len);	\
	for (i = 0; i < cmp_len; i++) {				\
		if (sos_array_data(a, _f_)[i] < sos_array_data(b, _f_)[i]) \
			return -1;				\
		if (sos_array_data(a, _f_)[i] > sos_array_data(b, _f_)[i])	\
			return 1;				\
	}							\
	return a_len - b_len;					\
}

ARRAY_CMP(INT16_ARRAY, int16_t, int16_)
ARRAY_CMP(INT32_ARRAY, int32_t, int32_)
ARRAY_CMP(INT64_ARRAY, int64_t, int64_)
ARRAY_CMP(UINT16_ARRAY, uint16_t, uint16_)
ARRAY_CMP(UINT32_ARRAY, uint32_t, uint32_)
ARRAY_CMP(UINT64_ARRAY, uint64_t, uint64_)
ARRAY_CMP(FLOAT_ARRAY, float, float_)
ARRAY_CMP(DOUBLE_ARRAY, double, double_)
ARRAY_CMP(LONG_DOUBLE_ARRAY, long double, long_double_)

static int JOIN_cmp(sos_value_t a, sos_value_t b, size_t size)
{
	sos_array_t ck_a = sos_array(a);
	sos_array_t ck_b = sos_array(b);
	ods_key_comp_t comp_a;
	ods_key_comp_t comp_b;
	off_t koff;
	int64_t res = 0;
	int cmp_len = ck_a->count;
	if (ck_b->count < cmp_len)
		cmp_len = ck_b->count;
	for (koff = 0; koff < cmp_len;) {
		comp_a = (ods_key_comp_t)&(ck_a->data.byte_[koff]);
		comp_b = (ods_key_comp_t)&(ck_b->data.byte_[koff]);
		switch (comp_a->type) {
		case SOS_TYPE_DOUBLE:
			if (comp_a->value.double_ < comp_b->value.double_)
				return -1;
			if (comp_a->value.double_ > comp_b->value.double_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.double_);
			break;
		case SOS_TYPE_TIMESTAMP:
			if (comp_a->value.tv_.tv_sec < comp_b->value.tv_.tv_sec)
				return -1;
			if (comp_a->value.tv_.tv_sec > comp_b->value.tv_.tv_sec)
				return 1;
			if (comp_a->value.tv_.tv_usec < comp_b->value.tv_.tv_usec)
				return -1;
			if (comp_a->value.tv_.tv_usec > comp_b->value.tv_.tv_usec)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.tv_);
			break;
		case SOS_TYPE_UINT64:
			if (comp_a->value.uint64_ < comp_b->value.uint64_)
				return -1;
			if (comp_a->value.uint64_ > comp_b->value.uint64_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.uint64_);
			break;
		case SOS_TYPE_INT64:
			if (comp_a->value.int64_ < comp_b->value.int64_)
				return -1;
			if (comp_a->value.int64_ > comp_b->value.int64_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.int64_);
			break;
		case SOS_TYPE_FLOAT:
			if (comp_a->value.float_ < comp_b->value.float_)
				return -1;
			if (comp_a->value.float_ > comp_b->value.float_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.float_);
			break;
		case SOS_TYPE_UINT32:
			if (comp_a->value.uint32_ < comp_b->value.uint32_)
				return -1;
			if (comp_a->value.uint32_ > comp_b->value.uint32_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.uint32_);
			break;
		case SOS_TYPE_INT32:
			if (comp_a->value.int32_ < comp_b->value.int32_)
				return -1;
			if (comp_a->value.int32_ > comp_b->value.int32_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.int32_);
			break;
		case SOS_TYPE_UINT16:
			if (comp_a->value.uint16_ < comp_b->value.uint16_)
				return -1;
			if (comp_a->value.uint16_ > comp_b->value.uint16_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.uint16_);
			break;
		case SOS_TYPE_INT16:
			if (comp_a->value.int16_ < comp_b->value.int16_)
				return -1;
			if (comp_a->value.int16_ > comp_b->value.int16_)
				return 1;
			koff += sizeof(uint16_t) + sizeof(comp_a->value.int16_);
			break;
		case SOS_TYPE_STRUCT:
		case SOS_TYPE_BYTE_ARRAY:
			res = comp_a->value.str.len < comp_b->value.str.len ? comp_a->value.str.len : comp_b->value.str.len;
			res = memcmp(comp_a->value.str.str, comp_b->value.str.str, res);
			if (res == 0)
				res = comp_a->value.str.len - comp_b->value.str.len;
			if (res)
				return res;
			/* NB: if we get here we know the value and length of the two values are identical */
			koff += sizeof(uint16_t) + sizeof(comp_a->value.str.len) + comp_a->value.str.len;
			break;
		case SOS_TYPE_CHAR_ARRAY:
			res = comp_a->value.str.len < comp_b->value.str.len ? comp_a->value.str.len : comp_b->value.str.len;
			res = strncmp(comp_a->value.str.str, comp_b->value.str.str, res);
			if (res == 0)
				res = comp_a->value.str.len - comp_b->value.str.len;
			if (res)
				return res;
			/* NB: if we get here we know the value and length of the two strings are identical */
			koff += sizeof(uint16_t) + sizeof(comp_a->value.str.len) + comp_a->value.str.len;
			break;
		case SOS_TYPE_LONG_DOUBLE:
		case SOS_TYPE_INT16_ARRAY:
		case SOS_TYPE_INT32_ARRAY:
		case SOS_TYPE_INT64_ARRAY:
		case SOS_TYPE_UINT16_ARRAY:
		case SOS_TYPE_UINT32_ARRAY:
		case SOS_TYPE_UINT64_ARRAY:
		case SOS_TYPE_FLOAT_ARRAY:
		case SOS_TYPE_DOUBLE_ARRAY:
		case SOS_TYPE_LONG_DOUBLE_ARRAY:
		default:
			break;
		}
	}
	if (res == 0)
		return ck_a->count - ck_b->count;
	return res;
}

static cmp_fn_t cmp_fn_table[] = {
	[SOS_TYPE_INT16] = INT16_cmp,
	[SOS_TYPE_INT32] = INT32_cmp,
	[SOS_TYPE_INT64] = INT64_cmp,
	[SOS_TYPE_UINT16] = UINT16_cmp,
	[SOS_TYPE_UINT32] = UINT32_cmp,
	[SOS_TYPE_UINT64] = UINT64_cmp,
	[SOS_TYPE_FLOAT] = FLOAT_cmp,
	[SOS_TYPE_DOUBLE] = DOUBLE_cmp,
	[SOS_TYPE_LONG_DOUBLE] = LONG_DOUBLE_cmp,
	[SOS_TYPE_JOIN] = JOIN_cmp,
	[SOS_TYPE_OBJ] = OBJ_cmp,
	[SOS_TYPE_STRUCT] = STRUCT_cmp,
	[SOS_TYPE_TIMESTAMP] = TIMESTAMP_cmp,
	[SOS_TYPE_CHAR_ARRAY] = CHAR_ARRAY_cmp,
	[SOS_TYPE_BYTE_ARRAY] = BYTE_ARRAY_cmp,
	[SOS_TYPE_INT16_ARRAY] = INT16_ARRAY_cmp,
	[SOS_TYPE_INT32_ARRAY] = INT32_ARRAY_cmp,
	[SOS_TYPE_INT64_ARRAY] = INT64_ARRAY_cmp,
	[SOS_TYPE_UINT16_ARRAY] = UINT16_ARRAY_cmp,
	[SOS_TYPE_UINT32_ARRAY] = UINT32_ARRAY_cmp,
	[SOS_TYPE_UINT64_ARRAY] = UINT64_ARRAY_cmp,
	[SOS_TYPE_FLOAT_ARRAY] = FLOAT_ARRAY_cmp,
	[SOS_TYPE_DOUBLE_ARRAY] = DOUBLE_ARRAY_cmp,
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = LONG_DOUBLE_ARRAY_cmp,
	[SOS_TYPE_OBJ_ARRAY] = OBJ_ARRAY_cmp,
};

/**
 * \brief Compare two value
 *
 * Compares <tt>a</tt> and <tt>b</tt> and returns <0 if a < b, 0 if
 * a == b, and >0 if a > b
 *
 * \param a The lhs
 * \param b The rhs
 * \returns The result as described above
 */
int sos_value_cmp(sos_value_t a, sos_value_t b)
{
	if (a->type < sizeof(cmp_fn_table)/sizeof(cmp_fn_table[0]))
		return cmp_fn_table[a->type](a, b, sos_value_size(a));
	return a == b;
}

typedef int (*true_fn_t)(sos_value_t a);

static int INT16_true(sos_value_t a)
{
	return a->data->prim.int16_ != 0;
}

static int INT32_true(sos_value_t a)
{
	return a->data->prim.int32_ != 0;
}

static int INT64_true(sos_value_t a)
{
	return a->data->prim.int64_ != 0;
}

static int UINT16_true(sos_value_t a)
{
	return a->data->prim.uint16_ != 0;
}

static int UINT32_true(sos_value_t a)
{
	return a->data->prim.uint32_ != 0;
}

static int UINT64_true(sos_value_t a)
{
	return a->data->prim.uint64_ != 0;
}

static int FLOAT_true(sos_value_t a)
{
	return a->data->prim.float_ != 0;
}

static int DOUBLE_true(sos_value_t a)
{
	return a->data->prim.double_ != 0;
}

static int LONG_DOUBLE_true(sos_value_t a)
{
	return a->data->prim.long_double_ != 0;
}

static int TIMESTAMP_true(sos_value_t a)
{
	return (a->data->prim.timestamp_.tv.tv_sec != 0) && (a->data->prim.timestamp_.tv.tv_usec != 0);
}

static int OBJ_true(sos_value_t a)
{
	return 1;
}

static int STRUCT_true(sos_value_t a)
{
	return 1;
}

static int ARRAY_true(sos_value_t a)
{
	return 0 == sos_array_count(a);
}

static true_fn_t true_fn_table[] = {
	[SOS_TYPE_INT16] = INT16_true,
	[SOS_TYPE_INT32] = INT32_true,
	[SOS_TYPE_INT64] = INT64_true,
	[SOS_TYPE_UINT16] = UINT16_true,
	[SOS_TYPE_UINT32] = UINT32_true,
	[SOS_TYPE_UINT64] = UINT64_true,
	[SOS_TYPE_FLOAT] = FLOAT_true,
	[SOS_TYPE_DOUBLE] = DOUBLE_true,
	[SOS_TYPE_LONG_DOUBLE] = LONG_DOUBLE_true,
	[SOS_TYPE_OBJ] = OBJ_true,
	[SOS_TYPE_STRUCT] = STRUCT_true,
	[SOS_TYPE_TIMESTAMP] = TIMESTAMP_true,
	[SOS_TYPE_CHAR_ARRAY] = ARRAY_true,
	[SOS_TYPE_BYTE_ARRAY] = ARRAY_true,
	[SOS_TYPE_INT16_ARRAY] = ARRAY_true,
	[SOS_TYPE_INT32_ARRAY] = ARRAY_true,
	[SOS_TYPE_INT64_ARRAY] = ARRAY_true,
	[SOS_TYPE_UINT16_ARRAY] = ARRAY_true,
	[SOS_TYPE_UINT32_ARRAY] = ARRAY_true,
	[SOS_TYPE_UINT64_ARRAY] = ARRAY_true,
	[SOS_TYPE_FLOAT_ARRAY] = ARRAY_true,
	[SOS_TYPE_DOUBLE_ARRAY] = ARRAY_true,
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = ARRAY_true,
	[SOS_TYPE_OBJ_ARRAY] = ARRAY_true,
};
/**
 * \brief Determine if a value is true or false
 *
 * Primitive values are true if they are non-zero. An array
 * value is \c true if it has a non-zero count.
 *
 * \param a The value
 * \returns 1 (true) or 0 (false)
 */
int sos_value_true(sos_value_t a)
{
	if (a->type < sizeof(cmp_fn_table)/sizeof(cmp_fn_table[0]))
		return true_fn_table[a->type](a);
	return 0;
}

sos_value_t sos_value_new()
{
	return calloc(1, sizeof(struct sos_value_s));
}

void sos_value_free(sos_value_t v)
{
	free(v);
}

/**
 * \brief Return a value for the specified object's attribute.
 *
 * This function returns a sos_value_t for an object's attribute.
 * The reference on this value should be dropped with sos_value_put()
 * when the application is finished with the value.
 *
 * \param obj The object handle
 * \param attr The attribute handle
 * \returns The value of the object's attribute.
 */
sos_value_t sos_value(sos_obj_t obj, sos_attr_t attr)
{
	sos_value_t value = sos_value_new();
	if (value)
		value = sos_value_init(value, obj, attr);
	return value;
}

static sos_value_t __sos_join_value_init(sos_value_t val, sos_obj_t obj, sos_attr_t attr)
{
	struct sos_value_s *v_;
	sos_value_t *v;
	struct sos_value_s _v_[MAX_JOIN_ATTRS];
	sos_value_t _v[MAX_JOIN_ATTRS];
	ods_key_comp_t comp;
	size_t comp_len;
	struct sos_array_s *data;
	int i = 0, count, join_id;
	sos_schema_t schema = sos_attr_schema(attr);
	size_t size;
	sos_attr_t join_attr;

	assert(schema);
	assert(sos_attr_type(attr) == SOS_TYPE_JOIN);

	count = attr->ext_ptr->count;
	if (count <= MAX_JOIN_ATTRS) {
		v_ = _v_;
		v = _v;
	} else {
		v_ = calloc(count, sizeof *v_);
		if (!v_)
			return NULL;
		v = calloc(count, sizeof *v);
		if (!v)
			goto err;
	}

	size = 0;
	for (i = 0; i < count; i++) {
		join_id = attr->ext_ptr->data.uint32_[i];
		join_attr = sos_schema_attr_by_id(schema, join_id);
		if (!join_attr) {
			errno = EINVAL;
			goto err;
		}
		v[i] = sos_value_init(&v_[i], obj, join_attr);
		if (!v[i]) {
			errno = ENOMEM;
			goto err;
		}
		if (sos_attr_is_array(join_attr) || (sos_attr_type(join_attr) == SOS_TYPE_STRUCT))
			size += sos_value_size(v[i]) + sizeof(comp->value.str) + sizeof(uint16_t);
		else
			size += sos_value_size(v[i]) + sizeof(uint16_t);
	}

	if (size > (sizeof(val->data_) + sizeof(*data))) {
		data = malloc(sizeof(*data) + size);
		if (!data) {
			errno = ENOMEM;
			goto err;
		}
	} else {
		data = (sos_array_t)&val->data_;
	}
	val->type = sos_attr_type(attr);
	val->attr = attr;
	val->obj = NULL;
	val->data = (sos_value_data_t)data;

	comp = (ods_key_comp_t)data->data.byte_;

	for (size = 0, i = 0; i < count; i++) {
		comp = __sos_set_key_comp(comp, v[i], &comp_len);
		size += comp_len;
		sos_value_put(v[i]);
	}
	data->count = size;
	if (count > MAX_JOIN_ATTRS) {
		free(v_);
		free(v);
	}
	return val;
 err:
	while (i) {
		sos_value_put(v[i]);
		i--;
	}
	if (count > MAX_JOIN_ATTRS) {
		free(v_);
		free(v);
	}
	return NULL;
}

/**
 * \brief Initialize a value with an object's attribute data
 *
 * The returned value has a reference on the parameter object <tt>obj<tt>. The
 * caller must call sos_value_put() when the value is no longer in use.
 *
 * \param val Pointer to the value to be initialized
 * \param obj The object handle
 * \param attr The attribute handle
 * \retval The value handle
 */
sos_value_t sos_value_init(sos_value_t val, sos_obj_t obj, sos_attr_t attr)
{
	sos_value_data_t ref_val;
	if (!obj) {
		// TODO: What to do about arrays?
		memset(val, 0, sizeof(*val));
		val->type = sos_attr_type(attr);
		val->attr = attr;
		val->data = &val->data_;
		return val;
	}
	val->type = sos_attr_type(attr);
	if (sos_attr_type(attr) == SOS_TYPE_JOIN)
		return __sos_join_value_init(val, obj, attr);

	val->attr = attr;
	ref_val = (sos_value_data_t)&obj->obj->as.bytes[attr->data->offset];

	if (sos_attr_is_array(attr)) {
		if (0 == ref_val->array_info.offset)
			return NULL;
		else
			ref_val = (sos_value_data_t)&(obj->obj->as.bytes[ref_val->array_info.offset]);
	}
	val->obj = sos_obj_get(obj);
	val->data = ref_val;
	return val;
}

sos_value_t sos_value_init_const(sos_value_t val, sos_type_t type, ...)
{
	sos_value_data_t vd;
	int count, size;
	char *data;
	va_list ap;
	vd = &val->data_;
	va_start(ap, type);
	val->type = type;
	val->attr = NULL;
	val->obj = NULL;
	val->data = &val->data_;
	switch (type) {
	case SOS_TYPE_INT32:
		vd->prim.int32_ = va_arg(ap, int32_t);
		break;
	case SOS_TYPE_INT64:
		vd->prim.int64_ = va_arg(ap, int64_t);
		break;
	case SOS_TYPE_UINT16:
		vd->prim.uint16_ = va_arg(ap, int);
		break;
	case SOS_TYPE_UINT32:
		vd->prim.uint32_ = va_arg(ap, uint32_t);
		break;
	case SOS_TYPE_UINT64:
		vd->prim.uint64_ = va_arg(ap, uint64_t);
		break;
	case SOS_TYPE_FLOAT:
		vd->prim.float_ = va_arg(ap, double);
		break;
	case SOS_TYPE_DOUBLE:
		vd->prim.double_ = va_arg(ap, double);
		break;
	case SOS_TYPE_LONG_DOUBLE:
		vd->prim.long_double_ = va_arg(ap, long double);
		break;
	case SOS_TYPE_TIMESTAMP:
		vd->prim.timestamp_.tv = va_arg(ap, struct ods_timeval_s);
		break;
	case SOS_TYPE_BYTE_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count;
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, count);
		break;
	case SOS_TYPE_STRING:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count;
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, count);
		break;
	case SOS_TYPE_INT16_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(int16_t);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_INT32_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(int32_t);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_INT64_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(int64_t);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_UINT16_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(uint16_t);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_UINT32_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(uint32_t);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_UINT64_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(uint64_t);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_FLOAT_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(float);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_DOUBLE_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(double);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_LONG_DOUBLE_ARRAY:
		count = va_arg(ap, int);
		data =  va_arg(ap, char *);
		size = sizeof(val->data->array) + count * sizeof(long double);
		val->data = malloc(size);
		val->data->array.count = count;
		memcpy(val->data->array.data.byte_, data, size);
		break;
	case SOS_TYPE_JOIN:
	case SOS_TYPE_OBJ_ARRAY:
	case SOS_TYPE_OBJ:
	case SOS_TYPE_STRUCT:
	default:
		return NULL;
	}
	return val;
}

static size_t array_element_sizes[] =  {
	[SOS_TYPE_BYTE_ARRAY] = sizeof(unsigned char),
	[SOS_TYPE_CHAR_ARRAY] = sizeof(char),
	[SOS_TYPE_STRING] = sizeof(char),
	[SOS_TYPE_INT16_ARRAY] = sizeof(int16_t),
	[SOS_TYPE_INT32_ARRAY] = sizeof(int32_t),
	[SOS_TYPE_INT64_ARRAY] = sizeof(int64_t),
	[SOS_TYPE_UINT16_ARRAY] = sizeof(uint16_t),
	[SOS_TYPE_UINT32_ARRAY] = sizeof(uint32_t),
	[SOS_TYPE_UINT64_ARRAY] = sizeof(uint64_t),
	[SOS_TYPE_FLOAT_ARRAY] = sizeof(float),
	[SOS_TYPE_DOUBLE_ARRAY] = sizeof(double),
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = sizeof(long double),
	[SOS_TYPE_OBJ_ARRAY] = sizeof(sos_obj_ref_t)
};

size_t sos_array_data_size(sos_type_t typ, size_t count)
{
	if (typ < SOS_TYPE_ARRAY || typ > SOS_TYPE_LAST)
		return 0;
	return SOS_ROUNDUP(sizeof(sos_array_t) + (count * array_element_sizes[typ]), 8);
}

sos_value_data_t sos_value_data_new(sos_type_t typ, size_t count)
{
	sos_value_data_t data;
	if (typ < SOS_TYPE_ARRAY)
		return malloc(sizeof *data);
	data = malloc(sos_array_data_size(typ, count));
	data->array.count = count;
	return data;
}

void sos_value_data_del(sos_value_data_t vd)
{
	free(vd);
}

/**
 * \brief Copy a value to a memory value
 *
 * \param dst - Pointer to the destination value.
 * \param src - Pointer to the source value
 */
sos_value_t sos_value_copy(sos_value_t dst, sos_value_t src)
{
	dst->type = src->type;
	dst->attr = src->attr;
	size_t sz = sos_value_size(src);
	if (sos_value_type(src) == SOS_TYPE_STRUCT) {
		dst->data = malloc(sz);
		if (!dst->data)
			return NULL;
		memcpy(dst->data, src->data, sz);
		return dst;
	}
	if (!sos_value_is_array(src)) {
		memcpy(&dst->data_, src->data, sos_attr_size(src->attr));
		dst->data = &dst->data_;
		return dst;
	}
	if (sz > sizeof(union sos_value_data_u) - sizeof(struct sos_array_s)) {
		dst->data = malloc(sz + sizeof(struct sos_array_s));
		if (!dst->data)
			return NULL;
	} else {
		dst->data = &dst->data_;
	}
	sos_array(dst)->count = sos_array(src)->count;
	memcpy(sos_array_data(dst, byte_), sos_array_data(src, byte_), sz);
	return dst;
}

/**
 * \brief Returns a pointer into the memory of an object
 *
 * This function returns a pointer directly into the memory of an
 * object. The \t attr parameter specifies which attribute in the
 * object is to be accessed.
 *
 * If the attribute is an array, the array object to which the
 * attribute refers is returned in the arr_obj parameter if
 * provided. This is because the memory pointed to by the return value
 * has a reference on the array object and the caller is responsible
 * for releasing that reference when the memory is no longer in use.
 *
 * If the \t arr_obj pointer is NULL and the \t attr parameter is an
 * array, this function will assert.
 *
 * \param obj The object handle
 * \param attr The attribute handle
 * \param arr_obj Pointer to a sos_obj_t handle.
 */
sos_value_data_t sos_obj_attr_data(sos_obj_t obj, sos_attr_t attr, sos_obj_t *arr_obj)
{
	sos_value_data_t ref_val = NULL;
	if (arr_obj)
		*arr_obj = NULL;
	ref_val = (sos_value_data_t)&obj->obj->as.bytes[attr->data->offset];
	if (sos_attr_is_array(attr)) {
		if (0 == ref_val->array_info.offset)
			return NULL;
		ref_val = (sos_value_data_t)&(obj->obj->as.bytes[ref_val->array_info.offset]);
	}
	return ref_val;
}

/**
 * \brief Set an object value from a buffer
 *
 * Set the value from an untyped void buffer. If the buflen is too
 * large to fit, only sos_value_size() bytes will be written.
 *
 * \param val    The value handle
 * \param buf    The buffer containing the data
 * \param buflen The number of bytes to write from the buffer
 * \retval The number of bytes written
 */
size_t sos_value_memcpy(sos_value_t val, void *buf, size_t buflen)
{
	void *dst;
	if (buflen > sos_value_size(val))
		buflen = sos_value_size(val);
	if (!sos_attr_is_array(val->attr))
		dst = val->data;
	else
		dst = sos_array_data(val, byte_);
	memcpy(dst, buf, buflen);
	return buflen;
}

/**
 * \brief Drop a reference on a value object
 *
 * \param value The value handle.
 */
void sos_value_put(sos_value_t value)
{
	if (!value)
		return;
	if (value->obj) {
		sos_obj_put(value->obj);
		value->obj = NULL;
	} else {
		assert(value->data);
		if (value->data != &value->data_) {
			free(value->data);
			value->data = NULL;
		}
	}
}

/**
 * \brief Initialize a value with an object's attribute data
 *
 * Returns an object value handle for the specified attribute
 * name. If the <tt>attr_id</tt> parameter is non-null, the parameter
 * is filled in with associated attribute id.
 *
 * \param value Pointer to the value to be initialized
 * \param schema The schema handle
 * \param obj The object handle
 * \param name The attribute name
 * \param attr_id A pointer to the attribute id
 * \retval Pointer to the sos_value_t handle
 * \retval NULL if the specified attribute does not exist.
 */
sos_value_t sos_value_by_name(sos_value_t value, sos_schema_t schema, sos_obj_t obj,
			      const char *name, int *attr_id)
{
	sos_attr_t attr = sos_schema_attr_by_name(schema, name);
	if (!attr)
		return NULL;
	return sos_value_init(value, obj, attr);
}

/**
 * \brief Initialize a value with an object's attribute data
 *
 * Returns the sos_value_t for the attribute with the specified
 * id.
 *
 * \param value Pointer to the value to be initialized
 * \param obj		The SOS object handle
 * \param attr_id	The Id for the attribute.
 * \retval Pointer to the sos_value_t handle
 * \retval NULL if the specified attribute does not exist.
 */
sos_value_t sos_value_by_id(sos_value_t value, sos_obj_t obj, int attr_id)
{
	sos_attr_t attr = sos_schema_attr_by_id(obj->schema, attr_id);
	if (!attr)
		return NULL;
	return sos_value_init(value, obj, attr);
}

/**
 * \brief Format a value as a string
 *
 * \param v   The value handle
 * \param str Pointer to the string to receive the formatted value
 * \param len The size of the string in bytes.
 * \returns A pointer to the str argument or NULL if there was a
 *          formatting error.
 */
const char *sos_value_to_str(sos_value_t v, char *str, size_t len)
{
	return v->attr->to_str_fn(v, str, len);
}

/**
 * \brief Set the value from a string
 *
 * \param v The value handle
 * \param str The input string value to parse
 * \param endptr Receives the point in the str argumeent where parsing stopped.
 *               This parameter may be NULL.
 * \retval 0 The string was successfully parsed and the value set
 * \retval EINVAL The string was incorrectly formatted for this value
 *                type.
 */
int sos_value_from_str(sos_value_t v, const char *str, char **endptr)
{
	return v->attr->from_str_fn(v, str, endptr);
}

/**
 * \brief Return the length of the string required for the value
 *
 * This function returns the size of the string required to hold the
 * attribute value if formatted as a string. This function is useful
 * when allocating buffers used with the sos_obj_attr_to_str()
 * function. The returned value does not include the byte required
 * to contain the terminating '\0'.
 *
 * \param v The value handle
 * \returns The size of the string in bytes.
 */
size_t sos_value_strlen(sos_value_t v)
{
	return v->attr->strlen_fn(v);
}

size_t sos_value_data_set(sos_value_data_t vd, sos_type_t type, ...)
{
	size_t size;
	va_list ap;
	va_start(ap, type);
	size = sos_value_data_set_va(vd, type, ap);
	va_end(ap);
	return size;
}

size_t sos_value_data_set_va(sos_value_data_t vd, sos_type_t type, va_list ap)
{
	size_t size;
	switch (type) {
	case SOS_TYPE_INT32:
		vd->prim.int32_ = va_arg(ap, int32_t);
		size = sizeof(vd->prim.int32_);
		break;
	case SOS_TYPE_INT64:
		vd->prim.int64_ = va_arg(ap, int64_t);
		size = sizeof(vd->prim.int64_);
		break;
	case SOS_TYPE_INT16:
		vd->prim.int16_ = va_arg(ap, int);
		size = sizeof(vd->prim.int16_);
		break;
	case SOS_TYPE_UINT16:
		vd->prim.uint16_ = va_arg(ap, int);
		size = sizeof(vd->prim.uint16_);
		break;
	case SOS_TYPE_UINT32:
		vd->prim.uint32_ = va_arg(ap, uint32_t);
		size = sizeof(vd->prim.uint32_);
		break;
	case SOS_TYPE_UINT64:
		vd->prim.uint64_ = va_arg(ap, uint64_t);
		size = sizeof(vd->prim.uint64_);
		break;
	case SOS_TYPE_FLOAT:
		vd->prim.float_ = va_arg(ap, double);
		size = sizeof(vd->prim.float_);
		break;
	case SOS_TYPE_DOUBLE:
		vd->prim.double_ = va_arg(ap, double);
		size = sizeof(vd->prim.double_);
		break;
	case SOS_TYPE_LONG_DOUBLE:
		vd->prim.long_double_ = va_arg(ap, long double);
		size = sizeof(vd->prim.long_double_);
		break;
	case SOS_TYPE_TIMESTAMP:
		vd->prim.timestamp_.tv = va_arg(ap, struct ods_timeval_s);
		size = sizeof(vd->prim.timestamp_);
		break;
	case SOS_TYPE_OBJ:
		uuid_copy(vd->prim.ref_.ref.part_uuid, va_arg(ap, uint8_t *));
		vd->prim.ref_.ref.obj = va_arg(ap, uint64_t);
		size = sizeof(vd->prim.ref_);
		break;
	case SOS_TYPE_CHAR_ARRAY:
	case SOS_TYPE_STRUCT:
	case SOS_TYPE_BYTE_ARRAY:
		vd->array.count = va_arg(ap, int);
		size = vd->array.count;
		memcpy(vd->array.data.byte_, va_arg(ap, char *), size);
		break;
	case SOS_TYPE_DOUBLE_ARRAY:
	case SOS_TYPE_UINT64_ARRAY:
	case SOS_TYPE_INT64_ARRAY:
		vd->array.count = va_arg(ap, int);
		size = vd->array.count * sizeof(int64_t);
		memcpy(vd->array.data.byte_, va_arg(ap, char *), size);
		break;
	case SOS_TYPE_FLOAT_ARRAY:
	case SOS_TYPE_UINT32_ARRAY:
	case SOS_TYPE_INT32_ARRAY:
		vd->array.count = va_arg(ap, int);
		size = vd->array.count * sizeof(int32_t);
		memcpy(vd->array.data.byte_, va_arg(ap, char *), size);
		break;
	case SOS_TYPE_UINT16_ARRAY:
	case SOS_TYPE_INT16_ARRAY:
		vd->array.count = va_arg(ap, int);
		size = vd->array.count * sizeof(int16_t);
		memcpy(vd->array.data.byte_, va_arg(ap, char *), size);
		break;
	case SOS_TYPE_LONG_DOUBLE_ARRAY:
		vd->array.count = va_arg(ap, int);
		size = vd->array.count * sizeof(long double);
		memcpy(vd->array.data.byte_, va_arg(ap, char *), size);
		break;
	case SOS_TYPE_OBJ_ARRAY:
	case SOS_TYPE_JOIN:
	default:
		assert(0 == "JOIN is not a valid type");
		break;
	}
	return size;
}

size_t sos_obj_attr_value_set_va(sos_obj_t sos_obj, sos_attr_t attr, va_list ap)
{
	sos_value_data_t vd;
	sos_value_t v;
	struct sos_value_s val_;
	va_list ap_copy;
	size_t count;

	if (sos_attr_is_array(attr)) {
		va_copy(ap_copy, ap);
		count = va_arg(ap_copy, size_t);
		v = sos_array_new(&val_, attr, sos_obj, count);
		sos_obj_put(sos_obj);
		va_end(ap_copy);
		if (!v) {
			sos_error("%s: Error %d creating a new array of size %d\n",
				  __func__, errno, count);
			errno = ENOMEM;
			return 0;
		}
	}
	vd = sos_obj_attr_data(sos_obj, attr, NULL);
	if (!vd) {
		sos_error("%s: Error %d getting object data from attribute\n",
			  __func__, errno);
		return 0;
	}
	return sos_value_data_set_va(vd, sos_attr_type(attr), ap);
}

size_t sos_obj_attr_by_name_set(sos_obj_t sos_obj, const char *attr_name, ...)
{
	size_t size;
	va_list ap;
	va_start(ap, attr_name);
	sos_attr_t attr = sos_schema_attr_by_name(sos_obj->schema, attr_name);
	if (!attr)
		return 0;
	size = sos_obj_attr_value_set_va(sos_obj, attr, ap);
	va_end(ap);
	return size;
}

size_t sos_obj_attr_by_id_set(sos_obj_t sos_obj, int attr_id, ...)
{
	size_t size;
	va_list ap;
	va_start(ap, attr_id);
	sos_attr_t attr = sos_schema_attr_by_id(sos_obj->schema, attr_id);
	if (!attr)
		return 0;
	size = sos_obj_attr_value_set_va(sos_obj, attr, ap);
	va_end(ap);
	return size;
}

size_t sos_obj_attr_set(sos_obj_t sos_obj, sos_attr_t attr, ...)
{
	size_t size;
	va_list ap;
	va_start(ap, attr);
	size = sos_obj_attr_value_set_va(sos_obj, attr, ap);
	va_end(ap);
	return size;
}

size_t sos_value_data_size(sos_value_data_t vd, sos_type_t type)
{
	size_t size;
	switch (type) {
	case SOS_TYPE_INT32:
		size = sizeof(vd->prim.int32_);
		break;
	case SOS_TYPE_INT64:
		size = sizeof(vd->prim.int64_);
		break;
	case SOS_TYPE_UINT16:
		size = sizeof(vd->prim.uint16_);
		break;
	case SOS_TYPE_UINT32:
		size = sizeof(vd->prim.uint32_);
		break;
	case SOS_TYPE_UINT64:
		size = sizeof(vd->prim.uint64_);
		break;
	case SOS_TYPE_FLOAT:
	case SOS_TYPE_DOUBLE:
		size = sizeof(vd->prim.double_);
		break;
	case SOS_TYPE_LONG_DOUBLE:
		size = sizeof(vd->prim.long_double_);
		break;
	case SOS_TYPE_TIMESTAMP:
		size = sizeof(vd->prim.timestamp_);
		break;
	case SOS_TYPE_OBJ:
		size = sizeof(vd->prim.ref_);
		break;
	case SOS_TYPE_CHAR_ARRAY:
	case SOS_TYPE_STRUCT:
	case SOS_TYPE_BYTE_ARRAY:
		size = vd->array.count;
		break;
	case SOS_TYPE_DOUBLE_ARRAY:
	case SOS_TYPE_UINT64_ARRAY:
	case SOS_TYPE_INT64_ARRAY:
		size = vd->array.count * sizeof(int64_t);
		break;
	case SOS_TYPE_FLOAT_ARRAY:
	case SOS_TYPE_UINT32_ARRAY:
	case SOS_TYPE_INT32_ARRAY:
		size = vd->array.count * sizeof(int32_t);
		break;
	case SOS_TYPE_UINT16_ARRAY:
	case SOS_TYPE_INT16_ARRAY:
		size = vd->array.count * sizeof(int16_t);
		break;
	case SOS_TYPE_LONG_DOUBLE_ARRAY:
		size = vd->array.count * sizeof(long double);
		break;
	case SOS_TYPE_OBJ_ARRAY:
	case SOS_TYPE_JOIN:
	default:
		assert(0 == "JOIN is not a valid type");
		break;
	}
	return size;
}

sos_array_t sos_array(sos_value_t v)
{
	return &v->data->array;
}


