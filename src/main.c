#include "mgos.h"
#include "mgos_telegram.h"
#include "mgos_gpio.h"
#include "mgos_config.h"
#include "mgos_event.h"
#include "mgos_wifi.h"

#define LED_PIN 2
#define LED_ON 1
#define LED_OFF 0

uint32_t chat_id = 0;

void app_start_handler(int ev, void *ev_data, void *userdata);

void led_on(void *ev_data, void *userdata);
void led_off(void *ev_data, void *userdata);
void start_bot_handler(void *, void *);
void button_handler(int pin, void *data);

void wifi_connect_handler(int ev, void *ev_data, void *userdata);

// ***********************************************************************

void app_start_handler(int ev, void *ev_data, void *userdata)
{
  (void)ev;
  (void)userdata;
  (void)ev_data;

  // это работает
  mgos_telegram_subscribe("/on", led_on, NULL);
  mgos_telegram_subscribe("/off", led_off, NULL);
  mgos_telegram_subscribe("/start", start_bot_handler, NULL);
}

void led_on(void *ev_data, void *userdata)
{
  (void)ev_data;
  (void)userdata;

  mgos_gpio_write(LED_PIN, LED_ON);
}

void led_off(void *ev_data, void *userdata)
{
  (void)ev_data;
  (void)userdata;

  mgos_gpio_write(LED_PIN, LED_OFF);
}

// APP INIT FN
enum mgos_app_init_result mgos_app_init(void)
{

  mgos_event_add_handler(MGOS_WIFI_EV_STA_CONNECTED, wifi_connect_handler, NULL);
  chat_id = mgos_sys_config_get_mmm_chat_id();

  LOG(LL_INFO, ("CHATID = %d", chat_id));

  mgos_gpio_set_mode(LED_PIN, MGOS_GPIO_MODE_OUTPUT);
  mgos_event_add_handler(TGB_EV_CONNECTED, app_start_handler, NULL);
  mgos_gpio_set_button_handler(
      0,
      MGOS_GPIO_PULL_DOWN,
      MGOS_GPIO_INT_EDGE_POS,
      50,
      button_handler,
      NULL);
  return MGOS_APP_INIT_SUCCESS;
};

void start_bot_handler(void *ev_data, void *user_data)
{
  (void)user_data;

  char **msg = NULL;

  if (!chat_id && ev_data != NULL)
  {
    struct mgos_telegram_update *update = (struct mgos_telegram_update *)ev_data;
    mgos_sys_config_set_mmm_chat_id(update->chat_id);
    save_cfg(&mgos_sys_config, msg);

    chat_id = update->chat_id;

    if (msg != NULL)
    {
      LOG(LL_INFO, ("SAVE CONFIG ERROR: %s", *msg));
      free(msg);
    }

    mgos_telegram_send_message(chat_id, "Hello :)");
  }
}

void button_handler(int pin, void *data)
{
  if (chat_id)
    mgos_telegram_send_message(chat_id, "Вы нажали на кнопошку :)");
  else
    LOG(LL_INFO, ("\n\nCHAT_ID == NULL\n\rSend /start command.\n\r"));
}

void wifi_connect_handler(int ev, void *ev_data, void *userdata)
{
  (void)ev;
  (void)ev_data;
  (void)userdata;

  mgos_gpio_write(LED_PIN, LED_ON);
}