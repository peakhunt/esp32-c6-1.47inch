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

static lv_obj_t* _pages[] = 
{
  NULL,
  NULL,
};

//
// Page 0 Data
//
static lv_obj_t* page0_lbl_c1;
static lv_obj_t* page0_lbl_c2;
static lv_obj_t* page0_lbl_c3;
static lv_obj_t* page0_lbl_c4;
static lv_obj_t* page0_lbl_a1;
static lv_obj_t* page0_lbl_a2;

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

  sprintf(buf, "%d", w1);
  lv_label_set_text(page0_lbl_c1, buf);

  sprintf(buf, "%d", w2);
  lv_label_set_text(page0_lbl_c2, buf);

  sprintf(buf, "%d", w3);
  lv_label_set_text(page0_lbl_c3, buf);

  sprintf(buf, "%d", w4);
  lv_label_set_text(page0_lbl_c4, buf);

  sprintf(buf, "%d", w5);
  lv_label_set_text(page0_lbl_a1, buf);

  sprintf(buf, "%d", w6);
  lv_label_set_text(page0_lbl_a2, buf);

  lvgl_port_unlock();
}

static void
my_lvgl_app_init_page1(void)
{
#define LABEL_NUM_WIDTH     58
#define LABEL_UNIT_WIDTH    22

  lv_obj_t * bg = lv_img_create(lv_scr_act());
  lv_img_set_src(bg, &page1_ui);
  lv_obj_set_pos(bg, 0, 0);

  /* Create a style for white text */
  static lv_style_t style_white;
  lv_style_init(&style_white);
  lv_style_set_text_color(&style_white, lv_color_white());
  lv_style_set_text_font(&style_white, &lv_font_montserrat_20);

  lv_obj_t* lbl_c1 = lv_label_create(bg);
  lv_label_set_text(lbl_c1, "100");
  lv_obj_add_style(lbl_c1, &style_white, 0);
  lv_obj_set_width(lbl_c1, LABEL_NUM_WIDTH);
  lv_obj_set_style_text_align(lbl_c1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c1, 5, 67);

  lv_obj_t* unit_c1 = lv_label_create(bg);
  lv_label_set_text(unit_c1, "W");
  lv_obj_add_style(unit_c1, &style_white, 0);
  lv_obj_set_width(unit_c1, LABEL_UNIT_WIDTH);
  lv_obj_set_style_text_align(unit_c1, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(unit_c1, 54, 67);

  lv_obj_t* lbl_c2 = lv_label_create(bg);
  lv_label_set_text(lbl_c2, "0");
  lv_obj_add_style(lbl_c2, &style_white, 0);
  lv_obj_set_width(lbl_c2, LABEL_NUM_WIDTH);
  lv_obj_set_style_text_align(lbl_c2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c2, 91, 67);

  lv_obj_t* unit_c2 = lv_label_create(bg);
  lv_label_set_text(unit_c2, "W");
  lv_obj_add_style(unit_c2, &style_white, 0);
  lv_obj_set_width(unit_c2, LABEL_UNIT_WIDTH);
  lv_obj_set_style_text_align(unit_c1, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(unit_c2, 139, 67);

  lv_obj_t* lbl_c3 = lv_label_create(bg);
  lv_label_set_text(lbl_c3, "0");
  lv_obj_add_style(lbl_c3, &style_white, 0);
  lv_obj_set_width(lbl_c3, LABEL_NUM_WIDTH);
  lv_obj_set_style_text_align(lbl_c3, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c3, 5, 162);

  lv_obj_t* unit_c3 = lv_label_create(bg);
  lv_label_set_text(unit_c3, "W");
  lv_obj_add_style(unit_c3, &style_white, 0);
  lv_obj_set_width(unit_c3, LABEL_UNIT_WIDTH);
  lv_obj_set_style_text_align(unit_c3, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(unit_c3, 54, 162);

  lv_obj_t* lbl_c4 = lv_label_create(bg);
  lv_label_set_text(lbl_c4, "0");
  lv_obj_add_style(lbl_c4, &style_white, 0);
  lv_obj_set_width(lbl_c4, LABEL_NUM_WIDTH);
  lv_obj_set_style_text_align(lbl_c4, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_c4, 91, 162);

  lv_obj_t* unit_c4 = lv_label_create(bg);
  lv_label_set_text(unit_c4, "W");
  lv_obj_add_style(unit_c4, &style_white, 0);
  lv_obj_set_width(unit_c4, LABEL_UNIT_WIDTH);
  lv_obj_set_style_text_align(unit_c4, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(unit_c4, 139, 162);

  lv_obj_t* lbl_a1 = lv_label_create(bg);
  lv_label_set_text(lbl_a1, "0");
  lv_obj_add_style(lbl_a1, &style_white, 0);
  lv_obj_set_width(lbl_a1, LABEL_NUM_WIDTH);
  lv_obj_set_style_text_align(lbl_a1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_a1, 5, 256);

  lv_obj_t* unit_a1 = lv_label_create(bg);
  lv_label_set_text(unit_a1, "W");
  lv_obj_add_style(unit_a1, &style_white, 0);
  lv_obj_set_width(unit_a1, LABEL_UNIT_WIDTH);
  lv_obj_set_style_text_align(unit_a1, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(unit_a1, 54, 256); 

  lv_obj_t* lbl_a2 = lv_label_create(bg);
  lv_label_set_text(lbl_a2, "0");
  lv_obj_add_style(lbl_a2, &style_white, 0);
  lv_obj_set_width(lbl_a2, LABEL_NUM_WIDTH);
  lv_obj_set_style_text_align(lbl_a2, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(lbl_a2, 91, 256);

  lv_obj_t* unit_a2 = lv_label_create(bg);
  lv_label_set_text(unit_a2, "W");
  lv_obj_add_style(unit_a2, &style_white, 0);
  lv_obj_set_width(unit_a2, LABEL_UNIT_WIDTH);
  lv_obj_set_style_text_align(unit_a1, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(unit_a2, 139, 256);

  _pages[0] = bg;
  page0_lbl_c1 = lbl_c1;
  page0_lbl_c2 = lbl_c2;
  page0_lbl_c3 = lbl_c3;
  page0_lbl_c4 = lbl_c4;
  page0_lbl_a1 = lbl_a1;
  page0_lbl_a2 = lbl_a2;

  lv_obj_add_flag(_pages[0], LV_OBJ_FLAG_HIDDEN);
}

static void
my_lvgl_app_show_page1(bool show)
{
  if(show)
  {
    lv_obj_clear_flag(_pages[0], LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    lv_obj_add_flag(_pages[0], LV_OBJ_FLAG_HIDDEN);
  }
}

void
my_lvgl_app_init(void)
{
  lvgl_port_lock(0);

  my_lvgl_app_init_page1();
  my_lvgl_app_show_page1(true);

  /* Seed random generator */
  srand(time(NULL));

  /* Create a timer to update every second */
  lv_timer_create(watt_simulator_cb, 1000, NULL);

  lvgl_port_unlock();
}
