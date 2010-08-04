
#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sexp.h>

#ifdef TEST
#endif

enum {
	LIBRARY_LIST_UNKNOWN_IDENTIFIER,
	LIBRARY_LIST_START_NOT_LIST,
	LIBRARY_LIST_START_EMPTY,
	LIBRARY_LIST_START_NOT_VALUE,
	LIBRARY_LIST_START_NOT_UNITSYSTEM,
	LIBRARY_LIST_UNITSYSTEM_EMPTY,
	LIBRARY_LIST_START_NOT_LIBRARY,
	LIBRARY_LIST_LIBRARY_EMPTY,
	LIBRARY_LIST_TUPLE_EMPTY,
	LIBRARY_LIST_PREFIXES_EMPTY,
	LIBRARY_LIST_PREFIX_EMPTY,
	LIBRARY_LIST_PREFIX_INCOMPLETE,
	LIBRARY_LIST_PREFIX_SYMBOL_TOO_LONG,
	LIBRARY_LIST_PREFIX_TEXT_TOO_LONG,
	LIBRARY_LIST_ATOM_EMPTY,
	LIBRARY_LIST_ATOM_INCOMPLETE,
	LIBRARY_LIST_ATOM_SYMBOL_TOO_LONG,
	LIBRARY_LIST_ATOM_TEXT_TOO_LONG,
};

err_t _us_library_tuple_getval(const char ** val, size_t * val_len, sexp_t * tuple)
{
	err_t err = {0};
	*val = NULL;
	*val_len = 0;

	if (!tuple->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_TUPLE_EMPTY);
	if (tuple->next->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	
	*val = tuple->next->val;
	*val_len = tuple->next->val_used;

	return err;
}

err_t _us_library_parse_template(us_library_t uslib, sexp_t * template)
{
	err_t err = {0};
	return err;
}


err_t _us_library_parse_prefix(us_prefix_t usprefix, sexp_t * prefix)
{
	err_t err = {0};
	sexp_t * args;
	sexp_t * arg;
	sexp_t * val;
	const char * symbol = NULL;
	size_t symbol_len = 0;
	const char * text = NULL;
	size_t text_len = 0;
	const char * base = NULL;
	size_t base_len = 0;
	const char * power = NULL;
	size_t power_len = 0;


	if (!prefix->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PREFIX_EMPTY);
	
	args = prefix->next;

	arg = args;
	while (arg) {
		val = arg->list;

		if (val->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	
		if (strncmp(val->val, "symbol", val->val_used) == 0) {
			err = _us_library_tuple_getval(&symbol, &symbol_len, val);
		} else if (strncmp(val->val, "text", val->val_used) == 0){
			err = _us_library_tuple_getval(&text, &text_len, val);
		} else if (strncmp(val->val, "base", val->val_used) == 0){
			err = _us_library_tuple_getval(&base, &base_len, val);
		} else if (strncmp(val->val, "power", val->val_used) == 0){
			err = _us_library_tuple_getval(&power, &power_len, val);
		} else
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

		if (err.composite)
			return err;

		arg = arg->next;
	}
	
	if (!symbol || !text || !base || !power)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PREFIX_INCOMPLETE);

	if (symbol_len > US_MAX_PREFIX_SYMBOL_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PREFIX_SYMBOL_TOO_LONG);
	if (text_len > US_MAX_PREFIX_TEXT_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PREFIX_TEXT_TOO_LONG);

	strcpy(usprefix->symbol, symbol);
	strcpy(usprefix->text, text);
	{
		char buff[base_len+1];
		memcpy(buff, base, base_len); buff[base_len] = '\0';
		usprefix->base = atoi(buff);
	}
	{
		char buff[power_len+1];
		memcpy(buff, power, power_len); buff[power_len] = '\0';
		usprefix->power = atoi(buff);
	}


	return err;
}

err_t _us_library_parse_prefixes(us_library_t uslib, sexp_t * prefixes)
{
	err_t err = {0};
	sexp_t * pfxlist;
	sexp_t * iter;
	sexp_t * entry;
	us_prefix_t prefix;
	us_prefix_list_t * list = NULL, * new;

	if (!prefixes->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PREFIXES_EMPTY);

	pfxlist = prefixes->next;

	iter = pfxlist;
	while (iter) {
		entry = iter->list;
	
		if (entry->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
		if (strncmp(entry->val, "prefix", entry->val_used) != 0)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);
			
		err = _us_library_parse_prefix(prefix, entry);
		if (err.composite)
			return err;

		checked_malloc(new, us_prefix_list_t, err, 0, malloc_error);
		memset(new, 0, sizeof(us_prefix_list_t));

		us_prefix_copy(new->prefix, prefix);

		/* append */
		if (list) {
			list->next = new;
			list = new;
		} else {
			list = new;
			uslib->prefixes = new;
		}

		iter = iter->next;
	}

malloc_error:
	return err;
}

err_t _us_library_parse_atom(us_atom_t usatom, sexp_t * atom)
{
	err_t err = {0};
	sexp_t * args;
	sexp_t * arg;
	sexp_t * val;
	const char * symbol = NULL;
	size_t symbol_len = 0;
	const char * text = NULL;
	size_t text_len = 0;

	if (!atom->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_ATOM_EMPTY);
	
	args = atom->next;

	arg = args;
	while (arg) {
		val = arg->list;

		if (val->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	
		if (strncmp(val->val, "symbol", val->val_used) == 0) {
			err = _us_library_tuple_getval(&symbol, &symbol_len, val);
		} else if (strncmp(val->val, "text", val->val_used) == 0){
			err = _us_library_tuple_getval(&text, &text_len, val);
		} else
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

		if (err.composite)
			return err;

		arg = arg->next;
	}
	
	if (!symbol || !text)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_ATOM_INCOMPLETE);

	if (symbol_len > US_MAX_ATOM_SYMBOL_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_ATOM_SYMBOL_TOO_LONG);
	if (text_len > US_MAX_ATOM_TEXT_LENGTH)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_ATOM_TEXT_TOO_LONG);

	strcpy(usatom->symbol, symbol);
	strcpy(usatom->text, text);

	return err;
}

err_t _us_library_parse_atoms(us_library_t uslib, sexp_t * atoms)
{
	err_t err = {0};
	sexp_t * atmlist;
	sexp_t * iter;
	sexp_t * entry;
	us_atom_t atom;
	us_atom_list_t * list = NULL, * new;

	if (!atoms->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PREFIXES_EMPTY);

	atmlist = atoms->next;

	iter = atmlist;
	while (iter) {
		entry = iter->list;
	
		if (entry->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
		if (strncmp(entry->val, "atom", entry->val_used) != 0)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);
			
		err = _us_library_parse_atom(atom, entry);
		if (err.composite)
			return err;

		checked_malloc(new, us_atom_list_t, err, 0, malloc_error);
		memset(new, 0, sizeof(us_atom_list_t));

		us_atom_copy(new->atom, atom);

		/* append */
		if (list) {
			list->next = new;
			list = new;
		} else {
			list = new;
			uslib->atoms = new;
		}

		iter = iter->next;
	}

malloc_error:
	return err;
}

err_t _us_library_parse_base_units(us_library_t uslib, sexp_t * base_units)
{
	err_t err = {0};
	return err;
}

err_t _us_library_parse_library(us_library_t uslib, sexp_t * lib)
{
	err_t err = {0};
	sexp_t * liblist;
	sexp_t * iter;
	sexp_t * entry;

	if (!lib->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_LIBRARY_EMPTY);
	liblist = lib->next;
	
	iter = liblist;
	while (iter) {
		entry = iter->list;
	
		if (entry->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);

		if (strncmp(entry->val, "template", entry->val_used) == 0)
			err = _us_library_parse_template(uslib, entry);
		else if (strncmp(entry->val, "prefixes", entry->val_used) == 0)
			err = _us_library_parse_prefixes(uslib, entry);
		else if (strncmp(entry->val, "atoms", entry->val_used) == 0)
			err = _us_library_parse_atoms(uslib, entry);
		else if (strncmp(entry->val, "base_units", entry->val_used) == 0)
			err = _us_library_parse_base_units(uslib, entry);
		else
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

		if (err.composite)
			return err;

		iter = iter->next;
	}

	return err;
}

err_t _us_library_parse_unitsystem(us_library_t uslib, sexp_t * sx)
{
	sexp_t * us;
	sexp_t * uslist;
	sexp_t * lib;


	if (sx->ty != SEXP_LIST)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_LIST);
	if (!sx->list)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_EMPTY);

	us = sx->list;
	if (us->ty != SEXP_VALUE)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	if (strncmp(us->val, "unitsystem", us->val_used) != 0)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_UNITSYSTEM);
	
	if (!us->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNITSYSTEM_EMPTY);
	uslist = us->next;
	
	lib = uslist->list;
	if (lib->ty != SEXP_VALUE)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	if (strncmp(lib->val, "library", lib->val_used) != 0)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_LIBRARY);

	return _us_library_parse_library(uslib, lib);
}

err_t us_library_init(us_library_t lib, const char specfile[])
{
	check_in_ptr(lib, 0);
	check_in_ptr(specfile, 0);

	err_t err = {0};
	char buff[516];
	sexp_t	* sx = NULL;
	pcont_t *	cont = NULL;
	ssize_t len;
	int fd;
	
	memset(lib, 0, sizeof(us_library_s));

	fd = open(specfile, O_RDONLY);
	if (fd == -1)
		return construct_errno_error();

	len = -1;
	
	while(len != 0) {
		len = read(fd, buff, 512);
		if (len == -1) {
			err = construct_errno_error();
			goto cleanup;
		}
		else if (len == 0)
			break;

		buff[len]='\0';

		if (!cont)
			cont = init_continuation(buff);

		sx = iparse_sexp(buff, len, cont);
		while (sx) {
			err = _us_library_parse_unitsystem(lib, sx);
			destroy_sexp(sx);
			sx = iparse_sexp(buff, len, cont);
		}

	}

cleanup:

	if (sx)
		destroy_sexp(sx);
	if (cont)
		destroy_continuation(cont);
	sexp_cleanup();
	close(fd);

	return err;
}

err_t us_library_clear(us_library_t lib)
{
	check_in_ptr(lib, 0);

	{
		us_prefix_list_t * iter = lib->prefixes;
		us_prefix_list_t * next = NULL;
		while (iter) {
			next = iter->next;
			free(iter);
			iter = next;
		}
	}
		{
		us_atom_list_t * iter = lib->atoms;
		us_atom_list_t * next = NULL;
		while (iter) {
			next = iter->next;
			free(iter);
			iter = next;
		}
	}


	err_t err = {0};
	
	return err;
}

#if TEST
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢╭────────╮▢▢▢╭────────╮▢▢▢╭────────╮▢▢▢╭────────╮▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢╰──╮  ╭──╯▢▢▢│ ╭──────╯▢▢▢│ ╭──────╯▢▢▢╰──╮  ╭──╯▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢│  │▢▢▢▢▢▢│ ╰────╮▢▢▢▢▢│ ╰──────╮▢▢▢▢▢▢│  │▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢│  │▢▢▢▢▢▢│ ╭────╯▢▢▢▢▢╰──────╮ │▢▢▢▢▢▢│  │▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢│  │▢▢▢▢▢▢│ ╰──────╮▢▢▢╭──────╯ │▢▢▢▢▢▢│  │▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢╰──╯▢▢▢▢▢▢╰────────╯▢▢▢╰────────╯▢▢▢▢▢▢╰──╯▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/

BT_SUITE_DEF(us_library, "us_library tests");
BT_SUITE_SETUP_DEF(us_library)
{
	err_t err = {0};
	us_library_s * lib;

	lib = malloc(sizeof(us_library_s));
	if (!lib)
		return BT_RESULT_FAIL;

	memset(lib, 0, sizeof(us_library_s));

	err = us_library_init(lib, "si.lst");
	bt_assert_err_equal_i(err, 0, 0, 0);

	*object = lib;
	
	return BT_RESULT_OK;
}

BT_TEST_DEF(us_library, empty, "")
{
	us_library_s * lib = object;

	{
		us_prefix_list_t * iter = lib->prefixes;
		while (iter) {
			char buff[US_MAX_PREFIX_TEXT_LENGTH];

			us_prefix_totext(iter->prefix, sizeof(buff), buff);
			bt_log("prefix: %s [%d^%d]\n", buff, iter->prefix->base, iter->prefix->power);

			iter = iter->next;
		}
	}
	{
		us_atom_list_t * iter = lib->atoms;
		while (iter) {
			char buff[US_MAX_PREFIX_TEXT_LENGTH];

			us_atom_totext(iter->atom, sizeof(buff), buff);
			bt_log("atom: %s\n", buff);

			iter = iter->next;
		}
	}


	return BT_RESULT_OK;
}

BT_SUITE_TEARDOWN_DEF(us_library)
{
	err_t err = {0};
	us_library_s * lib = *object;

	err = us_library_clear(lib);
	bt_assert_err_equal_i(err, 0, 0, 0);

	free(lib);
	
	return BT_RESULT_OK;
}

#endif
