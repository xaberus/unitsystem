

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	PART_NILL_POWER,
	PART_PREFIX_TOSTRING_FAILED,
	PART_ATOM_TOSTRING_FAILED,
	PART_NOT_SAME_PREFIX,
	PART_NOT_SAME_ATOM,
	PART_NOT_SAME_PATTERN,

};

err_t us_part_new(const us_prefix_t * prefix, const us_atom_t * atom, const mpq_t power, const us_part_text_pattern_t * pattern, us_part_t ** ret)
{
	check_in_ptr(prefix, 0);
	check_in_ptr(atom, 0);
	check_in_ptr(power, 0);
	check_in_ptr(pattern, 0);
	check_out_ptr(ret, 0);

	*ret = NULL;
	us_part_t * part = NULL;
	err_t err = {0};

	if (mpq_cmp_si(power, 0, 1) == 0)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NILL_POWER);

	checked_malloc(part, us_part_t, err, 0, malloc_failed);
	mpq_init(part->power);

	part->prefix = prefix;
	part->atom = atom;
	mpq_set(part->power, power);
	part->pattern = pattern;

	*ret = part;

	return err;

malloc_failed:
	return err;
}

err_t us_part_tostring_length(const us_part_t * part, size_t * length)
{
	check_in_ptr(part, 0);
	check_out_ptr(length, 0);
	
	err_t err = {0};

	*length = US_MAX_ATOM_SYMBOL_LENGTH
			+ US_MAX_PREFIX_SYMBOL_LENGTH + 2
			+ mpz_sizeinbase (mpq_numref(part->power), 10)+ mpz_sizeinbase (mpq_denref(part->power), 10) + 3;

	return err;
}
err_t us_part_tostring(const us_part_t * part, size_t length, char buffer[])
{
	check_in_ptr(part, 0);
	check_in_ptr(buffer, 0);

	char atm[US_MAX_ATOM_SYMBOL_LENGTH+1];
	char pfx[US_MAX_PREFIX_SYMBOL_LENGTH+1];
	char pow[mpz_sizeinbase (mpq_numref(part->power), 10)+ mpz_sizeinbase (mpq_denref(part->power), 10) + 3];

	err_t err = {0};

	err = us_prefix_tostring(part->prefix, sizeof(pfx)-1, pfx);
	if (err.composite)
		return reconstruct_error(err, PART_PREFIX_TOSTRING_FAILED);
	
	err = us_atom_tostring(part->atom, sizeof(atm)-1, atm);
	if (err.composite)
		return reconstruct_error(err, PART_ATOM_TOSTRING_FAILED);

	mpq_get_str(pow, 10, part->power);

	snprintf(buffer, length, "(<%s> %s)^%s", pfx, atm, pow);

	return err;
}

err_t us_part_totext_length(const us_part_t * part, size_t * length, bool invert)
{
	check_in_ptr(part, 0);
	check_out_ptr(length, 0);

	*length = 0;
	err_t err = {0};
	size_t l = 0;
	mpq_t power;
	
	l += US_MAX_ATOM_TEXT_LENGTH+1;
	l += US_MAX_PREFIX_TEXT_LENGTH+1;

	const us_part_text_pattern_t * pattern = part->pattern;
	bool apar = false;
	bool sign = false;
	bool frac = false;

	if (invert) {
		if (mpq_cmp_si(part->power, -1, 1) != 0)
			apar = true;
	} else {
		if (mpq_cmp_si(part->power, 1, 1) != 0)
			apar = true;
	}
	
	if (strlen(part->prefix->symbol) == 0 && apar)
		apar = false;
	
	if (mpq_cmp_si(part->power, 0,1) < 0)
		sign = true;
	if (mpz_cmp_si(mpq_numref(part->power), 1) != 0)
		frac = true;

	mpq_init(power);
	mpq_abs(power, part->power);

	l += mpz_sizeinbase (mpq_numref(power), 10) + 2;
	l += mpz_sizeinbase (mpq_denref(power), 10) + 2;

	mpq_clear(power);

	if (apar) {
		if (frac) {
			l += strlen(pattern->pow_s) + strlen(pattern->lpar) + strlen(pattern->prefix_sep) + strlen(pattern->rpar) + strlen(pattern->pow_m);
			l += strlen(pattern->frac_s) + strlen(sign ? pattern->sign : "") + strlen(pattern->frac_m);
			l += strlen(pattern->frac_e) + strlen(pattern->pow_e);
		} else {
			l += strlen(pattern->pow_s) + strlen(pattern->lpar) + strlen(pattern->prefix_sep) + strlen(pattern->rpar);
			l += strlen(pattern->pow_m) + strlen(pattern->lbrace) + strlen(sign ? pattern->sign : "") + strlen(pattern->rbrace);
			l += strlen(pattern->pow_e);
		}
	} else {
				if (frac) {
				 l += strlen(pattern->pow_s) + strlen(pattern->prefix_sep) + strlen(pattern->pow_m) + strlen(pattern->frac_s);
				 l += strlen(sign ? pattern->sign : "") + strlen(pattern->frac_m) + strlen(pattern->frac_e) + strlen(pattern->pow_e);
		} else {
			l += strlen(pattern->pow_s) + strlen(pattern->prefix_sep) + strlen(pattern->pow_m) + strlen(pattern->lbrace);
			l += strlen(sign ? pattern->sign : "") + strlen(pattern->rbrace) + strlen(pattern->pow_e);
		}
	}

	*length = l;

	return err;
}

static void _us_part_totext_par_frac(const us_part_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], char pow_den[], size_t length, char buffer[])
{
	snprintf(buffer, length,
			"%s" /* { */
				"%s" /* ( */ "%s" /* prefix */ "%s" /* separator */ "%s" /* atom */ "%s" /* ) */
			"%s" /* }^{ */
				"%s" /* \frac{ */ "%s" /* sign */ "%s" /* exp num */ "%s" /* }{ */ "%s" /* exp den */ "%s" /* } */
			"%s", /* } */
			pattern->pow_s,
				pattern->lpar, pfx, pattern->prefix_sep, atm, pattern->rpar,
			pattern->pow_m,
				pattern->frac_s,
					sign ? pattern->sign : "",
					pow_num,
				pattern->frac_m,
					pow_den,
				pattern->frac_e,
			pattern->pow_e);
}

static void _us_part_totext_par(const us_part_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], size_t length, char buffer[])
{
	snprintf(buffer, length,
			"%s" /* { */
				"%s" /* ( */ "%s" /* prefix */ "%s" /* separator */ "%s" /* atom */ "%s" /* ) */
			"%s" /* }^{ */ 
				"%s" /* { */ "%s" /* sign */ "%s" /* exp num */ "%s" /* } */
			"%s", /* } */
			pattern->pow_s,
				pattern->lpar, pfx, pattern->prefix_sep, atm, pattern->rpar,
			pattern->pow_m,
				pattern->lbrace,
					sign ? pattern->sign : "",
					pow_num,
				pattern->rbrace,
			pattern->pow_e);
}

static void _us_part_totext_frac(const us_part_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], char pow_den[], size_t length, char buffer[])
{
	snprintf(buffer, length,
			"%s" /* { */
				"%s" /* prefix */ "%s" /* separator */ "%s" /* atom */
			"%s" /* }^{ */
				"%s" /* \frac{ */ "%s" /* sign */ "%s" /* exp num */ "%s" /* }{ */ "%s" /* exp den */ "%s" /* } */
			"%s", /* } */
			pattern->pow_s,
				pfx, pattern->prefix_sep, atm,
			pattern->pow_m,
				pattern->frac_s,
					sign ? pattern->sign : "",
					pow_num,
				pattern->frac_m,
					pow_den,
				pattern->frac_e,
			pattern->pow_e);
}

static void _us_part_totext(const us_part_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], size_t length, char buffer[])
{
	snprintf(buffer, length,
			"%s" /* { */
				"%s" /* prefix */ "%s" /* separator */ "%s" /* atom */
			"%s" /* }^{ */ 
				"%s" /* { */ "%s" /* sign */ "%s" /* exp num */ "%s" /* } */
			"%s", /* } */
			pattern->pow_s,
				pfx, pattern->prefix_sep, atm,
			pattern->pow_m,
				pattern->lbrace,
					sign ? pattern->sign : "",
					pow_num,
				pattern->rbrace,
			pattern->pow_e);
}

err_t us_part_totext(const us_part_t * part, size_t length, char buffer[], bool invert)
{
	check_in_ptr(part, 0);
	check_in_ptr(buffer, 0);

	mpq_t power;
	err_t err = {0};
	char atm[US_MAX_ATOM_TEXT_LENGTH+1];
	char pfx[US_MAX_PREFIX_TEXT_LENGTH+1];
	const us_part_text_pattern_t * pattern = part->pattern;
	bool apar = false;
	bool sign = false;
	bool frac = false;

	if (invert) {
		if (mpq_cmp_si(part->power, -1, 1) != 0)
			apar = true;
	} else {
		if (mpq_cmp_si(part->power, 1, 1) != 0)
			apar = true;
	}
	
	if (strlen(part->prefix->symbol) == 0 && apar)
		apar = false;
	
	err = us_prefix_totext(part->prefix, sizeof(pfx)-1, pfx);
	if (err.composite)
		return reconstruct_error(err, PART_PREFIX_TOSTRING_FAILED);
	
	err = us_atom_totext(part->atom, sizeof(atm)-1, atm);
	if (err.composite)
		return reconstruct_error(err, PART_ATOM_TOSTRING_FAILED);
	
	if (mpq_cmp_si(part->power, 0,1) < 0)
		sign = true;
	if (mpz_cmp_si(mpq_denref(part->power), 1) != 0)
		frac = true;


	mpq_init(power);
	mpq_abs(power, part->power);

	char pow_num[mpz_sizeinbase (mpq_numref(power), 10) + 2];
	char pow_den[mpz_sizeinbase (mpq_denref(power), 10) + 2];

	mpz_get_str(pow_num, 10, mpq_numref(power));
	mpz_get_str(pow_den, 10, mpq_denref(power));

	mpq_clear(power);

	if (apar) {
		if (frac) {
			_us_part_totext_par_frac(pattern, pfx, atm, sign, pow_num, pow_den, length, buffer);
		} else {
			_us_part_totext_par(pattern, pfx, atm, sign, pow_num, length, buffer);
		}
	} else {
		if (frac) {
			_us_part_totext_frac(pattern, pfx, atm, sign, pow_num, pow_den, length, buffer);
		} else {
			_us_part_totext(pattern, pfx, atm, sign, pow_num, length, buffer);
		}
	}

	return err;
}

err_t us_part_power(const us_part_t * part, const mpq_t power, us_part_t ** result)
{
	check_in_ptr(part, 0);
	check_in_ptr(power, 0);
	check_out_ptr(result, 0);

	err_t err = {0};
	mpq_t tmp;

	mpq_init(tmp);
	mpq_mul(tmp, part->power, power);

	err = us_part_new(part->prefix, part->atom, tmp, part->pattern, result);
	
	mpq_clear(tmp);
	
	return err;
}

err_t us_part_multiply(const us_part_t * left, const us_part_t * right, us_part_t ** result)
{
	check_in_ptr(left, 0);
	check_in_ptr(right, 0);
	check_out_ptr(result, 0);

	err_t err = {0};
	mpq_t tmp;

	if (left->prefix != right->prefix)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NOT_SAME_PREFIX);
	
	if (left->atom != right->atom)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NOT_SAME_ATOM);
	
	if (left->pattern != right->pattern)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NOT_SAME_PATTERN);

	mpq_init(tmp);
	mpq_add(tmp, left->power, right->power);

	err = us_part_new(left->prefix, left->atom, tmp, left->pattern, result);
	
	mpq_clear(tmp);

	return err;
}

err_t us_part_delete(us_part_t ** ret)
{
	check_out_ptr(ret, 0);
	check_in_ptr(*ret, 0);

	mpq_clear((*ret)->power);

	err_t err = {0};

	free(*ret);
	*ret = NULL;

	return err;
}


#ifdef TEST

const us_part_text_pattern_t test_patten = {
	.lbrace = "{",
	.rbrace = "}",
	.lpar = "(",
	.rpar = ")",
	.frac_s = "\\nicefrac{",
	.frac_m = "}{",
	.frac_e = "}",
	.pow_s = "{",
	.pow_m = "}^{",
	.pow_e = "}",
	.neg_s = "- {",
	.neg_e = "}",
	.part_s = "\\sipart{",
	.part_e = "}",
	.prefix_sep = " ",
	.sign = "-",
};

BT_SUITE_DEF(us_part,"us_part tests");

BT_SUITE_SETUP_DEF(us_part)
{
	err_t err = {0};
	us_prefix_t * prefix = NULL;
	us_atom_t * atom = NULL;
	mpq_t power;
	
	us_part_t * part = NULL;

	err = us_prefix_new("k", "\\siprefix{k}", 10, 3, &prefix);
	bt_assert_ptr_not_equal(prefix, NULL);
	bt_assert_int_equal(err.composite, 0);
	
	err = us_atom_new("g", "\\siatom{g}", &atom);
	bt_assert_ptr_not_equal(atom, NULL);
	bt_assert_int_equal(err.composite, 0);

	mpq_init(power);
	mpq_set_si(power, -1, 2);


	err = us_part_new(prefix, atom, power, &test_patten, &part);
	bt_assert_ptr_not_equal(part, NULL);
	bt_assert_int_equal(err.composite, 0);

	mpq_clear(power);

	*object = part;

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, tostring, "tostring")
{
	us_part_t * part = object;
	err_t err = {0};

	size_t length = 0;
	err = us_part_tostring_length(part, &length);
	bt_assert_int_equal(err.composite, 0);

	char buffer[length+1];
	err = us_part_tostring(part, length, buffer);
	bt_assert_int_equal(err.composite, 0);

	bt_assert_str_equal(buffer, "(<k> g)^-1/2");

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, tostext, "totext")
{
	us_part_t * part = object;
	err_t err = {0};

	size_t length = 0;
	err = us_part_totext_length(part, &length, false);
	bt_assert_int_equal(err.composite, 0);

	char buffer[length+1];
	err = us_part_totext(part, length, buffer, false);
	bt_assert_int_equal(err.composite, 0);

	bt_assert_str_equal(buffer, "{(\\siprefix{k} \\siatom{g})}^{\\nicefrac{-1}{2}}");

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, power, "power")
{
	us_part_t * part = object;
	us_part_t * part_sq = NULL;
	err_t err = {0};
	mpq_t two;

	mpq_init(two);
	mpq_set_si(two, 2, 1);

	err = us_part_power(part, two, &part_sq);
	
	mpq_clear(two);
	
	bt_assert_int_equal(err.composite, 0);


	size_t length = 0;
	err = us_part_totext_length(part_sq, &length, false);
	bt_assert_int_equal(err.composite, 0);

	char buffer[length+1];
	err = us_part_totext(part_sq, length, buffer, false);
	bt_assert_int_equal(err.composite, 0);

	bt_assert_str_equal(buffer, "{(\\siprefix{k} \\siatom{g})}^{{-1}}");
	
	err = us_part_delete(&part_sq);

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, multiply, "multiply")
{
	us_part_t * part = object;
	us_part_t * part_sq = NULL;
	err_t err = {0};

	err = us_part_multiply(part, part, &part_sq);
	bt_assert_int_equal(err.composite, 0);


	size_t length = 0;
	err = us_part_totext_length(part_sq, &length, false);
	bt_assert_int_equal(err.composite, 0);

	char buffer[length+1];
	err = us_part_totext(part_sq, length, buffer, false);
	bt_assert_int_equal(err.composite, 0);

	bt_assert_str_equal(buffer, "{(\\siprefix{k} \\siatom{g})}^{{-1}}");
	
	err = us_part_delete(&part_sq);

	return BT_RESULT_OK;
}


BT_SUITE_TEARDOWN_DEF(us_part)
{
	err_t err;
	us_part_t * part = *object;
	us_prefix_t * prefix = (us_prefix_t *)part->prefix;
	us_atom_t * atom = (us_atom_t *)part->atom;


	err = us_part_delete(&part);
	bt_assert_int_equal(err.composite, 0);
	
	err = us_prefix_delete(&prefix);
	bt_assert_int_equal(err.composite, 0);
	
	err = us_atom_delete(&atom);
	bt_assert_int_equal(err.composite, 0);

	return BT_RESULT_OK;
}
#endif
