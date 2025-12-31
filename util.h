/* See LICENSE file for copyright and license details. */

#include <stddef.h>

/**
 * @file util.h
 * @brief Utility macros and function declarations for suckless tools
 * @note Provides common utility functions and macros used across dwm components
 */

/**
 * @brief Return the maximum of two values
 * @param A First value to compare
 * @param B Second value to compare
 * @return The larger of A and B
 * @warning Macro evaluates parameters multiple times - avoid side effects
 * @bug No type checking - could cause unexpected behavior with different types
 */
#define MAX(A, B)               ((A) > (B) ? (A) : (B))

/**
 * @brief Return the minimum of two values
 * @param A First value to compare
 * @param B Second value to compare
 * @return The smaller of A and B
 * @warning Macro evaluates parameters multiple times - avoid side effects
 * @bug No type checking - could cause unexpected behavior with different types
 */
#define MIN(A, B)               ((A) < (B) ? (A) : (B))

/**
 * @brief Check if value X is between A and B (inclusive)
 * @param X Value to test
 * @param A Lower bound (inclusive)
 * @param B Upper bound (inclusive)
 * @return Non-zero if X is between A and B, zero otherwise
 * @warning Macro evaluates parameters multiple times - avoid side effects
 * @bug Assumes A <= B - no validation of bounds order
 */
#define BETWEEN(X, A, B)        ((A) <= (X) && (X) <= (B))

/**
 * @brief Calculate the number of elements in an array
 * @param X Array name (must be an actual array, not pointer)
 * @return Number of elements in the array
 * @warning Only works with actual arrays, not pointers
 * @bug Undefined behavior if X is not an array or is a function parameter
 */
#define LENGTH(X)               (sizeof (X) / sizeof (X)[0])

/**
 * @brief Print error message and exit program
 * @param fmt Format string for printf-style formatting
 * @param ... Variable arguments matching the format string
 * @note If format string ends with ':', appends strerror(errno)
 * @warning This function terminates the program with exit code 1
 * @return void (never returns due to exit)
 */
void die(const char *fmt, ...);

/**
 * @brief Allocate zero-initialized memory with error handling
 * @param nmemb Number of elements to allocate
 * @param size Size of each element in bytes
 * @return Pointer to allocated zero-initialized memory
 * @note Calls die() on allocation failure, never returns NULL
 * @warning Potential integer overflow if nmemb * size exceeds SIZE_MAX
 * @return Never returns NULL on success
 */
void *ecalloc(size_t nmemb, size_t size);
