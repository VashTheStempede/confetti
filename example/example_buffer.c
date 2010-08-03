#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void
out_warning(char *format, ...) {
    va_list args;

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

#include <my_product_cfg.h>

int
main(int argc __attribute__ ((unused)), char* argv[] __attribute__ ((unused))) {
	my_product	cfg;
	my_product_iterator_t	*i;
	char		*key, *value;

	fill_default_my_product(&cfg);

	parse_cfg_buffer_my_product(
		&cfg,
		"asdf.array[2].subarray[3].subkey = 123456789 "
		"root_array[3].ra = 12 "
		"asdf.k2=\"NEWVALUE\"" ,
		0
	);

	i = my_product_iterator_init();
	while ( (key = my_product_iterator_next(i, &cfg, &value)) != NULL ) {
		if (value) {
			printf("%s => '%s'\n", key, value);
			free(value);
		} else {
			printf("%s => (null)\n", key);
		}
	}

	return 0;
}