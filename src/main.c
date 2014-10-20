#include <pebble.h>
#include <confirmCall.c>
  
// Definitions
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1 


// Window Variable declarations
static Window *main_window;
ActionBarLayer *action_bar;

// Action Bar icons
static GBitmap *action_bar_call_icon;
static GBitmap *action_bar_up_icon;
static GBitmap *action_bar_down_icon;

// Watchface declarations
static TextLayer *s_time_layer;
static TextLayer *s_twelvehour_layer;
static TextLayer *s_weather_layer;


///////////////////////////////////
// Watchface Methods
///////////////////////////////////

//Update Time
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  //Use 12 hour format
  strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  if(tick_time->tm_hour > 12){
    text_layer_set_text(s_twelvehour_layer, "PM");
  }
  

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

///////////////////////////////////
// Click Handler Methods
///////////////////////////////////

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  confirmInit();
  app_event_loop();
  confirmDeinit();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

///////////////////////////////////
// Window Methods
///////////////////////////////////

//window elements
static void main_window_load(Window *window){
  // Window layer
  Layer *window_layer = window_get_root_layer(window);
  
  // Create the action bar and set background color
  action_bar = action_bar_layer_create();
  action_bar_layer_set_background_color(action_bar, GColorBlack);
  
  // Associate the action bar with the window and set the click handler
  action_bar_layer_add_to_window(action_bar, window);
  action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
  
  // Create and set action bar icons
  action_bar_call_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CALL);
  action_bar_up_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_UP);
  action_bar_down_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_DOWN);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, action_bar_call_icon);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, action_bar_up_icon);
  action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, action_bar_down_icon);
  
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 30, 144, 80));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentLeft);text_layer_set_text(s_time_layer, "00:00");
  
  // Create 12hour TextLayer
  s_twelvehour_layer = text_layer_create(GRect(10, 70, 144, 50));
  text_layer_set_background_color(s_twelvehour_layer, GColorClear);
  text_layer_set_text_color(s_twelvehour_layer, GColorBlack);
  text_layer_set_font(s_twelvehour_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(s_twelvehour_layer, GTextAlignmentLeft);
  text_layer_set_text(s_twelvehour_layer, "AM"); 
  
  // Create and style temperature Layer
  s_weather_layer = text_layer_create(GRect(5,120,144,25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  text_layer_set_text(s_weather_layer, "6 New Alerts!");
  
  // Add child layers to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_twelvehour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

// Memory Management
static void main_window_unload(Window *window){
  // Destroy Textlayer
  text_layer_destroy(s_time_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  //Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

///////////////////////////////////
// App Messaging
///////////////////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  //text_layer_set_text(s_weather_layer, weather_layer_buffer);
  text_layer_set_text(s_weather_layer, "6 New Alerts!");
  
}

// App Messaging Methods that must be defined for implementation but not used at this time
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

///////////////////////////////////
// App Initialization
///////////////////////////////////
static void init(){
  
  // Create main Window element and assign to pointer
  main_window = window_create();
  
  // Set Click Handler Provider
  //window_set_click_config_provider(main_window, click_config_provider);
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
 
  // Set window to full screen, hide status bar
  window_set_fullscreen(main_window, true);
  // Show the Window on the watch, with animated=true
  window_stack_push(main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
      
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}


///////////////////////////////////
// Memory Management
///////////////////////////////////
static void deinit(){
  window_destroy(main_window);
  gbitmap_destroy(action_bar_call_icon);
  gbitmap_destroy(action_bar_up_icon);
  gbitmap_destroy(action_bar_down_icon);
  action_bar_layer_remove_from_window(action_bar);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}