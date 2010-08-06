FIELDS="
	rbrace
	lbrace
	rpar
	lpar
	frac_s
	frac_m
	frac_e
	pow_s
	pow_m
	pow_e
	prefix_s
	prefix_e
	atom_s
	atom_e
	part_s
	part_e
	sign
	unit_s
	unit_e
	unitfrac_s
	unitfrac_m
	unitfrac_e
	prefix_sep
"


for a in $(echo ${FIELDS}); do
	echo -en "\tconst char * ${a} = NULL;\n\tsize_t ${a}_len = 0;\n"
done

echo "
err_t _us_library_parse_template(us_library_t uslib, sexp_t * template)
{
	err_t err = {0};
	sexp_t * args;
	sexp_t * arg;
	sexp_t * val;

	args = template->next;
	arg = args;
	while (arg) {
		val = arg->list;
		if (val->ty != SEXP_VALUE)
			return construct_error(ERR_MAJ_INVALID, ERR_MIN_IN_INVALID, LIBRARY_LIST_START_NOT_VALUE);
"

ELSE="		"

for a in $(echo ${FIELDS}); do
	echo -n "${ELSE}if (strncmp(val->val, \"${a}\", val->val_used) == 0) {
			err = _us_library_tuple_getval(&${a}, &${a}_len, val);
			if (!${a}) {
				${a} = \"\";
				${a}_len = 0;
			}
		}"
	ELSE=" else "
done
echo
echo

echo "
		arg = arg->next;
	}
"

PLUS="	size_t pos = 0;
	size_t length = "
C=0

for a in $(echo ${FIELDS}); do
	if [ $C == 5 ]; then
		echo  -en "\n		"
		C=0
	fi
	echo -en "${PLUS}${a}_len"
	PLUS=" + "
	C=$((C+1))
done
echo ";"

echo "
	checked_array_malloc(uslib->storage, char, 2 * length, err, 0, malloc_error);
"


for a in $(echo ${FIELDS}); do
	echo "
	memcpy(uslib->storage+pos, ${a}, ${a}_len);
	uslib->pattern.${a} = uslib->storage+pos;
	pos += ${a}_len; uslib->storage[pos++] = '\0';
	/*bt_log(\"${a} set\\n\");*/
"
done

echo "

	return err;
malloc_error:
	return err;
}
"
