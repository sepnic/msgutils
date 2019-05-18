/* The MIT License (MIT)
 *
 * Copyright (c) 2019 luoyun <sysu.zqlong@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __SMART_PTR_H__
#define __SMART_PTR_H__

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define ENABLE_SMART_PTR_DETECT

#if !defined(ENABLE_SMART_PTR_DETECT)
    void *smart_ptr_new(size_t size, void (*free_cb)(void *ptr));
    void smart_ptr_get(void *ptr);
    void smart_ptr_put(void *ptr);

    #define SMART_PTR_NEW(size, free_cb) smart_ptr_new((size), (free_cb))
    #define SMART_PTR_GET(ptr) smart_ptr_get(ptr)
    #define SMART_PTR_PUT(ptr) smart_ptr_put(ptr)
    #define SMART_PTR_DUMP() do {} while (0)

#else
    void *smart_ptr_new_debug(size_t size, void (*free_cb)(void *ptr),
                              const char *file, const char *func, int line);
    void smart_ptr_get_debug(void *ptr, const char *file, const char *func, int line);
    void smart_ptr_put_debug(void *ptr, const char *file, const char *func, int line);
    void smart_ptr_dump_debug();

    #define SMART_PTR_NEW(size, free_cb) \
        smart_ptr_new_debug((size), (free_cb), __FILE__, __FUNCTION__, __LINE__)
    #define SMART_PTR_GET(ptr) \
        smart_ptr_get_debug(ptr, __FILE__, __FUNCTION__, __LINE__)
    #define SMART_PTR_PUT(ptr) \
        smart_ptr_put_debug(ptr, __FILE__, __FUNCTION__, __LINE__)
    #define SMART_PTR_DUMP() smart_ptr_dump_debug()
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SMART_PTR_H__ */