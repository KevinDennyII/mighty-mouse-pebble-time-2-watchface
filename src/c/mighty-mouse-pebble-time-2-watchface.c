#include <pebble.h>

// --- Layer Declarations ---
static Window *s_main_window;
static BitmapLayer *s_background_layer, *s_icon_layer;
static GBitmap *s_background_bitmap, *s_icon_bitmap;

// Text layers for the "outlined text" effect
static TextLayer *s_time_layer_outline, *s_time_layer_text;
static TextLayer *s_date_layer_outline, *s_date_layer_text;
static TextLayer *s_weather_layer_outline, *s_weather_layer_text;
static TextLayer *s_steps_layer_outline, *s_steps_layer_text;
static TextLayer *s_hr_layer_outline, *s_hr_layer_text;
static TextLayer *s_battery_layer_outline, *s_battery_layer_text;

// --- Buffers and State ---
static char s_time_buffer[8], s_date_buffer[16];
static char s_weather_buffer[32], s_steps_buffer[16], s_hr_buffer[16], s_battery_buffer[16];

// --- Update Functions ---

// Creates a pair of text layers to simulate an outline
static void create_outlined_text(TextLayer **outline_layer, TextLayer **text_layer, GRect bounds, const char *font_key, GTextAlignment align, Layer *parent) {
  // Create black outline layer
  *outline_layer = text_layer_create(GRect(bounds.origin.x - 1, bounds.origin.y - 1, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(*outline_layer, GColorClear);
  text_layer_set_text_color(*outline_layer, GColorBlack);
  text_layer_set_font(*outline_layer, fonts_get_system_font(font_key));
  text_layer_set_text_alignment(*outline_layer, align);
  layer_add_child(parent, text_layer_get_layer(*outline_layer));

  // Create white text layer
  *text_layer = text_layer_create(bounds);
  text_layer_set_background_color(*text_layer, GColorClear);
  text_layer_set_text_color(*text_layer, GColorWhite);
  text_layer_set_font(*text_layer, fonts_get_system_font(font_key));
  text_layer_set_text_alignment(*text_layer, align);
  layer_add_child(parent, text_layer_get_layer(*text_layer));
}

// Updates the text on an outlined text layer pair
static void update_outlined_text(TextLayer *outline_layer, TextLayer *text_layer, const char *new_text) {
  text_layer_set_text(outline_layer, new_text);
  text_layer_set_text(text_layer, new_text);
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d %b", tick_time);

  update_outlined_text(s_time_layer_outline, s_time_layer_text, s_time_buffer);
  update_outlined_text(s_date_layer_outline, s_date_layer_text, s_date_buffer);
}

static void battery_handler(BatteryChargeState charge) {
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge.charge_percent);
  if (s_battery_layer_text) {
    update_outlined_text(s_battery_layer_outline, s_battery_layer_text, s_battery_buffer);
  }
}

static void health_handler(HealthEventType event, void *context) {
  if (event == HealthEventMovementUpdate) {
    snprintf(s_steps_buffer, sizeof(s_steps_buffer), "%d steps", (int)health_service_sum_today(HealthMetricStepCount));
    if (s_steps_layer_text) {
      update_outlined_text(s_steps_layer_outline, s_steps_layer_text, s_steps_buffer);
    }
  }
  if (event == HealthEventHeartRateUpdate) {
    snprintf(s_hr_buffer, sizeof(s_hr_buffer), "%d bpm", (int)health_service_peek_current_value(HealthMetricHeartRateBPM));
     if (s_hr_layer_text) {
      update_outlined_text(s_hr_layer_outline, s_hr_layer_text, s_hr_buffer);
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }

  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_KEY_TEMPERATURE);
  Tuple *cond_tuple = dict_find(iterator, MESSAGE_KEY_KEY_CONDITIONS);

  if (temp_tuple && cond_tuple) {
    snprintf(s_weather_buffer, sizeof(s_weather_buffer), "%s, %s", temp_tuple->value->cstring, cond_tuple->value->cstring);
    update_outlined_text(s_weather_layer_outline, s_weather_layer_text, s_weather_buffer);

    char *conditions = cond_tuple->value->cstring;
    if (strstr(conditions, "Sun") != NULL || strstr(conditions, "Clear") != NULL) {
      s_icon_bitmap = gbitmap_create_with_resource(IMAGE_WEATHER_SUNNY);
    } else if (strstr(conditions, "Cloud") != NULL) {
      s_icon_bitmap = gbitmap_create_with_resource(IMAGE_WEATHER_CLOUDY);
    } else if (strstr(conditions, "Rain") != NULL) {
      s_icon_bitmap = gbitmap_create_with_resource(IMAGE_WEATHER_RAINY);
    } else if (strstr(conditions, "Snow") != NULL) {
      s_icon_bitmap = gbitmap_create_with_resource(IMAGE_WEATHER_SNOWY);
    } else {
      s_icon_bitmap = gbitmap_create_with_resource(IMAGE_WEATHER_UNKNOWN);
    }
    bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
  }
}

// --- Window Load and Unload ---

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Background
  s_background_bitmap = gbitmap_create_with_resource(IMAGE_MIGHTY_MOUSE);
  s_background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Time (Large, centered)
  create_outlined_text(&s_time_layer_outline, &s_time_layer_text, GRect(0, 68, bounds.size.w, 50), FONT_KEY_LECO_42_NUMBERS, GTextAlignmentCenter, window_layer);

  // Date (Below time)
  create_outlined_text(&s_date_layer_outline, &s_date_layer_text, GRect(0, 115, bounds.size.w, 30), FONT_KEY_GOTHIC_24_BOLD, GTextAlignmentCenter, window_layer);

  // Weather Text (Top)
  create_outlined_text(&s_weather_layer_outline, &s_weather_layer_text, GRect(0, 8, bounds.size.w, 25), FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentCenter, window_layer);
  update_outlined_text(s_weather_layer_outline, s_weather_layer_text, "Loading...");
  
  // Weather Icon
  s_icon_layer = bitmap_layer_create(GRect(0, 32, bounds.size.w, 30));
  bitmap_layer_set_background_color(s_icon_layer, GColorClear);
  bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_icon_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));

  // Trackers (Bottom)
  if (persist_read_bool(MESSAGE_KEY_KEY_SHOW_STEPS)) {
    create_outlined_text(&s_steps_layer_outline, &s_steps_layer_text, GRect(5, bounds.size.h - 28, bounds.size.w, 25), FONT_KEY_GOTHIC_18, GTextAlignmentLeft, window_layer);
  }
  if (persist_read_bool(MESSAGE_KEY_KEY_SHOW_HEART_RATE)) {
    create_outlined_text(&s_hr_layer_outline, &s_hr_layer_text, GRect(-5, bounds.size.h - 28, bounds.size.w, 25), FONT_KEY_GOTHIC_18, GTextAlignmentRight, window_layer);
  }
  if (persist_read_bool(MESSAGE_KEY_KEY_SHOW_BATTERY)) {
    create_outlined_text(&s_battery_layer_outline, &s_battery_layer_text, GRect(0, 145, bounds.size.w, 25), FONT_KEY_GOTHIC_18_BOLD, GTextAlignmentCenter, window_layer);
  }
  
  // Get initial state
  update_time();
  battery_handler(battery_state_service_peek());
  health_handler(HealthEventMovementUpdate, NULL);
  health_handler(HealthEventHeartRateUpdate, NULL);
}

static void main_window_unload(Window *window) {
  // Destroy all layers to prevent memory leaks
  text_layer_destroy(s_time_layer_text);
  text_layer_destroy(s_time_layer_outline);
  text_layer_destroy(s_date_layer_text);
  text_layer_destroy(s_date_layer_outline);
  text_layer_destroy(s_weather_layer_text);
  text_layer_destroy(s_weather_layer_outline);
  
  if(s_steps_layer_text) { text_layer_destroy(s_steps_layer_text); }
  if(s_steps_layer_outline) { text_layer_destroy(s_steps_layer_outline); }
  if(s_hr_layer_text) { text_layer_destroy(s_hr_layer_text); }
  if(s_hr_layer_outline) { text_layer_destroy(s_hr_layer_outline); }
  if(s_battery_layer_text) { text_layer_destroy(s_battery_layer_text); }
  if(s_battery_layer_outline) { text_layer_destroy(s_battery_layer_outline); }
  
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);
  
  if(s_icon_bitmap) { gbitmap_destroy(s_icon_bitmap); }
  bitmap_layer_destroy(s_icon_layer);
}

// --- Init and Deinit ---

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  // Register services
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  health_service_events_subscribe(health_handler, NULL);
  
  // Register AppMessage
  app_message_register_inbox_received(inbox_received_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
