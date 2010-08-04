

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	ATOM_ERR_SYMBOL_TO_LONG,
	ATOM_ERR_TEXT_TO_LONG,
};

err_t us_atom_set(us_atom_t atom, const char symbol[], const char text[])
{
	check_in_ptr(atom, 0);
	check_in_string(symbol, 0);
	check_in_string(text, 0);

	if (strlen(symbol) > US_MAX_ATOM_SYMBOL_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, ATOM_ERR_SYMBOL_TO_LONG);
	if (strlen(text) > US_MAX_ATOM_TEXT_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, ATOM_ERR_TEXT_TO_LONG);

	err_t err = {0};

	strncpy(atom->symbol, symbol, US_MAX_ATOM_SYMBOL_LENGTH);
	strncpy(atom->text, text, US_MAX_ATOM_TEXT_LENGTH);

	return err;
}

err_t us_atom_copy(us_atom_t atom, const us_atom_t source)
{
	check_in_ptr(atom, 0);
	check_in_ptr(source, 0);

	err_t err = {0};
	/* TODO: checks */

	*atom = *source;

	return err;
}


err_t us_atom_tostring(const us_atom_t atom, size_t length, char buffer[])
{
	check_in_ptr(atom, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", atom->symbol);

	err_t err = {0};
	return err;
}

err_t us_atom_totext(const us_atom_t atom, size_t length, char buffer[])
{
	check_in_ptr(atom, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", atom->text);

	err_t err = {0};
	return err;
}

err_t us_atom_equal(const us_atom_t left, const us_atom_t right, bool * result)
{
	check_in_ptr(left, 0);
	check_in_ptr(right, 0);

	*result = 
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

BT_SUITE_DEF(us_atom,"us_atom tests");

BT_TEST_DEF(us_atom, tostring, "tostring")
{
	err_t err = {0};
	us_atom_t atom;
	char buffer[US_MAX_ATOM_SYMBOL_LENGTH + 1];
	UNUSED_PARAM(object);

	us_atom_set(atom, "g", "\\siatom{g}");
	bt_assert_int_equal(err.composite, 0);

	us_atom_tostring(atom, US_MAX_ATOM_SYMBOL_LENGTH + 1, buffer);
	bt_assert_int_equal(err.composite, 0);
	bt_assert_str_equal(buffer, "g");

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_atom, tostext, "totext")
{
	err_t err = {0};
	us_atom_t atom;
	char buffer[US_MAX_ATOM_TEXT_LENGTH + 1];
	UNUSED_PARAM(object);

	us_atom_set(atom, "g", "\\siatom{g}");
	bt_assert_int_equal(err.composite, 0);

	us_atom_totext(atom, US_MAX_ATOM_TEXT_LENGTH + 1, buffer);
	bt_assert_int_equal(err.composite, 0);
	bt_assert_str_equal(buffer, "\\siatom{g}");

	return BT_RESULT_OK;
}

#endif
