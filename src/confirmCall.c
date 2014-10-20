#include <pebble.h>
#include <call.c>
  
// Window Variable declarations
static Window *confirm_window;
ActionBarLayer *confirm_action_bar;

// Action Bar icons
static GBitmap *action_bar_call_icon;

// Watchface declarations
static TextLayer *confirm_text_layer;
static TextLayer *click_layer;

///////////////////////////////////
// Click Handler Methods
///////////////////////////////////

static void all_click_handler(ClickRecognizerRef recognizer, void *context) {
  callInit();
  app_event_loop();
  callDeinit();
}

static void confirm_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, all_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, all_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, all_click_handler);
}


///////////////////////////////////
// Window Methods
///////////////////////////////////

//window elements
static void confirm_window_load(Window *window){
  
  // Window layer
  Layer *window_layer = window_get_root_layer(window);
  
  // Create the action bar and set background color
  confirm_action_bar = action_bar_layer_create();
  action_bar_layer_set_background_color(confirm_action_bar, GColorBlack);
  
  // Associate the action bar with the window and set the click handler
  action_bar_layer_add_to_window(confirm_action_bar, window);
  action_bar_layer_set_click_config_provider(confirm_action_bar, confirm_click_config_provider);
  
  // Create and set action bar icons
  action_bar_call_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_CALL);
  action_bar_layer_set_icon(confirm_action_bar, BUTTON_ID_SELECT, action_bar_call_icon);
  
  // Create Confirm TextLayer
  confirm_text_layer = text_layer_create(GRect(10, 15, 144, 130));
  text_layer_set_background_color(confirm_text_layer, GColorClear);
  text_layer_set_text_color(confirm_text_layer, GColorBlack);
  text_layer_set_text(confirm_text_layer, "CALL\n911?");
  text_layer_set_font(confirm_text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(confirm_text_layer, GTextAlignmentLeft);
  
 
  // Click handle
  click_layer = text_layer_create(GRect(10,110,100,55));
  text_layer_set_text(click_layer, "Press any button");
  text_layer_set_text_alignment(click_layer, GTextAlignmentCenter);
  
  // Add child layers to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(confirm_text_layer));
  layer_add_child(window_layer, text_layer_get_layer(click_layer));
}

// Memory Management
static void confirm_window_unload(Window *window){
  // Destroy Textlayer
  text_layer_destroy(confirm_text_layer);
  
  // Destroy Click Handler Text
  text_layer_destroy(click_layer);

}

static void confirmInit(){
  
  // Create main Window element and assign to pointer
  confirm_window = window_create();
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(confirm_window, (WindowHandlers){
    .load = confirm_window_load,
    .unload = confirm_window_unload
  });
  
  // Set window to fullscreen
  window_set_fullscreen(confirm_window, true);
  
  // Show the Window on the watch, with animated=true
  window_stack_push(confirm_window, true);
  
}


///////////////////////////////////
// Memory Management
///////////////////////////////////
static void confirmDeinit(){
  window_destroy(confirm_window);
}

