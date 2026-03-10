#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "esp_lvgl_port.h"
#include "esp_check.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

LV_IMG_DECLARE(page1_ui)

static lv_obj_t * lbl_c1;
static lv_obj_t * lbl_c2;
static lv_obj_t * lbl_c3;
static lv_obj_t * lbl_c4;
static lv_obj_t * lbl_a1;
static lv_obj_t * lbl_a2;

/* Timer callback to update Watt values */
static void
watt_simulator_cb(lv_timer_t * timer)
{
  lvgl_port_lock(0);

  char buf[16];

  int w1 = rand() % 141;  // 0–140
  int w2 = rand() % 141;
  int w3 = rand() % 141;
  int w4 = rand() % 141;
  int w5 = rand() % 141;
  int w6 = rand() % 141;

  sprintf(buf, "%d W", w1);
  lv_label_set_text(lbl_c1, buf);

  sprintf(buf, "%d W", w2);
  lv_label_set_text(lbl_c2, buf);

  sprintf(buf, "%d W", w3);
  lv_label_set_text(lbl_c3, buf);

  sprintf(buf, "%d W", w4);
  lv_label_set_text(lbl_c4, buf);

  sprintf(buf, "%d W", w5);
  lv_label_set_text(lbl_a1, buf);

  sprintf(buf, "%d W", w6);
  lv_label_set_text(lbl_a2, buf);

  lvgl_port_unlock();
}

static void
my_lvgl_app_page_all_watts_init(void)
{
  lv_obj_t * bg = lv_img_create(lv_scr_act());
  lv_img_set_src(bg, &page1_ui);
  lv_obj_set_pos(bg, 0, 0);

  /* Create a style for white text */
  static lv_style_t style_white;
  lv_style_init(&style_white);
  lv_style_set_text_color(&style_white, lv_color_white());
  lv_style_set_text_font(&style_white, &lv_font_montserrat_20);  // built-in font at size 20

  lbl_c1 = lv_label_create(bg);
  lv_label_set_text(lbl_c1, "100 W");
  lv_obj_add_style(lbl_c1, &style_white, 0);
  lv_obj_set_width(lbl_c1, 80);   // adjust width to match your cell
  lv_obj_set_style_text_align(lbl_c1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c1, 5, 67);   // measured coords for C1

  lbl_c2 = lv_label_create(bg);
  lv_label_set_text(lbl_c2, "0 W");
  lv_obj_add_style(lbl_c2, &style_white, 0);
  lv_obj_set_width(lbl_c2, 80);   // adjust width to match your cell
  lv_obj_set_style_text_align(lbl_c2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c2, 91, 67);

  lbl_c3 = lv_label_create(bg);
  lv_label_set_text(lbl_c3, "0 W");
  lv_obj_add_style(lbl_c3, &style_white, 0);
  lv_obj_set_width(lbl_c3, 80);   // adjust width to match your cell
  lv_obj_set_style_text_align(lbl_c3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c3, 5, 162);

  lbl_c4 = lv_label_create(bg);
  lv_label_set_text(lbl_c4, "0 W");
  lv_obj_add_style(lbl_c4, &style_white, 0);
  lv_obj_set_width(lbl_c4, 80);   // adjust width to match your cell
  lv_obj_set_style_text_align(lbl_c4, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c4, 91, 162);

  lbl_a1 = lv_label_create(bg);
  lv_label_set_text(lbl_a1, "0 W");
  lv_obj_add_style(lbl_a1, &style_white, 0);
  lv_obj_set_width(lbl_a1, 80);   // adjust width to match your cell
  lv_obj_set_style_text_align(lbl_a1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_a1, 5, 256);

  lbl_a2 = lv_label_create(bg);
  lv_label_set_text(lbl_a2, "0 W");
  lv_obj_add_style(lbl_a2, &style_white, 0);
  lv_obj_set_width(lbl_a2, 80);   // adjust width to match your cell
  lv_obj_set_style_text_align(lbl_a2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_a2, 91, 256);
}

void
my_lvgl_app_init(void)
{
  lvgl_port_lock(0);

  my_lvgl_app_page_all_watts_init();

  /* Seed random generator */
  srand(time(NULL));

  /* Create a timer to update every second */
  lv_timer_create(watt_simulator_cb, 1000, NULL);

  lvgl_port_unlock();
}
