
#define TEST

#include <stdint.h>
#ifdef TEST
#		include <bt.h>
#endif

#define API extern

typedef double approx_t;

struct us_rational_frac {
	unsigned long numerator;
	unsigned long denominator;
} exact;

struct us_rational_exact {
		struct us_rational_frac		frac;
		struct us_rational_frac		power;
};

struct us_rational {
	struct {
		unsigned int one : 1;
		unsigned int zero : 1;
		unsigned int exact : 1;
		unsigned int approx : 1;
	} type;
	union {
		struct us_rational_exact		exact;
		approx_t										approx;
	} value;
};

/* empty [con|de]structors */
API int us_rational_new(struct us_rational ** ret);
API int us_rational_one(struct us_rational * rat);
API int us_rational_zero(struct us_rational * rat);
API int us_rational_delete(struct us_rational ** ret);
/* copy, set */
API int us_rational_copy(const struct us_rational * src, struct us_rational * dst);
API int us_rational_set_frac(struct us_rational * rat, unsigned int numerator, unsigned int denominator);
/* output */
API int us_rational_tostring(const struct us_rational * rat, unsigned int length, char * buffer, unsigned int *written);
API int us_rational_totext(const struct us_rational * rat, unsigned int length, char * buffer, unsigned int *written);
/* tools */
API int us_rational_approximize(const struct us_rational * rat, approx_t * ret);
API int us_rational_simplify(struct us_rational * rat);
API int us_rational_sign(const struct us_rational * rat, int * sign);
API int us_rational_isexact(const struct us_rational * rat, int * result);
API int us_rational_isint(const struct us_rational * rat, int * result);
API int us_rational_iseven(const struct us_rational * rat, int * result);
API int us_rational_isodd(const struct us_rational * rat, int * result);
/* operations */
API int us_rational_multiply(const struct us_rational * left, const struct us_rational * right, struct us_rational * result);
API int us_rational_divide(const struct us_rational * left, const struct us_rational * right, struct us_rational * result);
API int us_rational_add(const struct us_rational * left, const struct us_rational * right, struct us_rational * result);
API int us_rational_substract(const struct us_rational * left, const struct us_rational * right, struct us_rational * result);
API int us_rational_power(const struct us_rational * left, const struct us_rational * right, struct us_rational * result);
API int us_rational_negate(const struct us_rational * rat, struct us_rational * result);
/* comparison */
API int us_rational_compare(struct us_rational * left, struct us_rational * right, int * result);
/* functions */
API int us_rational_abs(const struct us_rational * rat, struct us_rational * result);
API int us_rational_acos(const struct us_rational * rat, approx_t * result);
API int us_rational_acosh(const struct us_rational * rat, approx_t * result);
API int us_rational_asin(const struct us_rational * rat, approx_t * result);
API int us_rational_asinh(const struct us_rational * rat, approx_t * result);
API int us_rational_atan(const struct us_rational * rat, approx_t * result);
API int us_rational_atan2(const struct us_rational * xrat, const struct us_rational * yrat, approx_t * result);
API int us_rational_atanh(const struct us_rational * rat, approx_t * result);
API int us_rational_ceil(const struct us_rational * rat, approx_t * result);
API int us_rational_cos(const struct us_rational * rat, approx_t * result);
API int us_rational_cosh(const struct us_rational * rat, approx_t * result);
API int us_rational_exp(const struct us_rational * rat, approx_t * result);
API int us_rational_floor(const struct us_rational * rat, approx_t * result);
API int us_rational_log(const struct us_rational * rat, approx_t * result);
API int us_rational_round(const struct us_rational * rat, unsigned int digits, approx_t * result);
API int us_rational_sin(const struct us_rational * rat, approx_t * result);
API int us_rational_sinh(const struct us_rational * rat, approx_t * result);
API int us_rational_sqrt(const struct us_rational * rat, approx_t * result);
API int us_rational_tan(const struct us_rational * rat, approx_t * result);
API int us_rational_tanh(const struct us_rational * rat, approx_t * result);
/**/


#define UT_MAX_PREFIX_SYMBOL_LENGTH 5
#define UT_MAX_PREFIX_TEXT_LENGTH UT_MAX_PREFIX_SYMBOL_LENGTH + 26

struct us_prefix {
	uint8_t				base;
	int8_t				power;
	char					symbol[UT_MAX_PREFIX_SYMBOL_LENGTH + 1];
	char					text[UT_MAX_PREFIX_TEXT_LENGTH + 1];
};

API int us_prefix_new(const char symbol[], const char text[], const uint8_t base, const int8_t power, struct us_prefix ** ret);
API int us_prefix_tostring(const struct us_prefix * prefix);
API int us_prefix_totext(const struct us_prefix * prefix);
API int us_prefix_delete(struct us_prefix ** ret);

#define UT_MAX_ATOM_SYMBOL_LENGTH 7
#define UT_MAX_ATOM_TEXT_LENGTH UT_MAX_ATOM_SYMBOL_LENGTH + 24

struct us_atom {
	char					symbol[UT_MAX_ATOM_SYMBOL_LENGTH + 1];
	char					text[UT_MAX_ATOM_TEXT_LENGTH + 1];
};

API int us_atom_new(const char symbol[], const char text[],struct us_atom ** ret);
API int us_atom_tostring(const struct us_atom * atom);
API int us_atom_totext(const struct us_atom * atom);
API int us_atom_delete(struct us_atom ** ret);


struct us_part {
	const struct us_prefix		* prefix;
	const struct us_atom			* atom;
	struct us_rational	* power;
};

API int us_part_new(const struct us_prefix * prefix, const struct us_atom * atom, const struct us_rational * power, struct us_part ** ret);
API int us_part_delete(struct us_part ** ret);
/**/
API int us_part_tostring(const struct us_part * part);
API int us_part_totext(const struct us_part * part);
/**/
API int us_part_power(const struct us_part * part, const struct us_rational * power, struct us_part * result);
API int us_part_multiply(const struct us_part * left, const struct us_part * right, int * result);
API int us_part_normalize(const struct us_part * part, struct us_part * result);
/**/

struct us_part_list {
	struct us_part				* part;
	struct us_part_list		* next;
};

struct us_library;

struct us_unit {
	struct us_rational				* factor;
	const struct us_prefix		* prefix;
	struct us_part_list				* parts;
	struct us_atom						* abbreviation;
	const struct us_library		* library;
};

API int us_unit_new(struct us_library * library, unsigned int length, struct us_part * parts[], struct us_unit ** ret);
API int us_unit_delete(struct us_unit ** ret);
/**/
API int us_unit_tostring(struct us_unit * unit);
API int us_unit_totext(struct us_unit * unit);
/**/
API int us_unit_power(struct us_unit * left, struct us_rational * power, struct us_unit * result);
API int us_unit_multiply(const struct us_unit * left, const struct us_unit * right, struct us_unit * result);
API int us_unit_divide(const struct us_unit * left, const struct us_unit * right, struct us_unit * result);
/**/
API int us_unit_compare(const struct us_unit * left, const struct us_unit * right, int * result);
API int us_unit_strip(struct us_unit * unit);
/**/

struct us_library_prefix_list {
	const struct us_prefix * prefix;
	struct us_library_prefix_list * next;
};

struct us_library_unit_list {
	const struct us_unit * unit;
	struct us_library_unit_list * next;
};

struct us_library_abbreviation_list {
	const struct us_atom * abbreviation;
	const struct us_unit * unit;
	struct us_library_abbreviation_list * next;
};

struct us_library {
	struct us_library_unit_list * units;
	struct us_library_prefix_list * prefixes;
	struct us_library_abbreviation_list * abbreviations;
};

int us_library_new(const char specfile[], struct us_library ** ret);
int us_library_delete(struct us_library ** ret);
