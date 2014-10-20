#include <pebble.h>

// Window Variable declarations
static Window *call_window;

// Text layer declarations
static TextLayer *call_text_layer;
static TextLayer *call_timer_layer;


//Timer
//Update Time
static void update_timer() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  //Use 12 hour format
  strftime(buffer, sizeof("00:00"), "%M:%S", tick_time);
  if(tick_time->tm_second > 10){
    text_layer_set_text(s_twelvehour_layer, "PM");
  }
  

  // Display this time on the TextLayer
  text_layer_set_text(call_timer_layer, buffer);
}

///////////////////////////////////
// Window Methods
///////////////////////////////////

//window elements
static void call_window_load(Window *window){
  // Window and Fram Layers for use by motion discs
  Layer *window_layer = window_get_root_layer(window);
  
  // Create Confirm TextLayer
  call_text_layer = text_layer_create(GRect(10, 45, 144, 130));
  text_layer_set_background_color(call_text_layer, GColorClear);
  text_layer_set_text_color(call_text_layer, GColorBlack);
  text_layer_set_text(call_text_layer, "Calling...");
  text_layer_set_font(call_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK));
  text_layer_set_text_alignment(call_text_layer, GTextAlignmentLeft);
  
  
  // Add child layers to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(call_text_layer));
}

// Memory Management
static void call_window_unload(Window *window){
  // Destroy Textlayer
  text_layer_destroy(call_text_layer);
  

}

static void callInit(){
  
  // Create main Window element and assign to pointer
  call_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(call_window, (WindowHandlers){
    .load = call_window_load,
    .unload = call_window_unload
  });
  
  // Show the Window on the watch, with animated=true
  window_stack_push(call_window, true);
  
}


///////////////////////////////////
// Memory Management
///////////////////////////////////
static void callDeinit(){
  window_destroy(call_window);
}