#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;  
static GFont s_time_font;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;

static int s_battery_level;
static Layer *s_battery_layer;

static void battery_callback(BatteryChargeState state) {
	// Record the new battery level
	s_battery_level = state.charge_percent;
	// Update meter
	layer_mark_dirty(s_battery_layer);

}

static void bluetooth_callback(bool connected) {
	// Show icon if disconnected
	layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);
	if(!connected) {
    	// Issue a vibrating alert
    	vibes_double_pulse();
	}
	
	
}


static void battery_update_proc(Layer *layer, GContext *ctx) {
	
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
 
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // format the way that the time text is going to be displayed
strftime(buffer, sizeof("00:00"),"%I:%M", tick_time);


  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}


static void main_window_load(Window *window) {
	// Create GBitmap, then set to created BitmapLayer
	//s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BLACK);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 180));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
	// Create time TextLayer
	//s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MINIMAL_BOLD_56));

	//time layer
	s_time_layer = text_layer_create(GRect(0, 50, 144, 180));
	bitmap_layer_set_background_color(s_background_layer, GColorBlack);
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	// Use a system font in a TextLayer
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text(s_time_layer, "00:00");

	// Improve the layout to be more like a watchface
	// text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

	// Add it as a child layer to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	// Create battery meter Layer
	s_battery_layer = layer_create(GRect(14, 54, 115, 2));
	layer_set_update_proc(s_battery_layer, battery_update_proc);

	// Add to Window
	layer_add_child(window_get_root_layer(window), s_battery_layer);
	
	// Create the Bluetooth icon GBitmap
	s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_ICON);

	// Create the BitmapLayer to display the GBitmap
	s_bt_icon_layer = bitmap_layer_create(GRect(105, 123, 50, 50));
	bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
	bitmap_layer_set_background_color(s_bt_icon_layer, GColorBlack);
	//layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
	
	// Show the correct state of the BT connection from the start
	bluetooth_callback(connection_service_peek_pebble_app_connection());

layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

}

static void main_window_unload(Window *window) {
    
    // Unload GFont
    fonts_unload_custom_font(s_time_font);
    // Destroy time layer
    text_layer_destroy(s_time_layer);
	layer_destroy(s_battery_layer);
	
	gbitmap_destroy(s_bt_icon_bitmap);
	bitmap_layer_destroy(s_bt_icon_layer);

    
}

//register all events
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 update_time();
 
}

  


static void init() {
 
	// Register with TickTimerService
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	// Register for battery level updates
	battery_state_service_subscribe(battery_callback);
  
	// Create main Window element and assign to pointer
	s_main_window = window_create();

	// Set handlers to manage the elements inside the Window
	window_set_window_handlers(s_main_window, (WindowHandlers) {
    	.load = main_window_load,
    	.unload = main_window_unload
	});

	// Show the Window on the watch, with animated=true
	window_stack_push(s_main_window, true);
  
	// Make sure the time is displayed from the start
	update_time();
	
	// Ensure battery level is displayed from the start
	battery_callback(battery_state_service_peek());
	
	// Register for Bluetooth connection updates
	connection_service_subscribe((ConnectionHandlers) {
	.pebble_app_connection_handler = bluetooth_callback
});


}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}


//this is the main road map for the app  first it will initialize by running init(), then it will run the app until
//the user exits the app and then will run the deinit() code, which will free up all of the memory that we 
//set aside at the for the app.
int main(void) {
  init();
  app_event_loop();
  deinit();
}