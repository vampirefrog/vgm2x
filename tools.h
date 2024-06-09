#ifndef TOOLS_H_
#define TOOLS_H_

#include <stdlib.h>
#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int gcd(int a, int b); /* Greatest Common Divisor */
void csv_quote(char *str, size_t len);

#endif /* TOOLS_H_ */
