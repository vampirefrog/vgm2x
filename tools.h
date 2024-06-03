#ifndef TOOLS_H_
#define TOOLS_H_

#include <stdlib.h>
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int gcd(int a, int b); /* Greatest Common Divisor */
void csv_quote(char *str, size_t len);

#ifndef __MINGW32__
// Execute fn for every file or for each file in every folder
// if recurse = 1, recurse subdirectories
// names and num_names can come from argv and argc
void each_file(const char **names, int num_names, void (*fn)(char *), int recurse);
#endif

#endif /* TOOLS_H_ */
