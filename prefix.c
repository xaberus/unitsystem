

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	PREFIX_ERR_NULL_BASE,
	PREFIX_ERR_SYMBOL_TO_LONG,
	PREFIX_ERR_TEXT_TO_LONG,
};

err_t us_prefix_new(const char symbol[], const char text[], const uint8_t base, const int8_t power, us_prefix_t ** ret)
{
	check_in_string(symbol, 0);
	check_in_string(text, 0);
	check_out_ptr(ret, 0);

	if (base == 0)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PREFIX_ERR_NULL_BASE);
	if (strlen(symbol) > US_MAX_PREFIX_SYMBOL_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PREFIX_ERR_SYMBOL_TO_LONG);
	if (strlen(text) > US_MAX_PREFIX_TEXT_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PREFIX_ERR_TEXT_TO_LONG);

	*ret = NULL;
	us_prefix_t * prefix = NULL;
	err_t err = {0};

	checked_malloc(prefix, us_prefix_t, err, 0, malloc_failed);

	prefix->base = base;
	prefix->power = power;
	strncpy(prefix->symbol, symbol, US_MAX_PREFIX_SYMBOL_LENGTH);
	strncpy(prefix->text, text, US_MAX_PREFIX_TEXT_LENGTH);

	*ret = prefix;

	return err;

malloc_failed:
	return err;
}

err_t us_prefix_tostring(const us_prefix_t * prefix, size_t length, char buffer[])
{
	check_in_ptr(prefix, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", prefix->symbol);

	err_t err = {0};
	return err;
}

err_t us_prefix_totext(const us_prefix_t * prefix, size_t length, char buffer[])
{
	check_in_ptr(prefix, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", prefix->text);

	err_t err = {0};
	return err;
}

err_t us_prefix_delete(us_prefix_t ** ret)
{
	check_out_ptr(ret, 0);
	check_in_ptr(*ret, 0);

	err_t err = {0};

	free(*ret);
	*ret = NULL;

	return err;
}



#ifdef TEST
BT_SUITE_DEF(us_prefix,"us_prefix tests");

BT_SUITE_SETUP_DEF(us_prefix)
{
	err_t err;
	us_prefix_t * prefix = NULL;

	err = us_prefix_new("k", "\\siprefix{k}", 10, 3, &prefix);
	
	bt_assert_ptr_not_equal(prefix, NULL);
	bt_assert_int_equal(err.composite, 0);

	*object = prefix;

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_prefix, tostring, "tostring")
{
	us_prefix_t * prefix = object;

	bt_assert_str_equal(prefix->symbol, "k");

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_prefix, tostext, "totext")
{
	us_prefix_t * prefix = object;
	
	bt_assert_str_equal(prefix->text, "\\siprefix{k}");

	return BT_RESULT_OK;
}

BT_SUITE_TEARDOWN_DEF(us_prefix)
{
	err_t err;
	us_prefix_t * prefix;

	prefix = *object;
	err = us_prefix_delete(&prefix);
	
	bt_assert_int_equal(err.composite, 0);

	return BT_RESULT_OK;
}


#endif
