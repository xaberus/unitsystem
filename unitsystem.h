
#ifndef __UNITSYSTEM_H__
#define __UNITSYSTEM_H__

#include <err.h>

#include <stdint.h>
#ifdef TEST
#		include <bt.h>
#endif

#include <gmp.h>
#include <mpfr.h>

#define API extern


#define US_MAX_PREFIX_SYMBOL_LENGTH 5
#define US_MAX_PREFIX_TEXT_LENGTH US_MAX_PREFIX_SYMBOL_LENGTH + 26

struct us_prefix {
	uint8_t		base;
	int8_t		power;
	char			symbol[US_MAX_PREFIX_SYMBOL_LENGTH + 1];
	char			text[US_MAX_PREFIX_TEXT_LENGTH + 1];
};

typedef struct us_prefix us_prefix_t;

API err_t us_prefix_new(const char symbol[], const char text[], const uint8_t base, const int8_t power, us_prefix_t ** ret);
API err_t us_prefix_tostring(const us_prefix_t * prefix, size_t length, char buffer[]);
API err_t us_prefix_totext(const us_prefix_t * prefix, size_t length, char buffer[]);
API err_t us_prefix_delete(us_prefix_t ** ret);

#define US_MAX_ATOM_SYMBOL_LENGTH 7
#define US_MAX_ATOM_TEXT_LENGTH US_MAX_ATOM_SYMBOL_LENGTH + 24

struct us_atom {
	char		symbol[US_MAX_ATOM_SYMBOL_LENGTH + 1];
	char		text[US_MAX_ATOM_TEXT_LENGTH + 1];
};

typedef struct us_atom us_atom_t;

API err_t us_atom_new(const char symbol[], const char text[], us_atom_t ** ret);
API err_t us_atom_tostring(const us_atom_t * atom, size_t length, char buffer[]);
API err_t us_atom_totext(const us_atom_t * atom, size_t length, char buffer[]);
API err_t us_atom_delete(us_atom_t ** ret);


struct us_part {
	const us_prefix_t		* prefix;
	const us_atom_t			* atom;
	mpq_t									power;
};

typedef struct us_part us_part_t;

API err_t us_part_new(const us_prefix_t * prefix, const us_atom_t * atom, const mpq_t power, us_part_t ** ret);
API err_t us_part_delete(us_part_t ** ret);
/**/
API err_t us_part_tostring_length(const us_part_t * part, size_t * length);
API err_t us_part_tostring(const us_part_t * part, size_t length, char buffer[]);
API err_t us_part_totext(const us_part_t * part, size_t length, char buffer[]);
/**/
API err_t us_part_power(const us_part_t * part, const mpq_t power, us_part_t * result);
API err_t us_part_multiply(const us_part_t * left, const us_part_t * right, int * result);
API err_t us_part_normalize(const us_part_t * part, us_part_t * result);
/**/

struct us_part_list {
	us_part_t							* part;
	struct us_part_list		* next;
};

typedef struct us_part_list us_part_list_t;

typedef struct us_library us_library_t;

struct us_unit {
	const mpq_t						factor;
	const us_prefix_t		* prefix;
	us_part_list_t			* parts;
	us_atom_t						* abbreviation;
	const us_library_t	* library;
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

struct us_library_prefix_list {
	const us_prefix_t							* prefix;
	struct us_library_prefix_list	* next;
};

typedef struct us_library_prefix_list us_library_prefix_list_t;

struct us_library_unit_list {
	const us_unit_t							* unit;
	struct us_library_unit_list	* next;
};

typedef struct us_library_unit_list us_library_unit_list_t;

struct us_library_abbreviation_list {
	const us_atom_t											* abbreviation;
	const us_unit_t											* unit;
	struct us_library_abbreviation_list	* next;
};

typedef struct us_library_abbreviation_list us_library_abbreviation_list_t;

struct us_library {
	us_library_unit_list_t					* units;
	us_library_prefix_list_t				* prefixes;
	us_library_abbreviation_list_t	* abbreviations;
};

API err_t us_library_new(const char specfile[], us_library_t ** ret);
API err_t us_library_delete(us_library_t ** ret);

#endif /* __UNITSYSTEM_H__ */
