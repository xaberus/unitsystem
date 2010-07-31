

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	ATOM_ERR_SYMBOL_TO_LONG,
	ATOM_ERR_TEXT_TO_LONG,
};

err_t us_atom_new(const char symbol[], const char text[], us_atom_t ** ret)
{
	check_in_string(symbol, 0);
	check_in_string(text, 0);
	check_out_ptr(ret, 0);

	if (strlen(symbol) > US_MAX_ATOM_SYMBOL_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, ATOM_ERR_SYMBOL_TO_LONG);
	if (strlen(text) > US_MAX_ATOM_TEXT_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, ATOM_ERR_TEXT_TO_LONG);

	*ret = NULL;
	us_atom_t * atom = NULL;
	err_t err = {0};

	checked_malloc(atom, us_atom_t, err, 0, malloc_failed);

	strncpy(atom->symbol, symbol, US_MAX_ATOM_SYMBOL_LENGTH);
	strncpy(atom->text, text, US_MAX_ATOM_TEXT_LENGTH);

	*ret = atom;

	return err;

malloc_failed:
	return err;
}

err_t us_atom_tostring(const us_atom_t * atom, size_t length, char buffer[])
{
	check_in_ptr(atom, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", atom->symbol);

	err_t err = {0};
	return err;
}

err_t us_atom_totext(const us_atom_t * atom, size_t length, char buffer[])
{
	check_in_ptr(atom, 0);
	check_in_ptr(buffer, 0);

	snprintf(buffer, length, "%s", atom->text);

	err_t err = {0};
	return err;
}

err_t us_atom_delete(us_atom_t ** ret)
{
	check_out_ptr(ret, 0);
	check_in_ptr(*ret, 0);

	err_t err = {0};

	free(*ret);
	*ret = NULL;

	return err;
}



#ifdef TEST
BT_SUITE_DEF(us_atom,"us_atom tests");

BT_SUITE_SETUP_DEF(us_atom)
{
	err_t err;
	us_atom_t * atom = NULL;

	err = us_atom_new("g", "\\siatom{g}", &atom);
	
	bt_assert_ptr_not_equal(atom, NULL);
	bt_assert_int_equal(err.composite, 0);

	*object = atom;

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_atom, tostring, "tostring")
{
	us_atom_t * atom = object;

	bt_assert_str_equal(atom->symbol, "g");

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_atom, tostext, "totext")
{
	us_atom_t * atom = object;
	
	bt_assert_str_equal(atom->text, "\\siatom{g}");

	return BT_RESULT_OK;
}

BT_SUITE_TEARDOWN_DEF(us_atom)
{
	err_t err;
	us_atom_t * atom;

	atom = *object;
	err = us_atom_delete(&atom);
	
	bt_assert_int_equal(err.composite, 0);

	return BT_RESULT_OK;
}


#endif
