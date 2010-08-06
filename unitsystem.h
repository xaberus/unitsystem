
#ifndef __UNITSYSTEM_H__
#define __UNITSYSTEM_H__

#include <err.h>

#include <stdint.h>
#include <stdbool.h>
#ifdef TEST
#		include <bt.h>
#endif

#include <gmp.h>
#include <mpfr.h>

#define API extern

#ifdef TEST
#undef bt_assert_err_equal
#undef bt_assert_err_not_equal

#define bt_assert_err_equal_i(__actual, __maj, __min, __extra) \
	do { \
		err_t __expected = {.values = {.major = __maj, .minor = __min, .extra = __extra}};\
		if ((__actual).composite != (__expected).composite) { \
			dprintf(STDOUT_FILENO, "%s:%s:%d: Assertion failed: expeced error '%u.%u.%u' , got '%u.%u.%u' \n", \
					__FILE__, __FUNCTION__, __LINE__, \
					(__expected).values.major, (__expected).values.minor, (__expected).values.extra,\
					(__actual).values.major, (__actual).values.minor, (__actual).values.extra); \
			return BT_RESULT_FAIL; \
		} \
	} while(0)
#endif

#define US_MAX_PREFIX_SYMBOL_LENGTH 5
#define US_MAX_PREFIX_TEXT_LENGTH US_MAX_PREFIX_SYMBOL_LENGTH + 26

struct us_prefix {
	uint8_t		base;
	int8_t		power;
	char			symbol[US_MAX_PREFIX_SYMBOL_LENGTH + 1];
	char			text[US_MAX_PREFIX_TEXT_LENGTH + 1];
};

typedef struct us_prefix us_prefix_t[1];
typedef struct us_prefix us_prefix_s;

API err_t us_prefix_set(us_prefix_t prefix, const char symbol[], const char text[], const uint8_t base, const int8_t power);
API err_t us_prefix_copy(us_prefix_t prefix, const us_prefix_t source);
API err_t us_prefix_tostring(const us_prefix_t prefix, size_t length, char buffer[]);
API err_t us_prefix_totext(const us_prefix_t prefix, size_t length, char buffer[]);

API err_t us_prefix_equal(const us_prefix_t left, const us_prefix_t right, bool * result);

#define US_MAX_ATOM_SYMBOL_LENGTH 7
#define US_MAX_ATOM_TEXT_LENGTH US_MAX_ATOM_SYMBOL_LENGTH + 24

struct us_atom {
	char		symbol[US_MAX_ATOM_SYMBOL_LENGTH + 1];
	char		text[US_MAX_ATOM_TEXT_LENGTH + 1];
};

typedef struct us_atom us_atom_t[1];
typedef struct us_atom us_atom_s;

API err_t us_atom_set(us_atom_t atom, const char symbol[], const char text[]);
API err_t us_atom_copy(us_atom_t atom, const us_atom_t source);
API err_t us_atom_tostring(const us_atom_t atom, size_t length, char buffer[]);
API err_t us_atom_totext(const us_atom_t atom, size_t length, char buffer[]);

err_t us_atom_equal(const us_atom_t left, const us_atom_t right, bool * result);

struct us_text_pattern {
	const char * lbrace;
	const char * rbrace;
	
	const char * lpar;
	const char * rpar;
	
	const char * frac_s;
	const char * frac_m;
	const char * frac_e;
	
	const char * pow_s;
	const char * pow_m;
	const char * pow_e;
	
	const char * neg_s;
	const char * neg_e;

	const char * part_s;
	const char * part_e;

	const char * prefix_sep;

	const char * sign;
	
	const char * unit_s;
	const char * unit_e;

	const char * unitfrac_s;
	const char * unitfrac_m;
	const char * unitfrac_e;
	
	const char * prefix_s;
	const char * prefix_e;

	const char * atom_s;
	const char * atom_e;
};

typedef struct us_text_pattern us_text_pattern_t;

struct us_part {
	const us_prefix_s							* prefix;
	const us_atom_s								* atom;
	mpq_t														power;
	bool initialized;
	bool set;
};

typedef struct us_part us_part_t[1];
typedef struct us_part us_part_s;

API err_t us_part_init(us_part_t part);
API err_t us_part_set(us_part_t part, const us_prefix_t prefix, const us_atom_t atom, const mpq_t power);
API err_t us_part_copy(us_part_t part, const us_part_t source);
API err_t us_part_clear(us_part_t part);
/**/
API err_t us_part_tostring_length(const us_part_t part, size_t * length);
API err_t us_part_tostring(const us_part_t part, size_t length, char buffer[]);
API err_t us_part_totext_length(const us_part_t part, const us_text_pattern_t * pattern, size_t * length, bool invert);
API err_t us_part_totext(const us_part_t part, const us_text_pattern_t * pattern, size_t length, char buffer[], bool invert);
/**/
API err_t us_part_power(const us_part_t part, const mpq_t power, us_part_t result);
API err_t us_part_multiply(const us_part_t left, const us_part_t right, us_part_t result);
API err_t us_part_normalize(const us_part_t part, us_part_t result);
/**/
API err_t us_part_joinable(const us_part_t left, const us_part_t right, bool * result);
API err_t us_part_sort(unsigned int length, const us_part_s * parts[]);
/**/
API err_t us_part_equal(const us_part_t left, const us_part_t right, bool * result);

struct us_base_unit {
	bool initialized;
	bool parts_set;
	bool composite_set;

	/* derived unit if composite_set set */
	const us_atom_s	* composite;
	/* base units */
	unsigned int			parts_length;
	us_part_s				* parts;
};

typedef struct us_base_unit us_base_unit_t[1];
typedef struct us_base_unit us_base_unit_s;

API err_t us_base_unit_init(us_base_unit_t base);
API err_t us_base_unit_clear(us_base_unit_t base);
/**/
API err_t us_base_unit_totext_length(const us_base_unit_t base, const us_text_pattern_t * pattern, size_t * length);
API err_t us_base_unit_totext(const us_base_unit_t base, const us_text_pattern_t * pattern, size_t length, char buffer[]);
/**/
API err_t us_base_unit_set_composite(us_base_unit_t base, const us_atom_t atom);
API err_t us_base_unit_set_parts(us_base_unit_t base, unsigned int length, const us_part_s * parts[]);
/**/

typedef struct us_part_list us_part_list_t;

/* store them inside, as they shall not be modified */
struct us_prefix_list {
	us_prefix_t							prefix;
	struct us_prefix_list	* next;
};

typedef struct us_prefix_list us_prefix_list_t;

/* store them inside, as they shall not be modified */
struct us_atom_list {
	us_atom_t							atom;
	struct us_atom_list	* next;
};

typedef struct us_atom_list us_atom_list_t;


/* store them inside, as they shall not be modified */
struct us_base_unit_list {
	us_base_unit_t						unit;
	struct us_base_unit_list	* next;
};

typedef struct us_base_unit_list us_base_unit_list_t;

struct us_library {
	us_text_pattern_t				pattern;

	us_prefix_list_t			* prefixes;
	us_atom_list_t				* atoms;
	us_base_unit_list_t		* units;

	char * storage;
};

typedef struct us_library us_library_t[1];
typedef struct us_library us_library_s;

API err_t us_library_init(us_library_t lib, const char specfile[]);
API err_t us_library_clear(us_library_t lib);


struct us_part_list {
	us_part_t							* part;
	struct us_part_list		* next;
};

/* volatile unit expressions */

struct us_unit {
	/* base units cannot have zero parts - units can, with implications */

	struct {
		/* reducible part */
		const us_prefix_s		* prefix;
		/* rational irreducible part */
		uint8_t	base;
		mpq_t		power;
		mpq_t		rational;
		/* approximate real irreducible part */
		mpf_t		real;
	} factor;

	/* list with atomic parts - may be empty */
	us_part_list_t			* parts;
	/* assigned derived unit */
	us_atom_t						* abbreviation;

	/* reference to the library - for lookups */
	const us_library_s	* library;
};

typedef struct us_unit us_unit_t;

API err_t us_unit_new(us_library_t * library, unsigned int length, us_part_t parts[], us_unit_t ** ret);
API err_t us_unit_delete(us_unit_t ** ret);
/**/
API err_t us_unit_tostring(us_unit_t * unit);
API err_t us_unit_totext(us_unit_t * unit);
/**/
API err_t us_unit_power(us_unit_t * left, const mpq_t power, us_unit_t * result);
API err_t us_unit_multiply(const us_unit_t * left, const us_unit_t * right, us_unit_t * result);
API err_t us_unit_divide(const us_unit_t * left, const us_unit_t * right, us_unit_t * result);
/**/
API err_t us_unit_compare(const us_unit_t * left, const us_unit_t * right, int * result);
API err_t us_unit_strip(us_unit_t * unit);
/**/

#endif /* __UNITSYSTEM_H__ */
