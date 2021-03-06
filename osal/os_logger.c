/*
 * Copyright (C) 2018-2020 luoyun <sysu.zqlong@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "cutils/os_thread.h"
#include "cutils/os_time.h"
#include "cutils/os_logger.h"

//#define ENABLE_LOG_SAVE

#if defined(ENABLE_LOG_SAVE)
OS_MUTEX_DECLARE(log_config_mutex);
#endif

#define LOG_BUFFER_SIZE    1024
#define LOG_FILE_ENABLE    false
#define LOG_FILE_PATH      "/tmp"
#define LOG_FILE_PREFIX    "log"
#define LOG_FILE_LIMITSIZE (4*1024*1024)

struct log_config {
    bool enable;
    enum os_logprio prio;

    bool file_enable;
    const char *file_path;
    const char *file_prefix;
    size_t file_limitsize;
    os_mutex_t file_mutex;
};

static struct log_config log_config = {
    .enable = true,
    .prio   = OS_LOG_VERBOSE,

    .file_enable    = LOG_FILE_ENABLE,
    .file_path      = LOG_FILE_PATH,
    .file_prefix    = LOG_FILE_PREFIX,
    .file_limitsize = LOG_FILE_LIMITSIZE,
    .file_mutex     = NULL,
};

static const char *log_prio_string[] = {
    [OS_LOG_FATAL]   = "F",
    [OS_LOG_ERROR]   = "E",
    [OS_LOG_WARN]    = "W",
    [OS_LOG_INFO]    = "I",
    [OS_LOG_DEBUG]   = "D",
    [OS_LOG_VERBOSE] = "V",
};

void os_logger_config(bool enable, enum os_logprio prio)
{
    log_config.enable = enable;
    log_config.prio = prio;
}

#if defined(ENABLE_LOG_SAVE)
static int file_size(const char *filename, size_t *size)
{
    struct stat statbuf;

    if (stat(filename, &statbuf) < 0)
        return -1;

    *size = statbuf.st_size;
    return 0;
}

static void os_logger_save(const char *data, size_t len)
{
    static bool init = false;
    static FILE *fp = NULL;
    static char filepath[64];
    size_t filesize;
    int ret;

    if (log_config.file_mutex == NULL) {
        if (log_config_mutex != NULL)
            OS_THREAD_MUTEX_LOCK(log_config_mutex);

        if (log_config.file_mutex == NULL)
            log_config.file_mutex = OS_THREAD_MUTEX_CREATE();

        if (log_config_mutex != NULL)
            OS_THREAD_MUTEX_UNLOCK(log_config_mutex);
    }

    OS_THREAD_MUTEX_LOCK(log_config.file_mutex);

    if (!init) {
        struct os_realtime ts;
        OS_TIMESTAMP_TO_LOCAL(&ts);

        memset(filepath, 0x0, sizeof(filepath));
        snprintf(filepath, sizeof(filepath),
                 "%s/%s-%04d%02d%02d-%02d%02d%02d.txt",
                 log_config.file_path, log_config.file_prefix,
                 ts.year, ts.mon, ts.day, ts.hour, ts.min, ts.sec
        );

        fp = fopen(filepath, "w+");
        init = true;
    }

    ret = file_size(filepath, &filesize);
    if (ret < 0) {
        fprintf(stderr, "Failed to get file size\n");
        goto out;
    }
    if (filesize > log_config.file_limitsize) {
        fprintf(stderr, "Max size occur, remove the old log\n");
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        remove(filepath);
        init = false;
        goto out;
    }

    if (fp != NULL && data != NULL && len > 0) {
        fwrite(data, len, 1, fp);
        fflush(fp);
    }

out:
    OS_THREAD_MUTEX_UNLOCK(log_config.file_mutex);
}
#endif

static void os_logger_print(enum os_logprio prio, const char *tag,
                            const char *func, unsigned int line,
                            const char *format, va_list arg_ptr)
{
    size_t offset = 0;
    int arg_size = 0;
    char log_entry[LOG_BUFFER_SIZE];
    size_t valid_size = LOG_BUFFER_SIZE - 2;

    struct os_realtime ts;
    // add data & time to header
    OS_TIMESTAMP_TO_LOCAL(&ts);
    if ((int)(valid_size - offset) > 0)
        offset += snprintf(log_entry + offset, valid_size - offset,
                           "%4d-%02d-%02d %02d:%02d:%02d:%03d",
                           ts.year, ts.mon, ts.day, ts.hour, ts.min, ts.sec, ts.msec);

    // add priority to header
    if ((int)(valid_size - offset) > 0)
        offset += snprintf(log_entry + offset, valid_size - offset,
                           " %s", log_prio_string[prio]);

    // add tag, function and line to header
    if ((int)(valid_size - offset) > 0)
        offset += snprintf(log_entry + offset, valid_size - offset,
                           " %s:%s:%u: ", tag, func, line);

    if ((int)(valid_size - offset) > 0) {
        arg_size = vsnprintf(log_entry + offset, valid_size - offset,
                             format, arg_ptr);
        if (arg_size > 0) {
            offset += arg_size;
            if (offset > valid_size)
                offset = valid_size - 1;
        }
        log_entry[offset++] = '\n';
        log_entry[offset] = '\0';
    }
    else {
        offset = valid_size - 1;
        log_entry[offset++] = '\n';
        log_entry[offset] = '\0';
    }

    // print log to console
    fprintf(stdout, "%s", log_entry);

#if defined(ENABLE_LOG_SAVE)
    // save log to file if need
    if (log_config.file_enable)
        os_logger_save(log_entry, offset);
#endif
}

void os_logger_trace(enum os_logprio prio, const char *tag,
                     const char *func, unsigned int line,
                     const char *format, ...)
{
    if (!log_config.enable || prio > log_config.prio)
        return;

    va_list arg_ptr;
    va_start(arg_ptr, format);
    os_logger_print(prio, tag, func, line, format, arg_ptr);
    va_end(arg_ptr);
}
