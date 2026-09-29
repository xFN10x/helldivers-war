#include <pebble.h>

#include "loading_win.h"

static void message_recieved(DictionaryIterator *iterator, void *context)
{
    APP_LOG(APP_LOG_LEVEL_INFO, "Got message: %d bytes", dict_size(iterator));
    Tuple *ready = dict_find(iterator, MESSAGE_KEY_ready);
    Tuple *HBPing = dict_find(iterator, MESSAGE_KEY_HBPing);

    if (ready)
    {
        HN_Win_Loading_JSReady();
    }
}

static void message_dropped(AppMessageResult reason, void *context)
{
    // A message was received, but had to be dropped
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int)reason);
}

static void message_sent(DictionaryIterator *iter, void *context)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "A message has been sent.");
}

static void message_send_fail(DictionaryIterator *iter,
                              AppMessageResult reason, void *context)
{
    // The message just sent failed to be delivered
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message send failed. Reason: %d", (int)reason);
}

void msg_init()
{
    app_message_register_inbox_received(message_recieved);
    app_message_register_inbox_dropped(message_dropped);

    app_message_register_outbox_sent(message_sent);
    app_message_register_outbox_failed(message_send_fail);

    AppMessageResult res = app_message_open(2048, 128);
    if (res != APP_MSG_OK)
    {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to open messages! %d", (int)res);
        window_stack_pop_all(true);
    }
}