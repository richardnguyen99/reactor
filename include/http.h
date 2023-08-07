/**
 * @file http.h
 * @author Richard Nguyen (richard@richardhnguyen.com)
 * @brief HTTP class for all http functionalities.
 * @version 0.2
 * @date 2023-08-07
 *
 * @copyright Copyright (c) 2023
 */

#ifndef _REACTOR_HTTP_H_
#define _REACTOR_HTTP_H_ 1

#include "dict.h"
#include "httpdef.h"
#include "request.h"
#include "response.h"

/**
 * @brief Example showing how to document a function with Doxygen.
 *
 * Description of what the function does. This part may refer to the parameters
 * of the function, like @p param1 or @p param2. A word of code can also be
 * inserted like @c this which is equivalent to <tt>this</tt> and can be useful
 * to say that the function returns a @c void or an @c int. If you want to have
 * more than one word in typewriter font, then just use @<tt@>.
 * We can also include text verbatim,
 * @verbatim like this@endverbatim
 * Sometimes it is also convenient to include an example of usage:
 * @code
 * BoxStruct *out = Box_The_Function_Name(param1, param2);
 * printf("something...\n");
 * @endcode
 * Or,
 * @code{.py}
 * pyval = python_func(arg1, arg2)
 * print pyval
 * @endcode
 * when the language is not the one used in the current source file (but
 * <b>be careful</b> as this may be supported only by recent versions
 * of Doxygen). By the way, <b>this is how you write bold text</b> or,
 * if it is just one word, then you can just do @b this.
 * @param param1 Description of the first parameter of the function.
 * @param param2 The second one, which follows @p param1.
 * @return Describe what the function returns.
 * @see Box_The_Second_Function
 * @see Box_The_Last_One
 * @see http://website/
 * @note Something to note.
 * @warning Warning.
 */
char *
http_get_status_text(int status);

typedef int (*http_header_handler)(const char *);

/**
 * @brief
 *
 * @param headers
 * @param key
 * @return int
 */
int
http_require_header(struct dict *headers, const char *key,
                    http_header_handler func);

struct dict *
http_require_accept(struct dict *headers);

#endif // _REACTOR_HTTP_H_
