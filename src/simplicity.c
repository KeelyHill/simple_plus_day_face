#include "pebble.h"

static Window *s_main_window;
static TextLayer *s_date_layer, *s_day_layer, *s_time_layer;
static Layer *s_line_layer;

#define FRONT_COLOR GColorWhite
#define BACKGROUND_COLOR GColorBlack
  
// For black text on white background
// #define FRONT_COLOR GColorBlack
// #define BACKGROUND_COLOR GColorWhite

static void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, FRONT_COLOR);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char s_time_text[] = "00:00";
  static char s_date_text[] = "Month 00";
  static char s_day_text[] = "day";

  strftime(s_date_text, sizeof(s_date_text), "%B%e", tick_time);
  text_layer_set_text(s_date_layer, s_date_text);
  
  strftime(s_day_text, sizeof(s_day_text), "%a", tick_time);
  text_layer_set_text(s_day_layer, s_day_text);

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);

  // Handle lack of non-padded hour format string for twelve hour clock.
  if (!clock_is_24h_style() && (s_time_text[0] == '0')) {
    memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
  }
  text_layer_set_text(s_time_layer, s_time_text);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  #define UP 50
  #define END_WIDTH window_bounds.size.w - 15

  // Date Layer (Month 00)
  s_date_layer = text_layer_create(GRect(5, UP, END_WIDTH, 50));
  text_layer_set_text_color(s_date_layer, FRONT_COLOR);
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  // Day Layer (Sun-Sat)
  s_day_layer = text_layer_create(GRect(5, UP, END_WIDTH, 50));
  text_layer_set_text_color(s_day_layer, FRONT_COLOR);
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentRight);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
  
  // Time Layer
  s_time_layer = text_layer_create(GRect(5, 24 + UP, END_WIDTH, 76));
  text_layer_set_text_color(s_time_layer, FRONT_COLOR);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  GRect line_frame = GRect(5, 29 + UP, END_WIDTH, 2);
  s_line_layer = layer_create(line_frame);
  layer_set_update_proc(s_line_layer, line_layer_update_callback);
  layer_add_child(window_layer, s_line_layer);
}


static void main_window_unload(Window *window) {
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_time_layer);

  layer_destroy(s_line_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, BACKGROUND_COLOR);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  
  // Prevent starting blank
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  handle_minute_tick(t, MINUTE_UNIT);
}

static void deinit() {
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}