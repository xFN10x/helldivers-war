#include <pebble_worker.h>

static AppTimer *launch_timer;

static void launch(void *ctx)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Starting app.");
  worker_launch_app();
  launch_timer = app_timer_register(5000, launch, NULL);
}

static void prv_init()
{
  launch_timer = app_timer_register(5000, launch, NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "Worker running...");
}

int main(void)
{
  prv_init();
  worker_event_loop();
  if (launch_timer)
    app_timer_cancel(launch_timer);
}
