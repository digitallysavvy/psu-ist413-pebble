#include <pebble.h>
  
// Weather key definitions
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1 

// Motion Disc definitions
#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50
  
// 2D vector declarations 
typedef struct Vec2d {
  double x;
  double y;
} Vec2d;

// Motion Disc Type declarations 
typedef struct Disc {
  Vec2d pos;
  Vec2d vel;
  double mass;
  double radius;
} Disc;  

// Window Variable declarations
static Window *main_window;
static GRect window_frame;

// Watchface declarations
static TextLayer *s_time_layer;
static TextLayer *s_twelvehour_layer;
static TextLayer *s_weather_layer;


// Motion Disc Variable declarations
static Disc discs[NUM_DISCS];
static double next_radius = 3;
static Layer *disc_layer;
static AppTimer *timer;
static double disc_calc_mass(Disc *disc) {
  return MATH_PI * disc->radius * disc->radius * DISC_DENSITY;
}

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
// Motion Disc Methods
///////////////////////////////////

static void disc_init(Disc *disc) {
  GRect frame = window_frame;
  disc->pos.x = frame.size.w/2;
  disc->pos.y = frame.size.h/2;
  disc->vel.x = 0;
  disc->vel.y = 0;
  disc->radius = next_radius;
  disc->mass = disc_calc_mass(disc);
  next_radius += 0.5;
}

static void disc_apply_force(Disc *disc, Vec2d force) {
  disc->vel.x += force.x / disc->mass;
  disc->vel.y += force.y / disc->mass;
}

static void disc_apply_accel(Disc *disc, AccelData accel) {
  Vec2d force;
  force.x = accel.x * ACCEL_RATIO;
  force.y = -accel.y * ACCEL_RATIO;
  disc_apply_force(disc, force);
}

static void disc_update(Disc *disc) {
  const GRect frame = window_frame;
  double e = 0.5;
  if ((disc->pos.x - disc->radius < 0 && disc->vel.x < 0)
    || (disc->pos.x + disc->radius > frame.size.w && disc->vel.x > 0)) {
    disc->vel.x = -disc->vel.x * e;
  }
  if ((disc->pos.y - disc->radius < 0 && disc->vel.y < 0)
    || (disc->pos.y + disc->radius > frame.size.h && disc->vel.y > 0)) {
    disc->vel.y = -disc->vel.y * e;
  }
  disc->pos.x += disc->vel.x;
  disc->pos.y += disc->vel.y;
}

static void disc_draw(GContext *ctx, Disc *disc) {
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, GPoint(disc->pos.x, disc->pos.y), disc->radius);
}

static void disc_layer_update_callback(Layer *me, GContext *ctx) {
  for (int i = 0; i < NUM_DISCS; i++) {
    disc_draw(ctx, &discs[i]);
  }
}

static void timer_callback(void *data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };

  accel_service_peek(&accel);

  for (int i = 0; i < NUM_DISCS; i++) {
    Disc *disc = &discs[i];
    disc_apply_accel(disc, accel);
    disc_update(disc);
  }

  layer_mark_dirty(disc_layer);

  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}


///////////////////////////////////
// Window Methods
///////////////////////////////////

//window elements
static void main_window_load(Window *window){
  // Window and Fram Layers for use by motion discs
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 35, 144, 80));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  // Create 12hour TextLayer
  s_twelvehour_layer = text_layer_create(GRect(20, 75, 144, 50));
  text_layer_set_background_color(s_twelvehour_layer, GColorClear);
  text_layer_set_text_color(s_twelvehour_layer, GColorBlack);
  text_layer_set_font(s_twelvehour_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_twelvehour_layer, GTextAlignmentLeft);
  text_layer_set_text(s_twelvehour_layer, "AM");
    
  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // Create and style temperature Layer
  s_weather_layer = text_layer_create(GRect(0,130,144,25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
   
  // Motion Disc Layer Setup
  disc_layer = layer_create(frame);
  layer_set_update_proc(disc_layer, disc_layer_update_callback);

  for (int i = 0; i < NUM_DISCS; i++) {
    disc_init(&discs[i]);
  }
  
  // Add child layers to the window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_twelvehour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, disc_layer);
  
  // Make sure the time is displayed from the start
  update_time();
}

// Memory Management
static void main_window_unload(Window *window){
  // Destroy Textlayer
  text_layer_destroy(s_time_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  
  // Destroy motion disc elements
  layer_destroy(disc_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get weather update every 30 minutes
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
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
  
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
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });
  
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
  
  //
  accel_data_service_subscribe(0, NULL);

  timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}


///////////////////////////////////
// Memory Management
///////////////////////////////////
static void deinit(){
  accel_data_service_unsubscribe();
  window_destroy(main_window);
  
}

int main(void){
  init();
  app_event_loop();
  deinit();
}