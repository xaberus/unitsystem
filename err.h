
#ifndef __ERR_H__
#define __ERR_H__

#include <stdint.h>

union err {
	uint32_t composite;
	struct {
		uint8_t major;
		uint8_t minor;
		uint16_t extra;
	} values;
};

typedef union err err_t;

enum {
	ERR_MAJ_SUCCESS = 0,
	ERR_MAJ_NULL_POINTER,
	ERR_MAJ_OVERFLOW,
	ERR_MAJ_INVALID,
	ERR_MAJ_ALLOC,
};

enum {
	ERR_MIN_SUCCESS = 0,
	ERR_MIN_IN_NULL_POINTER,
	ERR_MIN_OUT_NULL_POINTER,
	ERR_MIN_IN_OVERFLOW,
	ERR_MIN_MID_OVERFLOW,
	ERR_MIN_OUT_OVERFLOW,
	ERR_MIN_IN_INVALID,
	ERR_MIN_ALLOC,
	ERR_MIN_REALLOC,
	ERR_MIN_FREE,
};

typedef void (err_log_t)(void *, char format[], ...);

extern err_log_t		*	err_logger;
extern void					* err_logger_data;

inline static err_t construct_error(uint8_t minor, uint8_t major, uint16_t extra)
{
	err_t err = { .values = {.major = major, .minor = minor, .extra = extra} };
	return err;
}

inline static err_t reconstruct_error(err_t err, uint16_t extra)
{
	err.values.extra = extra;
	return err;
}

/* argument checks */
#define check_in_ptr(_ptr, _extra) \
	do {\
		if ((_ptr) == NULL) {\
			if (err_logger) err_logger(err_logger_data, \
					"Unexepeted NULL pointer passed as parameter %s to function %s in %s at %u",\
					#_ptr, __FUNCTION__, __FILE__, __LINE__);\
			return construct_error(ERR_MAJ_NULL_POINTER, ERR_MIN_IN_NULL_POINTER, _extra);\
		}\
	} while(0)

#define check_out_ptr(_ptr, _extra) \
	do {\
		if ((_ptr) == NULL) {\
			if (err_logger) err_logger(err_logger_data, \
					"Unexepeted NULL pointer passed as output %s to function %s in %s at %u",\
					#_ptr, __FUNCTION__, __FILE__, __LINE__);\
			return construct_error(ERR_MAJ_NULL_POINTER, ERR_MIN_IN_NULL_POINTER, _extra);\
		}\
	} while(0)

#define check_in_string(_ptr, _extra) \
	do {\
		if ((_ptr) == NULL) {\
			if (err_logger) err_logger(err_logger_data, \
					"Unexepeted NULL string passed as parameter %s to function %s in %s at %u",\
					#_ptr, __FUNCTION__, __FILE__, __LINE__);\
			return construct_error(ERR_MAJ_NULL_POINTER, ERR_MIN_IN_NULL_POINTER, _extra);\
		}\
	} while(0)

/**/

#define checked_malloc(_ret, _type, _err, _extra, _label) \
	do {\
		_ret = malloc(sizeof(_type));\
		if (_ret == NULL) {\
			if (err_logger) err_logger(err_logger_data, \
					"Memory allocation failed for %s (%s) of size %lu in function %s in %s at %u",\
					#_ret, #_type, sizeof(_type),__FUNCTION__, __FILE__, __LINE__);\
			_err = construct_error(ERR_MAJ_NULL_POINTER, ERR_MIN_IN_NULL_POINTER, _extra);\
			goto _label;\
		}\
	} while(0)

#endif /* __ERR_H__ */
