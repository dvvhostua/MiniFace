#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_date_layer;
static GFont s_date_font;
static TextLayer *s_time_layer;
static GFont s_time_font;
static TextLayer *s_week_layer[8]; // 0..6 - Mo..Su; 7 - current week number
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_weekdays(struct tm *tick_time){
  //get week day from tm-struct(0-Sunday, 6-Saturday)
  //convert to 0-Monday, 6-Sunday format
  int i;
  int weekday = tick_time->tm_wday == 0 ? 6 : (tick_time->tm_wday - 1);
  static char s_buffer[8][3] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su", "W"};
  //update current week layer (i=7)
  strftime(s_buffer[7], sizeof(s_buffer[7]), "%W", tick_time);
  text_layer_set_text(s_week_layer[7], s_buffer[7]);
  text_layer_set_background_color(s_week_layer[7], GColorWhite);
  text_layer_set_text_color(s_week_layer[7], GColorBlack); 
  
  //update week days layers
  for (i=0;i<7;i++){
    if (i != weekday){
      text_layer_set_background_color(s_week_layer[i], GColorBlack);
      text_layer_set_text_color(s_week_layer[i], GColorWhite);
    }
    else {
      text_layer_set_background_color(s_week_layer[i], GColorWhite);
      text_layer_set_text_color(s_week_layer[i], GColorBlack);   
    }
    text_layer_set_text(s_week_layer[i], s_buffer[i]); 
  } 
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  //Write the current date into a buffer
  static char s_date_buffer[14];
  strftime(s_date_buffer, sizeof(s_date_buffer), " %B %d", tick_time);

  // Display this date on the TextLayer
  text_layer_set_text(s_date_layer, s_date_buffer);
  
  update_weekdays(tick_time);
}


static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  
  int i;
  int width;
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Bounds: %d x %d\n", bounds.size.w, bounds.size.h);
  
  //Set Background Color
  window_set_background_color(window, GColorBlack);
  
  //------------DATE-------------------------------------------------------
  s_date_layer = text_layer_create(
      GRect(0, 40, bounds.size.w, 30));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  //----------------------------------------------
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  //-----------------------------------------------------------------------
  
  
  //------------------- TIME -----------------------------------------------
  s_time_layer = text_layer_create( GRect(0, 65, bounds.size.w, 45));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  //---------------------------------------------------------------------------
  
  //-------------------------- WEEK DAYs ----------------------------------------
  width = (int) bounds.size.w/8.0;
  for (i=0; i<8; i++){
    s_week_layer[i] = text_layer_create(GRect(i*width, 120, width, 20));
    text_layer_set_background_color(s_week_layer[i], GColorClear);
    text_layer_set_text_color(s_week_layer[i], GColorWhite);
    text_layer_set_font(s_week_layer[i], fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(s_week_layer[i], GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_week_layer[i]));
  }
  //-------------------------------------------------------------------------------
  
  
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  // Destroy Date TextLayer
  text_layer_destroy(s_date_layer);
  // Unload Date GFont
  fonts_unload_custom_font(s_date_font);
  
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  //window_set_background_color(s_main_window, GColorBlack);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();

}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}