/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/**
 * @brief Print error message and exit program
 * @param fmt Format string for printf-style formatting
 * @param ... Variable arguments matching the format string
 * @note If format string ends with ':', appends strerror(errno)
 * @warning This function terminates the program with exit code 1
 * @bug No validation of fmt parameter - could cause segmentation fault if NULL
 * @bug Always exits with code 1, doesn't allow for different error severity levels
 * @return void (never returns due to exit)
 */
void
die(const char *fmt, ...)
{
	va_list ap;
	int saved_errno;

	saved_errno = errno;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':')
		fprintf(stderr, " %s", strerror(saved_errno));
	fputc('\n', stderr);

	exit(1);
}

/**
 * @brief Allocate zero-initialized memory with error handling
 * @param nmemb Number of elements to allocate
 * @param size Size of each element in bytes
 * @return Pointer to allocated zero-initialized memory
 * @note Calls die() on allocation failure, never returns NULL
 * @warning Potential integer overflow if nmemb * size exceeds SIZE_MAX
 * @warning No bounds checking on parameters - could allocate excessive memory
 * @bug Does not check for multiplication overflow in calloc
 * @see die() for error handling behavior
 */
void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}
