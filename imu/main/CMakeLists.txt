set(srcs "app_main.c"
         "st7789_lcd.c"
         "lvgl_port_intf.c"
         "my_lvgl_app.c"
         "imu_task.c")

set(includes ".")


idf_component_register(SRCS ${srcs}
                       REQUIRES imu esp_app_format esp_lcd esp_driver_spi esp_driver_tsens
                       INCLUDE_DIRS ${includes})
