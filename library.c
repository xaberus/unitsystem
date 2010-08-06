
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
	LIBRARY_LIST_BASE_UNIT_EMPTY,
	LIBRARY_LIST_BASE_UNIT_INCOMPLETE,
	LIBRARY_LIST_PARTS_EMPTY,
	LIBRARY_LIST_PART_EMPTY,
	LIBRARY_LIST_PART_INCOMPLETE,

	LIBRARY_INVALID_PREFIX_REF,
	LIBRARY_INVALID_ATOM_REF,
};

static err_t _us_library_tuple_getval(const char ** val, size_t * val_len, sexp_t * tuple)
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

#include "template.inc"

static err_t _us_library_parse_prefix(us_prefix_t usprefix, sexp_t * prefix)
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

static err_t _us_library_parse_prefixes(us_library_t uslib, sexp_t * prefixes)
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

static err_t _us_library_parse_atom(us_atom_t usatom, sexp_t * atom)
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

static err_t _us_library_parse_atoms(us_library_t uslib, sexp_t * atoms)
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
	
	return err;

malloc_error:
	return err;
}

static err_t _us_library_lookup_prefix_by_symbol(us_library_t uslib, size_t length, const char * symbol, const us_prefix_s ** ret)
{
	err_t err = {0};
	
	*ret = NULL;

	us_prefix_list_t * iter = uslib->prefixes;
	const us_prefix_s * prefix = NULL;
	while (iter) {
		if (strncmp(symbol, iter->prefix->symbol, length) == 0)
			prefix = iter->prefix;
		iter = iter->next;
	}

	if (!prefix)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_INVALID_PREFIX_REF);

	*ret = prefix;

	return err;
}

static err_t _us_library_lookup_atom_by_symbol(us_library_t uslib, size_t length, const char * symbol, const us_atom_s ** ret)
{
	err_t err = {0};
	
	*ret = NULL;

	us_atom_list_t * iter = uslib->atoms;
	const us_atom_s * atom = NULL;
	while (iter) {
		if (strncmp(symbol, iter->atom->symbol, length) == 0)
			atom = iter->atom;
		iter = iter->next;
	}

	if (!atom)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_INVALID_ATOM_REF);

	*ret = atom;

	return err;
}

static err_t _us_library_parse_part(us_library_t uslib, us_part_t uspart, sexp_t * part)
{
	err_t err = {0};
	sexp_t * args;
	sexp_t * arg;
	sexp_t * val;

	const char * pref = NULL;
	size_t pref_len = 0;
	const char * aref = NULL;
	size_t aref_len = 0;
	const char * power = NULL;
	size_t power_len = 0;

	const us_prefix_s * usprefix;
	const us_atom_s * usatom;

	if (!part->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PART_EMPTY);

	args = part->next;

	arg = args;
	while (arg) {
		val = arg->list;

		if (val->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	
		if (strncmp(val->val, "prefixref", val->val_used) == 0) {
			err = _us_library_tuple_getval(&pref, &pref_len, val);
		} else if (strncmp(val->val, "atomref", val->val_used) == 0){
			err = _us_library_tuple_getval(&aref, &aref_len, val);
		} else if (strncmp(val->val, "power", val->val_used) == 0){
			err = _us_library_tuple_getval(&power, &power_len, val);
		} else
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

		arg = arg->next;
	}
	
	if (!power) {
		power = "1";
		power_len = 1;
	}

	
	if (!pref || !aref || !power)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PART_INCOMPLETE);

	err = _us_library_lookup_prefix_by_symbol(uslib, pref_len, pref, &usprefix);
	if (err.composite)
		return err;
	err = _us_library_lookup_atom_by_symbol(uslib, aref_len, aref, &usatom);
	if (err.composite)
		return err;

	mpq_t pow;

	mpq_init(pow); mpq_set_str(pow, power, 10);
	err = us_part_set(uspart, usprefix, usatom, pow);
	mpq_clear(pow);

	return err;
}

static err_t _us_library_parse_base_unit(us_library_t uslib, us_base_unit_t usbase, sexp_t * base_unit)
{
	err_t err = {0};
	sexp_t * atom;
	sexp_t * list;
	sexp_t * part;
	sexp_t * parts;
	sexp_t * iter;
	const us_atom_s * atomref = NULL;
	unsigned int usparts_length, i;
	us_atom_t usatom;
	us_atom_list_t * llist = uslib->atoms, * new;


	if (!base_unit->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_BASE_UNIT_EMPTY);

	list = base_unit->next;
	atom = list->list;
	
	if (atom->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
	if (strncmp(atom->val, "atom", atom->val_used) == 0) {
		/* uncommon, but valid */
		while(llist && llist->next)
			llist = llist->next;
		
		err = _us_library_parse_atom(usatom, atom);
		if (err.composite)
			return err;

		checked_malloc(new, us_atom_list_t, err, 0, malloc_error);
		memset(new, 0, sizeof(us_atom_list_t));
malloc_error:
		if (err.composite)
			return err;

		us_atom_copy(new->atom, usatom);

		/* append */
		if (llist) {
			llist->next = new;
		} else {
			uslib->atoms = new;
		}

		atomref = new->atom;
	} else if (strncmp(atom->val, "atomref", atom->val_used) == 0) {
		const char * symbol;
		size_t symbol_len;
		err = _us_library_tuple_getval(&symbol, &symbol_len, atom);
		if (err.composite)
			return err;
		err = _us_library_lookup_atom_by_symbol(uslib, symbol_len, symbol, &atomref);
		if (err.composite)
			return err;
	} else
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

	err = us_base_unit_set_composite(usbase, atomref);
	if (err.composite)
		return reconstruct_error(err, 0);

	if (!list->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_BASE_UNIT_INCOMPLETE);

	list = list->next;
	parts = list->list;
	if (!parts->next)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_PARTS_EMPTY);

	iter = parts->next;
	usparts_length = 0;
	while (iter) {
		part = iter->list;

		usparts_length++;

		if (part->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
		if (strncmp(part->val, "part", part->val_used) != 0)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

		iter = iter->next;
	}

	us_part_t usparts[usparts_length];
	const us_part_s * olist[usparts_length];

	for (unsigned int k = 0; k < usparts_length; k++) {
		us_part_init(usparts[k]);
		olist[k] = usparts[k];
	}

	i = 0;
	iter = parts->next;
	while (iter) {
		part = iter->list;

		err = _us_library_parse_part(uslib, usparts[i++], part);
		if (err.composite)
			goto clear_parts;

		iter = iter->next;
	}
	
	err = us_base_unit_set_parts(usbase, i, olist);
	if (err.composite) {
		err = reconstruct_error(err, 0); /* fall through */
	}

clear_parts:
	for (unsigned int k = 0; k < usparts_length; k++) {
		us_part_clear(usparts[k]);
	}

	return err;
}

static err_t _us_library_parse_base_units(us_library_t uslib, sexp_t * base_units)
{
	err_t err = {0};
	sexp_t * bulist;
	sexp_t * iter;
	sexp_t * entry;
	us_base_unit_list_t * list = NULL, * new;

	/* may be empty? */
	if (!base_units)
		return err;
	
	bulist = base_units->next;

	iter = bulist;
	while (iter) {
		entry = iter->list;
	
		if (entry->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
		if (strncmp(entry->val, "base_unit", entry->val_used) != 0)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_UNKNOWN_IDENTIFIER);

		checked_malloc(new, us_base_unit_list_t, err, 0, malloc_error);
		memset(new, 0, sizeof(us_base_unit_list_t));

		err = us_base_unit_init(new->unit);
		if (err.composite)
			goto cleanup;

		err = _us_library_parse_base_unit(uslib, new->unit, entry);
		if (err.composite)
			goto cleanup;

		/* append */
		if (list) {
			list->next = new;
			list = new;
		} else {
			list = new;
			uslib->units = new;
		}


		iter = iter->next;
	}

	return err;
cleanup:
	us_base_unit_clear(new->unit);
	free(new);
	return err;
malloc_error:
	return err;
}

static err_t _us_library_parse_library(us_library_t uslib, sexp_t * lib)
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

		if (strncmp(entry->val, "template", entry->val_used) == 0) {
			err = _us_library_parse_template(uslib, entry);
		} else if (strncmp(entry->val, "prefixes", entry->val_used) == 0)
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

static err_t _us_library_parse_unitsystem(us_library_t uslib, sexp_t * sx)
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
		us_base_unit_list_t * iter = lib->units;
		us_base_unit_list_t * next = NULL;
		while (iter) {
			next = iter->next;
			us_base_unit_clear(iter->unit);
			free(iter);
			iter = next;
		}
		lib->units = NULL;
	}
	{
		us_prefix_list_t * iter = lib->prefixes;
		us_prefix_list_t * next = NULL;
		while (iter) {
			next = iter->next;
			free(iter);
			iter = next;
		}
		lib->prefixes = NULL;
	}
	{
		us_atom_list_t * iter = lib->atoms;
		us_atom_list_t * next = NULL;
		while (iter) {
			next = iter->next;
			free(iter);
			iter = next;
		}
		lib->atoms = NULL;
	}

	free(lib->storage);
	lib->storage = NULL;

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
	us_library_s * lib;

	lib = malloc(sizeof(us_library_s));
	if (!lib)
		return BT_RESULT_FAIL;

	memset(lib, 0, sizeof(us_library_s));

	*object = lib;
	
	return BT_RESULT_OK;
}

BT_TEST_DEF(us_library, empty, "")
{
	err_t err = {0};
	us_library_s * lib = object;
	
	err = us_library_init(lib, "si.lst");
	bt_assert_err_equal_i(err, 0, 0, 0);


	{
		us_prefix_list_t * iter = lib->prefixes;
		while (iter) {
			char buff[US_MAX_PREFIX_TEXT_LENGTH + 1];

			us_prefix_totext(iter->prefix, sizeof(buff), buff);
			bt_log("prefix: %s [%d^%d]\n", buff, iter->prefix->base, iter->prefix->power);

			iter = iter->next;
		}
	}
	{
		us_atom_list_t * iter = lib->atoms;
		while (iter) {
			char buff[US_MAX_PREFIX_TEXT_LENGTH + 1];

			us_atom_totext(iter->atom, sizeof(buff), buff);
			bt_log("atom: %s\n", buff);

			iter = iter->next;
		}
	}
	{
		us_base_unit_list_t * iter = lib->units;
		while (iter) {
			size_t length;
			us_base_unit_totext_length(iter->unit, &lib->pattern, &length);
			char buff[length];
			us_base_unit_totext(iter->unit, &lib->pattern, sizeof(buff), buff);
			bt_log("unit: %s\n", buff);

			iter = iter->next;
		}
	}


	err = us_library_clear(lib);
	bt_assert_err_equal_i(err, 0, 0, 0);


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
