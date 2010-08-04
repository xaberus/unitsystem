

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	BASE_UNIT_EMPTY,
};

err_t us_base_unit_init(us_base_unit_t base)
{
	check_in_ptr(base, 0);

	memset(base, 0, sizeof(us_base_unit_s));

	err_t err = {0};

	base->parts_length = 0;
	base->parts = NULL;
	base->initialized = true;

	return err;
}

err_t us_base_unit_clear(us_base_unit_t base)
{
	check_in_ptr(base, 0);

	err_t err = {0};

	if (base->parts) {
		for (unsigned int i = 0; i < base->parts_length; i++) {
			err_t lerr = {0};
			lerr = us_part_clear(&base->parts[i]);
			if (lerr.composite)
				err = lerr;
		}
		free(base->parts);
	}

	memset(base, 0, sizeof(us_base_unit_s));
	
	return err;
}

err_t us_base_unit_set_composite(us_base_unit_t base, const us_atom_t atom)
{
	check_in_ptr(base, 0);
	check_bool_pfield(us_base_unit_t, base, initialized, true, ERR_MIN_NOT_INITIALIZED, 0);
	check_in_ptr(atom, 0);
	
	err_t err = {0};

	base->composite = atom;
	base->composite_set = true;

	return err;
}

err_t us_base_unit_totext_length(const us_base_unit_t base, size_t * length)
{
	check_in_ptr(base, 0);
	check_bool_pfield(us_base_unit_t, base, parts_set, true, ERR_MIN_NOT_SET, 0);
	check_out_ptr(length, 0);

	err_t err = {0};
	size_t res;

	*length = 0;
	
	for (unsigned int i = 0; i < base->parts_length; i++) {
		if (mpq_cmp_si(base->parts[i].power, 0, 1) < 0)
			err = us_part_totext_length(&base->parts[i], &res, true);
			if (err.composite)
				return reconstruct_error(err, 0);
		else
			err = us_part_totext_length(&base->parts[i], &res, false);
		*length += res;
	}

	const us_text_pattern_t * pattern = base->parts[0].pattern;

	*length += strlen(pattern->unit_s) + strlen(pattern->unit_e) + strlen(pattern->ufrac_s) + strlen(pattern->ufrac_m) + strlen(pattern->ufrac_e);
	
	return err;
}

err_t us_base_unit_totext(const us_base_unit_t base, size_t length, char buffer[])
{
	check_in_ptr(base, 0);
	check_bool_pfield(us_base_unit_t, base, parts_set, true, ERR_MIN_NOT_SET, 0);
	check_out_ptr(buffer, 0);
	
	err_t err = {0};
	const us_text_pattern_t * pattern = base->parts[0].pattern;
	size_t pos = 0;
	size_t len;

	unsigned int over = 0;
	unsigned int under = 0;
	
	for (unsigned int i = 0; i < base->parts_length; i++) {
		if (mpq_cmp_si(base->parts[i].power, 0, 1) < 0)
			under++;
		else
			over++;
	}

	const us_part_s * p_over[over];
	const us_part_s * p_under[under];

	for (unsigned int i = 0, o = 0, u = 0; i < base->parts_length; i++) {
		if (mpq_cmp_si(base->parts[i].power, 0, 1) < 0)
			p_under[u++] = &base->parts[i];
		else
			p_over[o++] = &base->parts[i];
	}

	pos += snprintf(buffer + pos, length-pos, "%s", pattern->unit_s);
	if (under)
		pos += snprintf(buffer + pos, length-pos, "%s", pattern->ufrac_s);

	for (unsigned int i = 0; i < over; i++) {
		if (pos >  length)
			return construct_error(ERR_MAJ_OVERFLOW, ERR_MIN_BUFFER_OVERFLOW, 0);
		err = us_part_totext_length(p_over[i], &len, false);
		if (err.composite)
			return reconstruct_error(err, 0);

		char buff[length+1];
		err = us_part_totext(p_over[i], len, buff, false);
		if (err.composite)
			return reconstruct_error(err, 0);
		pos += snprintf(buffer + pos, length-pos, "%s", buff);
	}
	
	if (under)
		pos += snprintf(buffer + pos, length-pos, "%s", pattern->ufrac_m);
	
	for (unsigned int i = 0; i < under; i++) {
		if (pos >  length)
			return construct_error(ERR_MAJ_OVERFLOW, ERR_MIN_BUFFER_OVERFLOW, 0);
		err = us_part_totext_length(p_under[i], &len, true);
		if (err.composite)
			return reconstruct_error(err, 0);

		char buff[length+1];
		err = us_part_totext(p_under[i], len, buff, true);
		if (err.composite)
			return reconstruct_error(err, 0);
		pos += snprintf(buffer + pos, length-pos, "%s", buff);
	}
	
	if (under)
		pos += snprintf(buffer + pos, length-pos, "%s", pattern->ufrac_e);
	
	pos += snprintf(buffer + pos, length-pos, "%s", pattern->unit_e);

	return err;
}

err_t _us_base_parts_copy_reduce(unsigned int length, const us_part_s * parts[], unsigned int * rlength, us_part_s rparts[])
{
	err_t err = {0};
	const us_part_s * oparts[length];
	bool join;
	unsigned int  i, r;

	if (length == 1) {
		err = us_part_init(&rparts[0], parts[0]->pattern);
		if (err.composite)
			goto init_error;
		err = us_part_copy(&rparts[0], parts[0]);
		if (err.composite)
			goto init_error;
		*rlength = 1;
	} else if (length > 1) {
		*rlength = 0;
		/* make a local sorted copy of parts array */
		for(unsigned int i = 0; i < length; i++) {
			oparts[i] = parts[i];
		}
		err = us_part_sort(length, oparts);
		if (err.composite)
			return reconstruct_error(err, 0);

		memset(rparts, 0, sizeof(us_part_s)*length);

		/* init counters */
		i = 0; r = 0;
		/* copy first part */
		err = us_part_init(&rparts[r], oparts[i]->pattern);
		if (err.composite)
			goto init_error;
		err = us_part_copy(&rparts[r], oparts[i]);
		if (err.composite)
			goto init_error;
		i++; r++;

		for (; i < length; i++) {
			/* compare */
			err = us_part_joinable(&rparts[r-1], oparts[i], &join);
			if (err.composite)
				goto compare_error;
			if (join) {
				/* join parts */
				mpq_t pow;
				mpq_init(pow);

				mpq_add(pow,  rparts[r-1].power, oparts[i]->power);

				if (mpq_cmp_si(pow, 0, 1) == 0) {
					mpq_clear(pow);

					us_part_clear(&rparts[r-1]);
					r--;
				} else {
					mpq_clear(pow);
					err = us_part_multiply(&rparts[r-1], oparts[i], &rparts[r-1]);
					if (err.composite)
						goto init_error;
				}
			} else {
				/* copy part */
				err = us_part_init(&rparts[r], oparts[i]->pattern);
				if (err.composite)
					goto init_error;
				err = us_part_copy(&rparts[r], oparts[i]);
				if (err.composite)
					goto init_error;
				r++;
			}
		}

	*rlength = r;
	}

	return err;

compare_error:
init_error:
	for (unsigned int i = 0; i < length; i++, (*rlength)++) {
		if (rparts[i].initialized)
			us_part_clear(&rparts[i]);
	}
	return reconstruct_error(err, 0);
}

err_t us_base_unit_set_parts(us_base_unit_t base, unsigned int length, const us_part_s * parts[])
{
	check_in_ptr(base, 0);
	check_in_ptr(parts, 0);
	check_bool_pfield(us_part_t, base, initialized, true, ERR_MIN_NOT_INITIALIZED, 0);

	err_t err = {0};
	unsigned int rlength;
	us_part_s rparts[length];

	err = _us_base_parts_copy_reduce(length, parts, &rlength, rparts);
	if (err.composite)
		return err;

	if (base->parts_set)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, 0);

	base->parts_length = rlength;
	checked_array_malloc(base->parts, us_part_s, rlength, err, 0, malloc_failed);
	for(unsigned int i = 0; i < rlength; i++) {
		us_part_init(&base->parts[i], rparts[i].pattern);
		us_part_copy(&base->parts[i], &rparts[i]);
	}


	for(unsigned int i = 0; i < rlength; i++) {
		us_part_clear(&rparts[i]);
	}

	if (base->parts_length)
		base->parts_set = true;
	else
		err = construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, BASE_UNIT_EMPTY);

	return err;
malloc_failed:
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


extern const us_text_pattern_t test_pattern;

#define PARTS 20

struct test_obj {
	us_prefix_t prefix[PARTS];
	us_atom_t atom[PARTS];
	mpq_t power[PARTS];
	us_part_t part[PARTS];
	us_base_unit_t base;
};

struct test_constants {
	const char * prefix_str;
	const char * prefix_txt;
	unsigned int prefix_base;
	signed int	 prefix_pow;
	const char * atom_str;
	const char * atom_txt;
	signed int	 power_num;
	unsigned int power_den;
	const char * base_str;
	const char * base_txt;
};

static const struct test_constants constants[PARTS] = {
	{"a", "a", 10, -6, "A", "A", 1, 1,
		"", "\\unit{\\sipart{a A}}"},
	{"b", "b", 10, -3, "B", "B", -1, 2,
		"", "\\unit{\\frac{\\sipart{a A}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"c", "c", 1,		0, "C", "C", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"d", "d", 10,	3, "D", "D", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"e", "e", 10,	6, "E", "E", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}\\sipart{e E}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"f", "f", 10, -6, "F", "F", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"g", "g", 10, -3, "G", "G", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"h", "h", 10,	1, "I", "H", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"i", "i", 10,	3, "J", "I", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"j", "j", 10,	6, "K", "J", 1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"k", "k", 10, 1, "A", "A", 1, 2,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{1}{2}}}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"k", "k", 10, 1, "A", "A", 1, 3,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{5}{6}}}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"k", "k", 10, 1, "A", "A", 1, 4,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}{\\sipart{{(b B)}^{\\nicefrac{1}{2}}}}}"},
	{"b", "b", 10, -3, "B", "B", 1, 2,
		"", "\\unit{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{c C}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}"},
	{"c", "c", 1, 0, "C", "C", -1, 1,
		"", "\\unit{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{d D}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}"},
	{"d", "d", 10, 3, "D", "D", -1, 1,
		"", "\\unit{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{e E}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}"},
	{"e", "e", 10, 6, "E", "E", -1, 1,
		"", "\\unit{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{f F}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}"},
	{"f", "f", 10, -6, "F", "F", -1, 1,
		"", "\\unit{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}"},
	{"a", "a", 10, 1, "A", "A", -1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}{\\sipart{a A}}}"},
	{"b", "b", 10, 1, "B", "B", -1, 1,
		"", "\\unit{\\frac{\\sipart{a A}\\sipart{{(k A)}^{\\nicefrac{13}{12}}}\\sipart{g G}\\sipart{h H}\\sipart{i I}\\sipart{j J}}{\\sipart{a A}\\sipart{b B}}}"},
};

BT_SUITE_DEF(us_base_unit, "us_base_unit tests");

BT_SUITE_SETUP_DEF(us_base_unit)
{
	err_t err = {0};
	struct test_obj * test = malloc(sizeof(struct test_obj));
	if (!test)
		return BT_RESULT_FAIL;

	memset(test, 0, sizeof(struct test_obj));

	for (unsigned int i = 0; i < PARTS; i++) {
		mpq_init(test->power[i]); 
	}

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_init(test->part[i], &test_pattern);
		bt_assert_err_equal_i(err, 0, 0, 0);
	}

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_prefix_set(test->prefix[i], constants[i].prefix_str, constants[i].prefix_txt, constants[i].prefix_base, constants[i].prefix_pow);
		bt_assert_err_equal_i(err, 0, 0, 0);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_atom_set(test->atom[i], constants[i].atom_str, constants[i].atom_txt);
		bt_assert_err_equal_i(err, 0, 0, 0);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		mpq_set_si(test->power[i], constants[i].power_num, constants[i].power_den);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_set(test->part[i], test->prefix[i], test->atom[i], test->power[i]);
		bt_assert_err_equal_i(err, 0, 0, 0);
	}

	err = us_base_unit_init(test->base);
	bt_assert_err_equal_i(err, 0, 0, 0);

	*object = test;
	
	return BT_RESULT_OK;
}

BT_TEST_DEF(us_base_unit, set_parts, "set parts")
{
	err_t err = {0};
	struct test_obj * test = object;

	{
		const us_part_s * list[3] = { test->part[10], test->part[11], test->part[12]};
		mpq_t two;

		mpq_init(two);

		err = us_base_unit_set_parts(test->base, 3, list);
		bt_assert_err_equal_i(err, 0, 0, 0);

		bt_assert_int_equal(test->base->parts_length, 1);

		mpq_add(two, two, list[0]->power);
		mpq_add(two, two, list[1]->power);
		mpq_add(two, two, list[2]->power);

		bt_assert_int_equal(mpq_cmp(two, test->base->parts[0].power), 0);

		mpq_clear(two);
	}

	err = us_base_unit_clear(test->base);
	bt_assert_err_equal_i(err, 0, 0, 0);
	err = us_base_unit_init(test->base);
	bt_assert_err_equal_i(err, 0, 0, 0);

	{
		const us_part_s * list[3] = { test->part[2], test->part[0], test->part[1]};
		const us_part_s * olist[3] = { test->part[0], test->part[1], test->part[2]};
		bool res;
		
		err = us_base_unit_set_parts(test->base, 3, list);
		bt_assert_err_equal_i(err, 0, 0, 0);

		bt_assert_int_equal(test->base->parts_length, 3);

		for (unsigned int i = 0; i < 3; i++) {
			err = us_part_equal(olist[i], &test->base->parts[i], &res);
			bt_assert_bool_equal(res, true);
		}
	}

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_base_unit, totext, "totext")
{
	err_t err = {0};
	struct test_obj * test = object;

	for (unsigned int j = 1; j <= PARTS; j++) {
		const us_part_s * parts[j];

		err = us_base_unit_clear(test->base);
		bt_assert_err_equal_i(err, 0, 0, 0);
		err = us_base_unit_init(test->base);
		bt_assert_err_equal_i(err, 0, 0, 0);

		for (unsigned int i = 0; i < j; i++) {
			parts[i] = test->part[i];
			{
			/*size_t length = 0;
			err = us_part_totext_length(parts[i], &length, false);
			bt_assert_err_equal_i(err, 0, 0, 0);

			char buffer[length+1];
			err = us_part_totext(parts[i], length, buffer, false);
			bt_log("joining %s\n", buffer);*/
		}

		}

		err = us_base_unit_set_parts(test->base, j, parts);
		bt_assert_err_equal_i(err, 0, 0, 0);

		size_t length = 0;
		err = us_base_unit_totext_length(test->base, &length);
		bt_assert_err_equal_i(err, 0, 0, 0);

		char buffer[length+1];
		err = us_base_unit_totext(test->base, length, buffer);
		bt_assert_err_equal_i(err, 0, 0, 0);

		bt_log("joined %u parts , got %s\n", j, buffer);

		bt_assert_str_equal(buffer, constants[j-1].base_txt);
	}
	
	return BT_RESULT_OK;
}

BT_SUITE_TEARDOWN_DEF(us_base_unit)
{
	err_t err = {0};
	struct test_obj * test = *object;
	
	for (unsigned int i = 0; i < PARTS; i++) {
		mpq_clear(test->power[i]);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_clear(test->part[i]);
		bt_assert_err_equal_i(err, 0, 0, 0);
	}

	err = us_base_unit_clear(test->base);
	bt_assert_err_equal_i(err, 0, 0, 0);

	free(test);

	return BT_RESULT_OK;
}

#endif
