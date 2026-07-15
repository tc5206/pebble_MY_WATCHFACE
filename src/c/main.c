#include <pebble.h>
#include <stdlib.h>

#define STORAGE_KEY_BACKGROUND_COLOR 1
#define STORAGE_KEY_TIME_X 2
#define STORAGE_KEY_TIME_Y 3
#define STORAGE_KEY_DATE_X 4
#define STORAGE_KEY_DATE_Y 5
#define STORAGE_KEY_DAY_X  6
#define STORAGE_KEY_DAY_Y  7
#define STORAGE_KEY_TIME_COLOR 8
#define STORAGE_KEY_DATE_COLOR 9
#define STORAGE_KEY_DAY_COLOR  10

static Window *s_main_window;
static BitmapLayer *s_image_layer;
static GBitmap *s_background_bitmap;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_day_layer;

// 設定変数
static GColor s_background_color;
static int s_time_x, s_time_y;
static int s_date_x, s_date_y;
static int s_day_x, s_day_y;
static GColor s_time_color;
static GColor s_date_color;
static GColor s_day_color;

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  static char s_date_buffer[8];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%m/%d", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);

  static char s_day_buffer[4];
  strftime(s_day_buffer, sizeof(s_day_buffer), "%a", tick_time);
  text_layer_set_text(s_day_layer, s_day_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  light_enable_interaction();
}

static void inbox_received_handler(DictionaryIterator *iterator, void *context) {
  // 背景色の受信
  Tuple *bg_color_tuple = dict_find(iterator, MESSAGE_KEY_BackgroundColor);
  if (bg_color_tuple) {
    uint32_t color_value = bg_color_tuple->value->uint32;
    persist_write_int(STORAGE_KEY_BACKGROUND_COLOR, color_value);
    s_background_color = GColorFromHEX(color_value);
    if (s_main_window) {
      window_set_background_color(s_main_window, s_background_color);
    }
  }

  // 時刻設定（座標・文字色）の受信
  Tuple *time_x_tuple = dict_find(iterator, MESSAGE_KEY_TimeX);
  if (time_x_tuple) {
    s_time_x = atoi(time_x_tuple->value->cstring);
    persist_write_int(STORAGE_KEY_TIME_X, s_time_x);
  }
  Tuple *time_y_tuple = dict_find(iterator, MESSAGE_KEY_TimeY);
  if (time_y_tuple) {
    s_time_y = atoi(time_y_tuple->value->cstring);
    persist_write_int(STORAGE_KEY_TIME_Y, s_time_y);
  }
  if ((time_x_tuple || time_y_tuple) && s_time_layer) {
    layer_set_frame(text_layer_get_layer(s_time_layer), GRect(s_time_x, s_time_y, 200, 60));
  }
  Tuple *time_color_tuple = dict_find(iterator, MESSAGE_KEY_TimeColor);
  if (time_color_tuple) {
    uint32_t color_value = time_color_tuple->value->uint32;
    persist_write_int(STORAGE_KEY_TIME_COLOR, color_value);
    s_time_color = GColorFromHEX(color_value);
    if (s_time_layer) {
      text_layer_set_text_color(s_time_layer, s_time_color);
    }
  }

  // 日付設定（座標・文字色）の受信
  Tuple *date_x_tuple = dict_find(iterator, MESSAGE_KEY_DateX);
  if (date_x_tuple) {
    s_date_x = atoi(date_x_tuple->value->cstring);
    persist_write_int(STORAGE_KEY_DATE_X, s_date_x);
  }
  Tuple *date_y_tuple = dict_find(iterator, MESSAGE_KEY_DateY);
  if (date_y_tuple) {
    s_date_y = atoi(date_y_tuple->value->cstring);
    persist_write_int(STORAGE_KEY_DATE_Y, s_date_y);
  }
  if ((date_x_tuple || date_y_tuple) && s_date_layer) {
    layer_set_frame(text_layer_get_layer(s_date_layer), GRect(s_date_x, s_date_y, 200, 40));
  }
  Tuple *date_color_tuple = dict_find(iterator, MESSAGE_KEY_DateColor);
  if (date_color_tuple) {
    uint32_t color_value = date_color_tuple->value->uint32;
    persist_write_int(STORAGE_KEY_DATE_COLOR, color_value);
    s_date_color = GColorFromHEX(color_value);
    if (s_date_layer) {
      text_layer_set_text_color(s_date_layer, s_date_color);
    }
  }

  // 曜日設定（座標・文字色）の受信
  Tuple *day_x_tuple = dict_find(iterator, MESSAGE_KEY_DayX);
  if (day_x_tuple) {
    s_day_x = atoi(day_x_tuple->value->cstring);
    persist_write_int(STORAGE_KEY_DAY_X, s_day_x);
  }
  Tuple *day_y_tuple = dict_find(iterator, MESSAGE_KEY_DayY);
  if (day_y_tuple) {
    s_day_y = atoi(day_y_tuple->value->cstring);
    persist_write_int(STORAGE_KEY_DAY_Y, s_day_y);
  }
  if ((day_x_tuple || day_y_tuple) && s_day_layer) {
    layer_set_frame(text_layer_get_layer(s_day_layer), GRect(s_day_x, s_day_y, 200, 30));
  }
  Tuple *day_color_tuple = dict_find(iterator, MESSAGE_KEY_DayColor);
  if (day_color_tuple) {
    uint32_t color_value = day_color_tuple->value->uint32;
    persist_write_int(STORAGE_KEY_DAY_COLOR, color_value);
    s_day_color = GColorFromHEX(color_value);
    if (s_day_layer) {
      text_layer_set_text_color(s_day_layer, s_day_color);
    }
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  // 1. 背景色の設定
  window_set_background_color(window, s_background_color);

  // 2. 画像リソース200x228の透過pngレイヤー
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_OVERLAY);
  s_image_layer = bitmap_layer_create(GRect(0, 0, 200, 228));
  bitmap_layer_set_bitmap(s_image_layer, s_background_bitmap);
  bitmap_layer_set_compositing_mode(s_image_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_image_layer));

  // 3. 曜日レイヤー
  s_day_layer = text_layer_create(GRect(s_day_x, s_day_y, 200, 30));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, s_day_color);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

  // 4. 日付レイヤー
  s_date_layer = text_layer_create(GRect(s_date_x, s_date_y, 200, 40));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, s_date_color);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // 5. 時刻レイヤー
  s_time_layer = text_layer_create(GRect(s_time_x, s_time_y, 200, 60));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, s_time_color);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  bitmap_layer_destroy(s_image_layer);
  gbitmap_destroy(s_background_bitmap);
}

static void init() {
  // 保存された設定の読み込みとデフォルト初期値の指定
  uint32_t default_bg_color = 65535; // Cyan
  if (persist_exists(STORAGE_KEY_BACKGROUND_COLOR)) {
    default_bg_color = (uint32_t)persist_read_int(STORAGE_KEY_BACKGROUND_COLOR);
  }
  s_background_color = GColorFromHEX(default_bg_color);

  s_time_x = persist_exists(STORAGE_KEY_TIME_X) ? persist_read_int(STORAGE_KEY_TIME_X) : 0;
  s_time_y = persist_exists(STORAGE_KEY_TIME_Y) ? persist_read_int(STORAGE_KEY_TIME_Y) : 40;

  s_date_x = persist_exists(STORAGE_KEY_DATE_X) ? persist_read_int(STORAGE_KEY_DATE_X) : 0;
  s_date_y = persist_exists(STORAGE_KEY_DATE_Y) ? persist_read_int(STORAGE_KEY_DATE_Y) : 110;

  s_day_x = persist_exists(STORAGE_KEY_DAY_X) ? persist_read_int(STORAGE_KEY_DAY_X) : 0;
  s_day_y = persist_exists(STORAGE_KEY_DAY_Y) ? persist_read_int(STORAGE_KEY_DAY_Y) : 160;

  uint32_t default_text_color = 0; // Black
  
  uint32_t time_color_val = persist_exists(STORAGE_KEY_TIME_COLOR) ? (uint32_t)persist_read_int(STORAGE_KEY_TIME_COLOR) : default_text_color;
  s_time_color = GColorFromHEX(time_color_val);

  uint32_t date_color_val = persist_exists(STORAGE_KEY_DATE_COLOR) ? (uint32_t)persist_read_int(STORAGE_KEY_DATE_COLOR) : default_text_color;
  s_date_color = GColorFromHEX(date_color_val);

  uint32_t day_color_val = persist_exists(STORAGE_KEY_DAY_COLOR) ? (uint32_t)persist_read_int(STORAGE_KEY_DAY_COLOR) : default_text_color;
  s_day_color = GColorFromHEX(day_color_val);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  update_time();
  
  // 各種サービスの購読
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  accel_tap_service_subscribe(tap_handler);

  // AppMessage通信の初期化
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(256, 256);
}

static void deinit() {
  accel_tap_service_unsubscribe();
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}