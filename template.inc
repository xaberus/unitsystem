	const char * rbrace = NULL;
	size_t rbrace_len = 0;
	const char * lbrace = NULL;
	size_t lbrace_len = 0;
	const char * rpar = NULL;
	size_t rpar_len = 0;
	const char * lpar = NULL;
	size_t lpar_len = 0;
	const char * frac_s = NULL;
	size_t frac_s_len = 0;
	const char * frac_m = NULL;
	size_t frac_m_len = 0;
	const char * frac_e = NULL;
	size_t frac_e_len = 0;
	const char * pow_s = NULL;
	size_t pow_s_len = 0;
	const char * pow_m = NULL;
	size_t pow_m_len = 0;
	const char * pow_e = NULL;
	size_t pow_e_len = 0;
	const char * prefix_s = NULL;
	size_t prefix_s_len = 0;
	const char * prefix_e = NULL;
	size_t prefix_e_len = 0;
	const char * atom_s = NULL;
	size_t atom_s_len = 0;
	const char * atom_e = NULL;
	size_t atom_e_len = 0;
	const char * part_s = NULL;
	size_t part_s_len = 0;
	const char * part_e = NULL;
	size_t part_e_len = 0;
	const char * sign = NULL;
	size_t sign_len = 0;
	const char * unit_s = NULL;
	size_t unit_s_len = 0;
	const char * unit_e = NULL;
	size_t unit_e_len = 0;
	const char * unitfrac_s = NULL;
	size_t unitfrac_s_len = 0;
	const char * unitfrac_m = NULL;
	size_t unitfrac_m_len = 0;
	const char * unitfrac_e = NULL;
	size_t unitfrac_e_len = 0;
	const char * prefix_sep = NULL;
	size_t prefix_sep_len = 0;

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

		if (strncmp(val->val, "rbrace", val->val_used) == 0) {
			err = _us_library_tuple_getval(&rbrace, &rbrace_len, val);
			if (!rbrace) {
				rbrace = "";
				rbrace_len = 0;
			}
		} else if (strncmp(val->val, "lbrace", val->val_used) == 0) {
			err = _us_library_tuple_getval(&lbrace, &lbrace_len, val);
			if (!lbrace) {
				lbrace = "";
				lbrace_len = 0;
			}
		} else if (strncmp(val->val, "rpar", val->val_used) == 0) {
			err = _us_library_tuple_getval(&rpar, &rpar_len, val);
			if (!rpar) {
				rpar = "";
				rpar_len = 0;
			}
		} else if (strncmp(val->val, "lpar", val->val_used) == 0) {
			err = _us_library_tuple_getval(&lpar, &lpar_len, val);
			if (!lpar) {
				lpar = "";
				lpar_len = 0;
			}
		} else if (strncmp(val->val, "frac_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&frac_s, &frac_s_len, val);
			if (!frac_s) {
				frac_s = "";
				frac_s_len = 0;
			}
		} else if (strncmp(val->val, "frac_m", val->val_used) == 0) {
			err = _us_library_tuple_getval(&frac_m, &frac_m_len, val);
			if (!frac_m) {
				frac_m = "";
				frac_m_len = 0;
			}
		} else if (strncmp(val->val, "frac_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&frac_e, &frac_e_len, val);
			if (!frac_e) {
				frac_e = "";
				frac_e_len = 0;
			}
		} else if (strncmp(val->val, "pow_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&pow_s, &pow_s_len, val);
			if (!pow_s) {
				pow_s = "";
				pow_s_len = 0;
			}
		} else if (strncmp(val->val, "pow_m", val->val_used) == 0) {
			err = _us_library_tuple_getval(&pow_m, &pow_m_len, val);
			if (!pow_m) {
				pow_m = "";
				pow_m_len = 0;
			}
		} else if (strncmp(val->val, "pow_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&pow_e, &pow_e_len, val);
			if (!pow_e) {
				pow_e = "";
				pow_e_len = 0;
			}
		} else if (strncmp(val->val, "prefix_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&prefix_s, &prefix_s_len, val);
			if (!prefix_s) {
				prefix_s = "";
				prefix_s_len = 0;
			}
		} else if (strncmp(val->val, "prefix_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&prefix_e, &prefix_e_len, val);
			if (!prefix_e) {
				prefix_e = "";
				prefix_e_len = 0;
			}
		} else if (strncmp(val->val, "atom_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&atom_s, &atom_s_len, val);
			if (!atom_s) {
				atom_s = "";
				atom_s_len = 0;
			}
		} else if (strncmp(val->val, "atom_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&atom_e, &atom_e_len, val);
			if (!atom_e) {
				atom_e = "";
				atom_e_len = 0;
			}
		} else if (strncmp(val->val, "part_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&part_s, &part_s_len, val);
			if (!part_s) {
				part_s = "";
				part_s_len = 0;
			}
		} else if (strncmp(val->val, "part_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&part_e, &part_e_len, val);
			if (!part_e) {
				part_e = "";
				part_e_len = 0;
			}
		} else if (strncmp(val->val, "sign", val->val_used) == 0) {
			err = _us_library_tuple_getval(&sign, &sign_len, val);
			if (!sign) {
				sign = "";
				sign_len = 0;
			}
		} else if (strncmp(val->val, "unit_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&unit_s, &unit_s_len, val);
			if (!unit_s) {
				unit_s = "";
				unit_s_len = 0;
			}
		} else if (strncmp(val->val, "unit_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&unit_e, &unit_e_len, val);
			if (!unit_e) {
				unit_e = "";
				unit_e_len = 0;
			}
		} else if (strncmp(val->val, "unitfrac_s", val->val_used) == 0) {
			err = _us_library_tuple_getval(&unitfrac_s, &unitfrac_s_len, val);
			if (!unitfrac_s) {
				unitfrac_s = "";
				unitfrac_s_len = 0;
			}
		} else if (strncmp(val->val, "unitfrac_m", val->val_used) == 0) {
			err = _us_library_tuple_getval(&unitfrac_m, &unitfrac_m_len, val);
			if (!unitfrac_m) {
				unitfrac_m = "";
				unitfrac_m_len = 0;
			}
		} else if (strncmp(val->val, "unitfrac_e", val->val_used) == 0) {
			err = _us_library_tuple_getval(&unitfrac_e, &unitfrac_e_len, val);
			if (!unitfrac_e) {
				unitfrac_e = "";
				unitfrac_e_len = 0;
			}
		} else if (strncmp(val->val, "prefix_sep", val->val_used) == 0) {
			err = _us_library_tuple_getval(&prefix_sep, &prefix_sep_len, val);
			if (!prefix_sep) {
				prefix_sep = "";
				prefix_sep_len = 0;
			}
		}


		arg = arg->next;
	}

	size_t pos = 0;
	size_t length = rbrace_len + lbrace_len + rpar_len + lpar_len + frac_s_len
		 + frac_m_len + frac_e_len + pow_s_len + pow_m_len + pow_e_len
		 + prefix_s_len + prefix_e_len + atom_s_len + atom_e_len + part_s_len
		 + part_e_len + sign_len + unit_s_len + unit_e_len + unitfrac_s_len
		 + unitfrac_m_len + unitfrac_e_len + prefix_sep_len;

	checked_array_malloc(uslib->storage, char, 2 * length, err, 0, malloc_error);


	memcpy(uslib->storage+pos, rbrace, rbrace_len);
	uslib->pattern.rbrace = uslib->storage+pos;
	pos += rbrace_len; uslib->storage[pos++] = '\0';
	/*bt_log("rbrace set\n");*/


	memcpy(uslib->storage+pos, lbrace, lbrace_len);
	uslib->pattern.lbrace = uslib->storage+pos;
	pos += lbrace_len; uslib->storage[pos++] = '\0';
	/*bt_log("lbrace set\n");*/


	memcpy(uslib->storage+pos, rpar, rpar_len);
	uslib->pattern.rpar = uslib->storage+pos;
	pos += rpar_len; uslib->storage[pos++] = '\0';
	/*bt_log("rpar set\n");*/


	memcpy(uslib->storage+pos, lpar, lpar_len);
	uslib->pattern.lpar = uslib->storage+pos;
	pos += lpar_len; uslib->storage[pos++] = '\0';
	/*bt_log("lpar set\n");*/


	memcpy(uslib->storage+pos, frac_s, frac_s_len);
	uslib->pattern.frac_s = uslib->storage+pos;
	pos += frac_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("frac_s set\n");*/


	memcpy(uslib->storage+pos, frac_m, frac_m_len);
	uslib->pattern.frac_m = uslib->storage+pos;
	pos += frac_m_len; uslib->storage[pos++] = '\0';
	/*bt_log("frac_m set\n");*/


	memcpy(uslib->storage+pos, frac_e, frac_e_len);
	uslib->pattern.frac_e = uslib->storage+pos;
	pos += frac_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("frac_e set\n");*/


	memcpy(uslib->storage+pos, pow_s, pow_s_len);
	uslib->pattern.pow_s = uslib->storage+pos;
	pos += pow_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("pow_s set\n");*/


	memcpy(uslib->storage+pos, pow_m, pow_m_len);
	uslib->pattern.pow_m = uslib->storage+pos;
	pos += pow_m_len; uslib->storage[pos++] = '\0';
	/*bt_log("pow_m set\n");*/


	memcpy(uslib->storage+pos, pow_e, pow_e_len);
	uslib->pattern.pow_e = uslib->storage+pos;
	pos += pow_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("pow_e set\n");*/


	memcpy(uslib->storage+pos, prefix_s, prefix_s_len);
	uslib->pattern.prefix_s = uslib->storage+pos;
	pos += prefix_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("prefix_s set\n");*/


	memcpy(uslib->storage+pos, prefix_e, prefix_e_len);
	uslib->pattern.prefix_e = uslib->storage+pos;
	pos += prefix_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("prefix_e set\n");*/


	memcpy(uslib->storage+pos, atom_s, atom_s_len);
	uslib->pattern.atom_s = uslib->storage+pos;
	pos += atom_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("atom_s set\n");*/


	memcpy(uslib->storage+pos, atom_e, atom_e_len);
	uslib->pattern.atom_e = uslib->storage+pos;
	pos += atom_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("atom_e set\n");*/


	memcpy(uslib->storage+pos, part_s, part_s_len);
	uslib->pattern.part_s = uslib->storage+pos;
	pos += part_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("part_s set\n");*/


	memcpy(uslib->storage+pos, part_e, part_e_len);
	uslib->pattern.part_e = uslib->storage+pos;
	pos += part_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("part_e set\n");*/


	memcpy(uslib->storage+pos, sign, sign_len);
	uslib->pattern.sign = uslib->storage+pos;
	pos += sign_len; uslib->storage[pos++] = '\0';
	/*bt_log("sign set\n");*/


	memcpy(uslib->storage+pos, unit_s, unit_s_len);
	uslib->pattern.unit_s = uslib->storage+pos;
	pos += unit_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("unit_s set\n");*/


	memcpy(uslib->storage+pos, unit_e, unit_e_len);
	uslib->pattern.unit_e = uslib->storage+pos;
	pos += unit_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("unit_e set\n");*/


	memcpy(uslib->storage+pos, unitfrac_s, unitfrac_s_len);
	uslib->pattern.unitfrac_s = uslib->storage+pos;
	pos += unitfrac_s_len; uslib->storage[pos++] = '\0';
	/*bt_log("unitfrac_s set\n");*/


	memcpy(uslib->storage+pos, unitfrac_m, unitfrac_m_len);
	uslib->pattern.unitfrac_m = uslib->storage+pos;
	pos += unitfrac_m_len; uslib->storage[pos++] = '\0';
	/*bt_log("unitfrac_m set\n");*/


	memcpy(uslib->storage+pos, unitfrac_e, unitfrac_e_len);
	uslib->pattern.unitfrac_e = uslib->storage+pos;
	pos += unitfrac_e_len; uslib->storage[pos++] = '\0';
	/*bt_log("unitfrac_e set\n");*/


	memcpy(uslib->storage+pos, prefix_sep, prefix_sep_len);
	uslib->pattern.prefix_sep = uslib->storage+pos;
	pos += prefix_sep_len; uslib->storage[pos++] = '\0';
	/*bt_log("prefix_sep set\n");*/



	return err;
malloc_error:
	return err;
}

