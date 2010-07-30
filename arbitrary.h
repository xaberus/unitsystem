
#include <stdint.h>
#include <stdlib.h>

#define API extern

struct arbitrary {
	uint8_t				* data;
	uint32_t				mlength;
	uint32_t				dlength;
	int32_t					exponent;
	int32_t					places;
	struct {
		signed int			sign : 2;

		unsigned int		zero : 1;
		unsigned int		one : 1;
		unsigned int		two : 1;
		unsigned int		three : 1;
		unsigned int		four : 1;
		unsigned int		five : 1;
		unsigned int		ten : 1;
		unsigned int		pi : 1;
		unsigned int		half_pi : 1;
		unsigned int		two_pi : 1;
		unsigned int		e : 1;
		unsigned int		log_e_base_10 : 1;
		unsigned int		log_10_base_e : 1;
		unsigned int		log_2_base_e : 1;
		unsigned int		log_3_base_e : 1;

		unsigned int		un1 : 15;
	} e;
};

API int arbitrary_new(struct arbitrary ** ret, uint32_t places);
API int arbitrary_delete(struct arbitrary ** ret);

API int arbitrary_set_sting(struct arbitrary * a, const char str[]);
API int arbitrary_set_long(struct arbitrary * a, long l);
API int arbitrary_set_double(struct arbitrary * a, double d);

API int arbitrary_to_string(struct arbitrary * a, size_t len, char buffer[], size_t * written);

API int arbitrary_abs(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_negate(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_copy(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_round(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_compare(const struct arbitrary * a, int * result);
API int arbitrary_sign(const struct arbitrary * a, int * result);
API int arbitrary_exponent(const struct arbitrary * a, uint32_t * result);
API int arbitrary_significance(const struct arbitrary * a, uint32_t * result);
API int arbitrary_is_integer(const struct arbitrary * a, int * result);
API int arbitrary_is_even(const struct arbitrary * a, int * result);
API int arbitrary_is_odd(const struct arbitrary * a, int * result);

API int arbitrary_is_gcd(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result);
API int arbitrary_is_lcm(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result);

API int arbitrary_add(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result);
API int arbitrary_substract(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result);
API int arbitrary_multiply(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result);
API int arbitrary_divide(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_integer_divide(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result);
API int arbitrary_integer_divide_rem(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result, struct arbitrary * rem);
API int arbitrary_reciprocal(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_factorial(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_floor(const struct arbitrary * a, struct arbitrary * result);
API int arbitrary_ceil(const struct arbitrary * a, struct arbitrary * result);

API int arbitrary_sqrt(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_cbrt(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_log(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_log10(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_exp(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_pow(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_integer_pow(const struct arbitrary * a, uint32_t mexp, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_integer_pow(const struct arbitrary * a, uint32_t mexp, struct arbitrary * result);

API int arbitrary_sin(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_cos(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_tan(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arcsin(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arccos(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arctan(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arctan2(const struct arbitrary * a, const struct arbitrary * b, struct arbitrary * result/*, uint32_t places*/);

API int arbitrary_sinh(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_cosh(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_tanh(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arcsinh(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arccosh(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
API int arbitrary_arctanh(const struct arbitrary * a, struct arbitrary * result/*, uint32_t places*/);
