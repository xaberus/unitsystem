

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	PREFIX_ERR_NULL_BASE,
	PREFIX_ERR_SYMBOL_TO_LONG,
	PREFIX_ERR_TEXT_TO_LONG,
};

err_t us_prefix_set(us_prefix_t prefix, const char symbol[], const char text[], const uint8_t base, const int8_t power)
{
	check_in_ptr(prefix, 0);
	check_in_string(symbol, 0);
	check_in_string(text, 0);

	if (base == 0)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PREFIX_ERR_NULL_BASE);
	if (strlen(symbol) > US_MAX_PREFIX_SYMBOL_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PREFIX_ERR_SYMBOL_TO_LONG);
	if (strlen(text) > US_MAX_PREFIX_TEXT_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PREFIX_ERR_TEXT_TO_LONG);

	err_t err = {0};

	prefix->base = base;
	prefix->power = power;
	strncpy(prefix->symbol, symbol, US_MAX_PREFIX_SYMBOL_LENGTH);
	strncpy(prefix->text, text, US_MAX_PREFIX_TEXT_LENGTH);

	return err;
}

err_t us_prefix_copy(us_prefix_t prefix, const us_prefix_t source)
{
	check_in_ptr(prefix, 0);
	check_in_ptr(source, 0);

	err_t err = {0};
	/* TODO: checks */

	*prefix = *source;

	return err;
}


err_t us_prefix_tostring(const us_prefix_t prefix, size_t length, char buffer[])
{
	check_in_ptr(prefix, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", prefix->symbol);

	err_t err = {0};
	return err;
}

err_t us_prefix_totext(const us_prefix_t prefix, size_t length, char buffer[])
{
	check_in_ptr(prefix, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", prefix->text);

	err_t err = {0};
	return err;
}

err_t us_prefix_equal(const us_prefix_t left, const us_prefix_t right, bool * result)
{
	check_in_ptr(left, 0);
	check_in_ptr(right, 0);

	*result = 
			(left->base == right->base) &&
			(left->power == right->power) &&
			(strncmp(left->symbol, right->symbol, US_MAX_PREFIX_SYMBOL_LENGTH) == 0) &&
			(strncmp(left->text, right->text, US_MAX_PREFIX_TEXT_LENGTH) == 0);

	err_t err = {0};
	return err;
}


#ifdef TEST

/*****************************************************************************/
/**************##########***##########***##########***##########**************/
/******************##*******##***********##***************##******************/
/******************##*******##########***##########*******##******************/
/******************##*******##*******************##*******##******************/
/******************##*******##########***##########*******##******************/
/*****************************************************************************/

BT_SUITE_DEF(us_prefix,"us_prefix tests");

BT_TEST_DEF(us_prefix, tostring, "tostring")
{
	err_t err = {0};
	us_prefix_t prefix;
	char buffer[US_MAX_PREFIX_SYMBOL_LENGTH + 1];
	UNUSED_PARAM(object);
	
	err = us_prefix_set(prefix, "k", "\\siprefix{k}", 10, 3);
	bt_assert_int_equal(err.composite, 0);

	err = us_prefix_tostring(prefix, US_MAX_PREFIX_SYMBOL_LENGTH + 1, buffer);
	bt_assert_int_equal(err.composite, 0);
	bt_assert_str_equal(buffer, "k");

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_prefix, tostext, "totext")
{
	err_t err = {0};
	us_prefix_t prefix;
	char buffer[US_MAX_PREFIX_TEXT_LENGTH + 1];
	UNUSED_PARAM(object);

	err = us_prefix_set(prefix, "k", "\\siprefix{k}", 10, 3);
	bt_assert_int_equal(err.composite, 0);

	err = us_prefix_totext(prefix, US_MAX_PREFIX_TEXT_LENGTH + 1, buffer);
	bt_assert_int_equal(err.composite, 0);
	bt_assert_str_equal(prefix->text, "\\siprefix{k}");

	return BT_RESULT_OK;
}

#endif
