

#include "unitsystem.h"

#include <stdlib.h>
#include <string.h>

enum {
	PART_NILL_POWER,
	PART_PREFIX_TOSTRING_FAILED,
	PART_ATOM_TOSTRING_FAILED,
	PART_NOT_SAME_PREFIX,
	PART_NOT_SAME_ATOM,
};

err_t us_part_init(us_part_t part)
{
	check_in_ptr(part, 0);

	err_t err = {0};

	memset(part, 0, sizeof(us_part_s));

	mpq_init(part->power);
	part->initialized = true;

	return err;
}

err_t us_part_clear(us_part_t ret)
{
	check_in_ptr(ret, 0);

	mpq_clear(ret->power);
	memset(ret, 0, sizeof(us_part_s));

	err_t err = {0};

	return err;
}

err_t us_part_set(us_part_t part, const us_prefix_t prefix, const us_atom_t atom, const mpq_t power)
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, initialized, true, ERR_MIN_NOT_INITIALIZED, 0);
	check_in_ptr(prefix, 0);
	check_in_ptr(atom, 0);
	check_in_ptr(power, 0);

	err_t err = {0};

	if (mpq_cmp_si(power, 0, 1) == 0)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NILL_POWER);

	part->prefix = prefix;
	part->atom = atom;
	mpq_set(part->power, power);
	mpq_canonicalize(part->power);
	part->set = true;

	return err;
}

err_t us_part_copy(us_part_t part, const us_part_t source)
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, initialized, true, ERR_MIN_NOT_INITIALIZED, 0);
	check_in_ptr(source, 0);
	check_bool_pfield(us_part_t, source, set, true, ERR_MIN_NOT_SET, 0);

	err_t err = {0};

	part->prefix = source->prefix;
	part->atom = source->atom;
	mpq_set(part->power, source->power);
	part->set = true;

	return err;
}


err_t us_part_tostring_length(const us_part_t part, size_t * length)
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, set, true, ERR_MIN_NOT_SET, 0);
	check_out_ptr(length, 0);
	
	err_t err = {0};

	*length = US_MAX_ATOM_SYMBOL_LENGTH
			+ US_MAX_PREFIX_SYMBOL_LENGTH + 2
			+ mpz_sizeinbase (mpq_numref(part->power), 10)+ mpz_sizeinbase (mpq_denref(part->power), 10) + 3;

	return err;
}
err_t us_part_tostring(const us_part_t part, size_t length, char buffer[])
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, set, true, ERR_MIN_NOT_SET, 0);
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

err_t us_part_totext_length(const us_part_t part, const us_text_pattern_t * pattern, size_t * length, bool invert)
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, set, true, ERR_MIN_NOT_SET, 0);
	check_out_ptr(length, 0);

	*length = 0;
	err_t err = {0};
	size_t l = 0;
	mpq_t power;
	
	l += US_MAX_ATOM_TEXT_LENGTH+1;
	l += US_MAX_PREFIX_TEXT_LENGTH+1;

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
	
	if (invert) {
		if (mpq_cmp_si(part->power, 0,1) < 0)
			sign = false;
	} else {
		if (mpq_cmp_si(part->power, 0,1) < 0)
			sign = true;
	}
	if (mpz_cmp_si(mpq_numref(part->power), 1) != 0)
		frac = true;

	mpq_init(power);
	mpq_abs(power, part->power);

	l += mpz_sizeinbase (mpq_numref(power), 10) + 2;
	l += mpz_sizeinbase (mpq_denref(power), 10) + 2;

	mpq_clear(power);

	if (apar) {
		if (frac) {
			l += strlen(pattern->part_s) + strlen(pattern->pow_s) + strlen(pattern->lpar) + strlen(pattern->prefix_sep) + strlen(pattern->rpar) + strlen(pattern->pow_m);
			l += strlen(pattern->frac_s) + strlen(sign ? pattern->sign : "") + strlen(pattern->frac_m);
			l += strlen(pattern->frac_e) + strlen(pattern->pow_e) + strlen(pattern->part_s);
		} else {
			l += strlen(pattern->part_s) + strlen(pattern->pow_s) + strlen(pattern->lpar) + strlen(pattern->prefix_sep) + strlen(pattern->rpar);
			l += strlen(pattern->pow_m) + strlen(pattern->lbrace) + strlen(sign ? pattern->sign : "") + strlen(pattern->rbrace);
			l += strlen(pattern->pow_e) + strlen(pattern->part_s);
		}
	} else {
				if (frac) {
				 l += strlen(pattern->part_s) + strlen(pattern->pow_s) + strlen(pattern->prefix_sep) + strlen(pattern->pow_m) + strlen(pattern->frac_s);
				 l += strlen(sign ? pattern->sign : "") + strlen(pattern->frac_m) + strlen(pattern->frac_e) + strlen(pattern->pow_e) + strlen(pattern->part_s);
		} else {
			l += strlen(pattern->part_s) + strlen(pattern->pow_s) + strlen(pattern->prefix_sep) + strlen(pattern->pow_m) + strlen(pattern->lbrace);
			l += strlen(sign ? pattern->sign : "") + strlen(pattern->rbrace) + strlen(pattern->pow_e) + strlen(pattern->part_s);
		}
	}

	*length = l;

	return err;
}

static void _us_part_totext_par_frac(const us_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], char pow_den[], size_t length, char buffer[])
{
	snprintf(buffer, length,
			"%s" /* \sipart{ */
			"%s" /* { */
				"%s" /* ( */ "%s" /* prefix */ "%s" /* separator */ "%s" /* atom */ "%s" /* ) */
			"%s" /* }^{ */
				"%s" /* \frac{ */ "%s" /* sign */ "%s" /* exp num */ "%s" /* }{ */ "%s" /* exp den */ "%s" /* } */
			"%s" /* } */
			"%s", /* } */
			pattern->part_s,
			pattern->pow_s,
				pattern->lpar, pfx, pattern->prefix_sep, atm, pattern->rpar,
			pattern->pow_m,
				pattern->frac_s,
					sign ? pattern->sign : "",
					pow_num,
				pattern->frac_m,
					pow_den,
				pattern->frac_e,
			pattern->pow_e,
			pattern->part_e);
}

static void _us_part_totext_par(const us_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], size_t length, char buffer[], bool pow)
{
	if (pow)
		snprintf(buffer, length,
				"%s" /* \sipart{ */
				"%s" /* { */
					"%s" /* ( */ "%s" /* prefix */ "%s" /* separator */ "%s" /* atom */ "%s" /* ) */
				"%s" /* }^{ */ 
					"%s" /* { */ "%s" /* sign */ "%s" /* exp num */ "%s" /* } */
				"%s" /* } */
				"%s", /* } */
				pattern->part_s,
				pattern->pow_s,
					pattern->lpar, pfx, pattern->prefix_sep, atm, pattern->rpar,
				pattern->pow_m,
					pattern->lbrace,
						sign ? pattern->sign : "",
						pow_num,
					pattern->rbrace,
				pattern->pow_e,
				pattern->part_e);
	else
		snprintf(buffer, length,
				"%s" /* \sipart{ */
				"%s" /* ( */ "%s" /* prefix */ "%s" /* separator */ "%s" /* atom */ "%s" /* ) */
				"%s", /* } */
				pattern->part_s,
				pattern->lpar, pfx, pattern->prefix_sep, atm, pattern->rpar,
				pattern->part_e);

}

static void _us_part_totext_frac(const us_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], char pow_den[], size_t length, char buffer[])
{
	snprintf(buffer, length,
			"%s" /* \sipart{ */
			"%s" /* { */
				"%s" /* prefix */ "%s" /* separator */ "%s" /* atom */
			"%s" /* }^{ */
				"%s" /* \frac{ */ "%s" /* sign */ "%s" /* exp num */ "%s" /* }{ */ "%s" /* exp den */ "%s" /* } */
			"%s"  /* } */
			"%s", /* } */
			pattern->part_s,
			pattern->pow_s,
				pfx, pattern->prefix_sep, atm,
			pattern->pow_m,
				pattern->frac_s,
					sign ? pattern->sign : "",
					pow_num,
				pattern->frac_m,
					pow_den,
				pattern->frac_e,
			pattern->pow_e,
			pattern->part_e);
}

static void _us_part_totext(const us_text_pattern_t * pattern, char pfx[], char atm[], bool sign, char pow_num[], size_t length, char buffer[], bool pow)
{
	if (pow)
		snprintf(buffer, length,
				"%s" /* \sipart{ */
				"%s" /* { */
					"%s" /* prefix */ "%s" /* separator */ "%s" /* atom */
				"%s" /* }^{ */ 
					"%s" /* { */ "%s" /* sign */ "%s" /* exp num */ "%s" /* } */
				"%s" /* } */
				"%s", /* } */
				pattern->part_s,
				pattern->pow_s,
					pfx, pattern->prefix_sep, atm,
				pattern->pow_m,
					pattern->lbrace,
						sign ? pattern->sign : "",
						pow_num,
					pattern->rbrace,
				pattern->pow_e,
				pattern->part_e);
	else
		snprintf(buffer, length,
				"%s" /* \sipart{ */
				"%s" /* prefix */ "%s" /* separator */ "%s" /* atom */
				"%s", /* } */
				pattern->part_s,
				pfx, pattern->prefix_sep, atm,
				pattern->part_e);
}

err_t us_part_totext(const us_part_t part, const us_text_pattern_t * pattern, size_t length, char buffer[], bool invert)
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, set, true, ERR_MIN_NOT_SET, 0);
	check_in_ptr(buffer, 0);

	mpq_t power;
	err_t err = {0};
	char atm[US_MAX_ATOM_TEXT_LENGTH+1];
	char pfx[US_MAX_PREFIX_TEXT_LENGTH+1];
	bool apar = false;
	bool sign = false;
	bool frac = false;
	bool pow = true;

	if (invert) {
		if (mpq_cmp_si(part->power, -1, 1) != 0)
			apar = true;
		else
			pow = false;
	} else {
		if (mpq_cmp_si(part->power, 1, 1) != 0)
			apar = true;
		else
			pow = false;
	}
	
	if (strlen(part->prefix->symbol) == 0 && apar)
		apar = false;
	
	err = us_prefix_totext(part->prefix, sizeof(pfx)-1, pfx);
	if (err.composite)
		return reconstruct_error(err, PART_PREFIX_TOSTRING_FAILED);
	
	err = us_atom_totext(part->atom, sizeof(atm)-1, atm);
	if (err.composite)
		return reconstruct_error(err, PART_ATOM_TOSTRING_FAILED);
	
	if (invert) {
		if (mpq_cmp_si(part->power, 0,1) < 0)
			sign = false;
	} else {
		if (mpq_cmp_si(part->power, 0,1) < 0)
			sign = true;
	}

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
			_us_part_totext_par(pattern, pfx, atm, sign, pow_num, length, buffer, pow);
		}
	} else {
		if (frac) {
			_us_part_totext_frac(pattern, pfx, atm, sign, pow_num, pow_den, length, buffer);
		} else {
			_us_part_totext(pattern, pfx, atm, sign, pow_num, length, buffer, pow);
		}
	}

	return err;
}

err_t us_part_power(const us_part_t part, const mpq_t power, us_part_t result)
{
	check_in_ptr(part, 0);
	check_bool_pfield(us_part_t, part, set, true, ERR_MIN_NOT_SET, 0);
	check_in_ptr(power, 0);
	check_in_ptr(result, 0);
	check_bool_pfield(us_part_t, result, initialized, true, ERR_MIN_NOT_INITIALIZED, 0);
	
	err_t err = {0};
	mpq_t tmp;

	mpq_init(tmp);
	mpq_mul(tmp, part->power, power);

	err = us_part_set(result, part->prefix, part->atom, tmp);
	
	mpq_clear(tmp);
	
	return err;
}

err_t us_part_multiply(const us_part_t left, const us_part_t right, us_part_t result)
{
	check_in_ptr(left, 0);
	check_bool_pfield(us_part_t, left, set, true, ERR_MIN_NOT_SET, 0);
	check_in_ptr(right, 0);
	check_bool_pfield(us_part_t, right, set, true, ERR_MIN_NOT_SET, 0);
	check_out_ptr(result, 0);
	check_bool_pfield(us_part_t, result, initialized, true, ERR_MIN_NOT_INITIALIZED, 0);
	
	err_t err = {0};
	mpq_t tmp;
	bool res;

	err = us_prefix_equal(left->prefix, right->prefix, &res);
	if (err.composite)
		return reconstruct_error(err, 0);
	else if(!res)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NOT_SAME_PREFIX);
	
	err = us_atom_equal(left->atom, right->atom, &res);
	if (err.composite)
		return reconstruct_error(err, 0);
	else if(!res)
		return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, PART_NOT_SAME_ATOM);
	
	mpq_init(tmp);
	mpq_add(tmp, left->power, right->power);

	err = us_part_set(result, left->prefix, left->atom, tmp);
	
	mpq_clear(tmp);

	return err;
}

#include <assert.h>

static int _us_part_cmp(const void * va, const void * vb)
{
	const us_part_s * a = *((us_part_s **)va), * b= *((us_part_s **)vb);
	int i = 0;

	i = strncmp(a->atom->symbol, b->atom->symbol, US_MAX_PREFIX_SYMBOL_LENGTH);
	if (i != 0)
		goto end;
	
	if (a->prefix->base == b->prefix->base) {
		if (a->prefix->power == b->prefix->power)
			i = 0;
		else if (a->prefix->power < b->prefix->power)
			i = -1;
		else
			i = 1;
	} else {
		mpf_t ta, tb;

		mpf_init_set_ui(ta, a->prefix->base);
		if (a->prefix->power < 0) {
			mpf_t t;
			mpf_init_set_ui(t, 1);
			mpf_div(ta, t, ta);
			mpf_clear(t);
		}
		mpf_init_set_ui(tb, b->prefix->base);
		if (b->prefix->power < 0) {
			mpf_t t;
			mpf_init_set_ui(t, 1);
			mpf_div(tb, t, tb);
			mpf_clear(t);
		}
		i = mpf_cmp(ta, tb);
		mpf_clear(ta);
		mpf_clear(tb);
	}
	
	if (i != 0)
		goto end;
	
	i = mpq_cmp(a->power, b->power);
	if (i != 0)
		goto end;

end:
	return i;
}

API err_t us_part_sort(unsigned int length, const us_part_s * parts[])
{
	err_t err = {0};

	if (length < 1)
		return err;
	check_in_ptr(parts[0], 0);

	for (unsigned int i = 1; i < length; i++) {
		check_in_ptr(parts[i], 0);
		check_bool_pfield(us_part_t, parts[i], set, true, ERR_MIN_NOT_SET, 0);
	}

	qsort(parts, length, sizeof(us_part_s *), _us_part_cmp);

	return err;
}

err_t us_part_joinable(const us_part_t left, const us_part_t right, bool * result)
{
	check_in_ptr(left, 0);
	check_in_ptr(right, 0);
	check_out_ptr(result, 0);
	check_bool_pfield(us_part_t, left, set, true, ERR_MIN_NOT_SET, 0);
	check_bool_pfield(us_part_t, right, set, true, ERR_MIN_NOT_SET, 0);
	
	err_t err = {0};
	bool prefeq;
	bool atomeq;

	*result = false;

	err = us_prefix_equal(left->prefix, right->prefix, &prefeq);
	if (err.composite)
		return reconstruct_error(err, 0);
	err = us_atom_equal(left->atom, right->atom, &atomeq);
	if (err.composite)
		return reconstruct_error(err, 0);

	*result = (prefeq && atomeq);

	return err;
}

err_t us_part_equal(const us_part_t left, const us_part_t right, bool * result)
{
	check_in_ptr(left, 0);
	check_in_ptr(right, 0);
	check_out_ptr(result, 0);
	check_bool_pfield(us_part_t, left, set, true, ERR_MIN_NOT_SET, 0);
	check_bool_pfield(us_part_t, right, set, true, ERR_MIN_NOT_SET, 0);
	
	err_t err = {0};
	bool prefeq;
	bool atomeq;

	*result = false;

	err = us_prefix_equal(left->prefix, right->prefix, &prefeq);
	if (err.composite)
		return reconstruct_error(err, 0);
	err = us_atom_equal(left->atom, right->atom, &atomeq);
	if (err.composite)
		return reconstruct_error(err, 0);

	*result = (prefeq && atomeq && mpq_cmp(right->power, left->power) == 0);

	return err;
}

#ifdef TEST

/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢╭────────╮▢▢▢╭────────╮▢▢▢╭────────╮▢▢▢╭────────╮▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢╰──╮  ╭──╯▢▢▢│ ╭──────╯▢▢▢│ ╭──────╯▢▢▢╰──╮  ╭──╯▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢│  │▢▢▢▢▢▢│ ╰────╮▢▢▢▢▢│ ╰──────╮▢▢▢▢▢▢│  │▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢│  │▢▢▢▢▢▢│ ╭────╯▢▢▢▢▢╰──────╮ │▢▢▢▢▢▢│  │▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢│  │▢▢▢▢▢▢│ ╰──────╮▢▢▢╭──────╯ │▢▢▢▢▢▢│  │▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢╰──╯▢▢▢▢▢▢╰────────╯▢▢▢╰────────╯▢▢▢▢▢▢╰──╯▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/
/*▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢▢*/

const us_text_pattern_t test_pattern = {
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
	.unit_s = "\\unit{",
	.unit_e = "}",
	.unitfrac_s = "\\frac{",
	.unitfrac_m = "}{",
	.unitfrac_e = "}",
};


#define PARTS 10

struct test_obj {
	us_prefix_t prefix[PARTS];
	us_atom_t atom[PARTS];
	mpq_t power[PARTS];
	us_part_t part[PARTS];
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
	const char * part_str;
	const char * part_txt;
};

static const struct test_constants constants[PARTS] = {
	{"",		"",									1,		0,		"g",		"\\siatom{g}",		1, 2, "(<> g)^1/2",			"\\sipart{{ \\siatom{g}}^{\\nicefrac{1}{2}}}"},
	{"",		"",									1,		0,		"g",		"\\siatom{g}",		1, 3, "(<> g)^1/3",			"\\sipart{{ \\siatom{g}}^{\\nicefrac{1}{3}}}"},
	{"",		"",									1,		0,		"g",		"\\siatom{g}",		1, 5, "(<> g)^1/5",			"\\sipart{{ \\siatom{g}}^{\\nicefrac{1}{5}}}"},
	{"mu",	"\\sipref{\\mu}",		10,		 -6,	"m",		"\\siatom{m}",		1, 1, "(<mu> m)^1",			"\\sipart{\\sipref{\\mu} \\siatom{m}}"},
	{"m",		"\\sipref{m}",			10,		-3,		"g",		"\\siatom{g}",		1, 2, "(<m> g)^1/2",		"\\sipart{{(\\sipref{m} \\siatom{g})}^{\\nicefrac{1}{2}}}"},
	{"",		"\\sipref{}",				1,		0,		"s",		"\\siatom{s}",		-1, 1, "(<> s)^-1",			"\\sipart{{\\sipref{} \\siatom{s}}^{{-1}}}"},
	{"k",		"\\sipref{k}",			10,		3,		"A",		"\\siatom{A}",		2, 1, "(<k> A)^2",			"\\sipart{{(\\sipref{k} \\siatom{A})}^{{2}}}"},
	{"M",		"\\sipref{M}",			10,		6,		"lux",	"\\siatom{lux}",	3, 1, "(<M> lux)^3",		"\\sipart{{(\\sipref{M} \\siatom{lux})}^{{3}}}"},
	{"",		"",									1,		0,		"g",		"\\siatom{g}",		1, 7, "(<> g)^1/7",			"\\sipart{{ \\siatom{g}}^{\\nicefrac{1}{7}}}"},
	{"",		"",									1,		0,		"g",		"\\siatom{g}",		1, 11, "(<> g)^1/11",		"\\sipart{{ \\siatom{g}}^{\\nicefrac{1}{11}}}"},
};

BT_SUITE_DEF(us_part,"us_part tests");

BT_SUITE_SETUP_DEF(us_part)
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
		err = us_part_init(test->part[i]);
		bt_assert_int_equal(err.composite, 0);
	}

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_prefix_set(test->prefix[i], constants[i].prefix_str, constants[i].prefix_txt, constants[i].prefix_base, constants[i].prefix_pow);
		bt_assert_int_equal(err.composite, 0);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_atom_set(test->atom[i], constants[i].atom_str, constants[i].atom_txt);
		bt_assert_int_equal(err.composite, 0);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		mpq_set_si(test->power[i], constants[i].power_num, constants[i].power_den);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_set(test->part[i], test->prefix[i], test->atom[i], test->power[i]);
		bt_assert_int_equal(err.composite, 0);
	}

	*object = test;
	
	return BT_RESULT_OK;
}

BT_SUITE_TEARDOWN_DEF(us_part)
{
	err_t err = {0};
	struct test_obj * test = *object;
	
	for (unsigned int i = 0; i < PARTS; i++) {
		mpq_clear(test->power[i]);
	}
	
	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_clear(test->part[i]);
		bt_assert_int_equal(err.composite, 0);
	}

	free(test);

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, tostring, "tostring")
{
	err_t err = {0};
	struct test_obj * test = object;

	for (unsigned int i = 0; i < PARTS; i++) {
		size_t length = 0;
		err = us_part_tostring_length(test->part[i], &length);
		bt_assert_int_equal(err.composite, 0);

		char buffer[length+1];
		err = us_part_tostring(test->part[i], length, buffer);
		bt_assert_int_equal(err.composite, 0);
		bt_assert_str_equal(buffer, constants[i].part_str);
	}

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, totext, "totext")
{
	err_t err = {0};
	struct test_obj * test = object;

	for (unsigned int i = 0; i < PARTS; i++) {
		size_t length = 0;
		err = us_part_totext_length(test->part[i], &test_pattern, &length, false);
		bt_assert_int_equal(err.composite, 0);

		char buffer[length+1];
		err = us_part_totext(test->part[i], &test_pattern, length, buffer, false);
		bt_assert_int_equal(err.composite, 0);
		bt_assert_str_equal(buffer, constants[i].part_txt);
	}

	return BT_RESULT_OK;
}


BT_TEST_DEF(us_part, power, "power")
{
	err_t err = {0};
	struct test_obj * test = object;
	mpq_t one;
	mpq_t two;
	us_part_t part;

	mpq_init(one);
	mpq_init(two);

	mpq_set_si(one, 2, 3);
	
	err = us_part_init(part);
	bt_assert_int_equal(err.composite, 0);

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_power(test->part[i], one, part);
		bt_assert_int_equal(err.composite, 0);

		mpq_mul(two, one, test->part[i]->power);

		bt_assert_int_equal(mpq_cmp(part->power, two), 0);
	}
	
	err = us_part_clear(part);
	bt_assert_int_equal(err.composite, 0);
	
	mpq_clear(one);
	mpq_clear(two);

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, multiply, "multiply")
{
	err_t err = {0};
	struct test_obj * test = object;
	mpq_t two;
	us_part_t part;

	mpq_init(two);

	err = us_part_init(part);
	bt_assert_int_equal(err.composite, 0);

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_multiply(test->part[i], test->part[i], part);
		bt_assert_int_equal(err.composite, 0);

		mpq_add(two, test->part[i]->power, test->part[i]->power);

		bt_assert_int_equal(mpq_cmp(part->power, two), 0);
	}
	
	err = us_part_clear(part);
	bt_assert_int_equal(err.composite, 0);
	
	mpq_clear(two);

	return BT_RESULT_OK;
}

BT_TEST_DEF(us_part, joinable, "joinable")
{
	err_t err = {0};
	struct test_obj * test = object;
	bool res;
	unsigned int len = 0;
	unsigned int r = 0;
	us_part_t part;
	mpq_t two;
	
	mpq_init(two);
	
	err = us_part_init(part);
	bt_assert_int_equal(err.composite, 0);

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_joinable(test->part[0], test->part[i], &res);
		bt_assert_int_equal(err.composite, 0);
		if (res)
			len++;
	}

	const us_part_s * join[len];

	for (unsigned int i = 0; i < PARTS; i++) {
		err = us_part_joinable(test->part[0], test->part[i], &res);
		bt_assert_int_equal(err.composite, 0);
		if (res)
			join[r++] = test->part[i];
	}
	
	
	mpq_set(two, test->part[0]->power);
	us_part_copy(part, test->part[0]);
	bt_assert_int_equal(err.composite, 0);

	for (unsigned int i = 0; i < len; i++) {
		mpq_add(two, two, test->part[i]->power);
		if (part->set)
			err = us_part_multiply(part, join[i], part);
		else
			err = us_part_copy(part, join[i]);
		bt_assert_int_equal(err.composite, 0);
		{
			size_t length = 0;
			err = us_part_totext_length(join[i], &test_pattern, &length, false);
			bt_assert_int_equal(err.composite, 0);

			char buffer[length+1];
			err = us_part_totext(join[i], &test_pattern, length, buffer, false);
			bt_log("joining %s\n", buffer);
		}
	}
	
	{
		size_t length = 0;
		err = us_part_totext_length(part, &test_pattern, &length, false);
		bt_assert_int_equal(err.composite, 0);

		char buffer[length+1];
		err = us_part_totext(part, &test_pattern, length, buffer, false);
		bt_log("joined %u parts, result is: %s\n", len+1, buffer);
	}

	err = us_part_clear(part);
	bt_assert_int_equal(err.composite, 0);
	
	mpq_clear(two);

	return BT_RESULT_OK;
}


BT_TEST_DEF(us_part, sort, "sort")
{
	err_t err = {0};
	struct test_obj * test = object;
	const us_part_s * parts[PARTS];
	
	for (unsigned int i = 0; i < PARTS; i++) {
		parts[i] = test->part[i];
	}

	err = us_part_sort(PARTS, parts);
	bt_assert_int_equal(err.composite, 0);
	
	return BT_RESULT_OK;
}


#endif
