/**
* @brief This is part of the project 'LoRaTelemetryModules'
* @file 'test.c'
* Copyright (c) Vitaliy Nimych <vitaliy.nimych@gmail.com>
* Created 18.10.2021
* License-Identifier: ???
**/

#include <device.h>
#include <kernel.h>
#include <drivers/lora.h>
#include <errno.h>
#include <sys/util.h>
#include <zephyr.h>
#include <string.h>
#include <drivers/gpio.h>
#include "autoconf.h"
#include <drivers/pwm.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#define LED1_PIN	DT_GPIO_PIN(LED1_NODE, gpios)
#define LED1_FLAGS	DT_GPIO_FLAGS(LED1_NODE, gpios)

#define DEFAULT_RADIO_NODE DT_ALIAS(lora0)
BUILD_ASSERT(DT_NODE_HAS_STATUS(DEFAULT_RADIO_NODE, okay),
             "No default LoRa radio specified in DT");


#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(lora_test);

const struct device *lora_dev = DEVICE_DT_GET(DEFAULT_RADIO_NODE);

void RxError(void)
{
  LOG_ERR("LoRa RxError");
}

void TXDone(void)
{
  lora_set_mode(lora_dev, LORA_MODEM_RX);
  LOG_INF("TXDone");
}

void RxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
  printk("%s()\r\n",__FUNCTION__ );
  printk("Receive data: Payload: '%s' ",payload);
  printk("RSSI: '%ddBm' ",rssi);
  printk("SNR: '%ddb' \r\n",snr);
}

void main(void)
{
  struct lora_modem_config config = {0};
  int ret;
  int len;
  int16_t rssi;
  int8_t snr;
  
  if (!device_is_ready(lora_dev)) {
    LOG_ERR("%s Device not ready", lora_dev->name);
    return;
  }
  
  config.modulation = MODULATION_LORA;
  config.frequency = 865100000;
  config.bandwidth = BW_125_KHZ;
  config.datarate = SF_10;
  config.preamble_len = 8;
  config.coding_rate = CR_4_5;
  config.tx_power = 10;
  config.crc = false;
  config.RxError = RxError;
  config.TXDone = TXDone;
  //config.RxDone = RxDone;
  
  ret = lora_config(lora_dev, &config);
  if (ret < 0) {
    LOG_ERR("LoRa config failed");
    return;
  }
  lora_set_mode(lora_dev, LORA_MODEM_RX);
  
  char send_data[256] = {0};
  uint32_t cnt = 0;
  
  while (1) {
#ifdef CONFIG_BOARD_TELEM_MODULE_TX
    sprintf(send_data, "Hello cnt: '%lu'", cnt);
    LOG_INF("Start Data sent!");
    ret = lora_send(lora_dev, send_data, strlen(send_data));
    if (ret < 0) {
      LOG_ERR("LoRa send failed");
      return;
    }
    cnt++;
    /* Send data at 1s interval */
    k_sleep(K_MSEC(10000));

    LOG_INF("Data sent!");
#endif

#ifdef CONFIG_BOARD_TELEM_MODULE_RX

    /* Block until data arrives */
    memset(send_data, 0, 255);
    len = lora_recv(lora_dev, send_data, 255, K_MSEC(1000),
                    &rssi, &snr);
    if (len > 0) {
      //LOG_INF("Received data: %s (RSSI:%ddBm, SNR:%ddBm)", log_strdup(send_data), rssi, snr);
      printk("Received data: %s (RSSI:%ddBm, SNR:%ddBm)\r\n", send_data, rssi, snr);
    }
#endif
  
  }
}

void led_task(void)
{
  const struct device *leds[2];
  bool led_is_on = true;
  int ret;
  
  leds[0] = device_get_binding(LED0);
  leds[1] = device_get_binding(LED0);
  if (leds[0] == NULL || leds[1] == NULL) {
    return;
  }
  
  ret = gpio_pin_configure(leds[0], LED0_PIN, GPIO_OUTPUT_ACTIVE | LED0_FLAGS);
  if (ret < 0) {
    return;
  }
  ret = gpio_pin_configure(leds[1], LED1_PIN, GPIO_OUTPUT_ACTIVE | LED1_FLAGS);
  if (ret < 0) {
    return;
  }
  
  while (1) {
    gpio_pin_set(leds[0], LED0_PIN, (int)led_is_on);
    gpio_pin_set(leds[1], LED1_PIN, (int)!led_is_on);
    led_is_on = !led_is_on;
    k_msleep(1000);
  }
}

K_THREAD_DEFINE(led_task_id, 512, led_task, NULL, NULL, NULL,
                7, 0, 0);
