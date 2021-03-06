#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "cutils/os_memory.h"
#include "cutils/os_logger.h"
#include "cutils/os_thread.h"
#include "cutils/msglooper.h"

#define LOG_TAG "msglooper_test"

struct priv_data {
    const char *str;
};

static void msg_handle(struct message *arg)
{
    struct message *msg = arg;
    struct priv_data *priv = msg->data;
    OS_LOGD(LOG_TAG, "--> Handle message: what=[%d], str=[%s]", msg->what, priv->str);
}

static void msg_free(struct message *msg)
{
    struct priv_data *priv = msg->data;
    OS_LOGD(LOG_TAG, "--> Free message: what=[%d], str=[%s]", msg->what, priv->str);
    OS_FREE(priv->str);
    OS_FREE(msg->data);
}

static void msg_timeout(struct message *msg)
{
    OS_LOGE(LOG_TAG, "--> Timeout message: what=[%d]", msg->what);
}

int main()
{
    struct os_threadattr attr;
    mlooper_t looper;
    struct message *msg;
    struct priv_data *priv;

    attr.name = "msglooper_test";
    attr.priority = OS_THREAD_PRIO_NORMAL;
    attr.stacksize = 1024;
    looper = mlooper_create(&attr, msg_handle, msg_free);

    mlooper_dump(looper);

    mlooper_start(looper);

    {
        priv = OS_MALLOC(sizeof(struct  priv_data));
        priv->str = OS_STRDUP("mlooper_post_message_delay");
        // timeout 2s
        msg = message_obtain2(1000, 0, 0, priv, 2000, NULL, NULL, msg_timeout);
        mlooper_post_message_delay(looper, msg, 2000); // delay 2s
    }

    {
        for (int i = 0; i < 3; i++) {
            priv = OS_MALLOC(sizeof(struct  priv_data));
            priv->str = OS_STRDUP("mlooper_post_message");
            msg = message_obtain(i+100, 0, 0, priv);
            mlooper_post_message(looper, msg);
        }
    }

    {
        priv = OS_MALLOC(sizeof(struct  priv_data));
        priv->str = OS_STRDUP("mlooper_post_message_front");
        msg = message_obtain(0, 0, 0, priv);
        mlooper_post_message_front(looper, msg);
    }

    mlooper_dump(looper);
    //OS_LOGW(LOG_TAG, "-->Dump memory after post message");
    //OS_MEMORY_DUMP();

    //OS_LOGI(LOG_TAG, "remove what=1000");
    //mlooper_remove_message(looper, 1000);
    //mlooper_dump(looper);

    //mlooper_stop(looper);
    //mlooper_dump(looper);

    OS_THREAD_SLEEP_MSEC(3000);
    mlooper_destroy(looper);

    //OS_LOGW(LOG_TAG, "-->Dump memory after destroy mlooper");
    //OS_MEMORY_DUMP();
    return 0;
}
