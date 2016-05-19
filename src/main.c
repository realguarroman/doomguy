/*
 * main.c
 * Sets up a Window, BitmapLayer and blank GBitmap to be used as the display
 * container for the GBitmapSequence. It also counts the number of frames.
 *
 * Animation source:
 * http://bestanimations.com/Science/Physics/Physics2.html
 */

#include <pebble.h>



#ifdef PBL_PLATFORM_CHALK
  #define OFFSET_X 18
  #define OFFSET_Y 10
	#define OFFSET_Y2 16
#endif

#ifdef PBL_PLATFORM_BASALT
  #define OFFSET_X 0
  #define OFFSET_Y 0
	#define OFFSET_Y2 0
#endif

#define ANIMATION_DURATION 10000

static Window *s_main_window;

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static TextLayer *s_time_layer;

//static AppTimer *timer;
//static int counter;
static TextLayer *s_time;
static GFont s_time_font;

static int doomguy_level = 0;
static int doomguy_animation = 0;
//temporizadores
AppTimer *timer;

const int DOOMGUYCENTER[7] = {
  RESOURCE_ID_GUY00, RESOURCE_ID_GUY10, RESOURCE_ID_GUY20,
  RESOURCE_ID_GUY30, RESOURCE_ID_GUY40, RESOURCE_ID_GUY50, RESOURCE_ID_GUY60
};


const int DOOMGUYRIGHT[7] = {
  RESOURCE_ID_GUY00, RESOURCE_ID_GUY11, RESOURCE_ID_GUY21,
  RESOURCE_ID_GUY31, RESOURCE_ID_GUY41, RESOURCE_ID_GUY51, RESOURCE_ID_GUY60
};

const int DOOMGUYLEFT[7] = {
  RESOURCE_ID_GUY00, RESOURCE_ID_GUY12, RESOURCE_ID_GUY22,
  RESOURCE_ID_GUY32, RESOURCE_ID_GUY42, RESOURCE_ID_GUY52, RESOURCE_ID_GUY60
};

/*
static void timer_handler(void *context) {
  uint32_t next_delay;

		// Advance to the next APNG frame
	
		if(gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
			bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
			layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
			// Timer for that delay
			counter=counter + 1;
			if (counter<TOTAL_ANI_FRAMES) {
					app_timer_register(next_delay, timer_handler, NULL);
			} else {
					gbitmap_sequence_destroy(s_sequence);
					s_sequence = NULL;
				  counter=0;
			}

		} else {
			// Start again
	    gbitmap_sequence_restart(s_sequence);
		}
		
}

*/


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    // Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
	layer_mark_dirty(text_layer_get_layer(s_time_layer));
}


static void update_doom_guy() {
	

	gbitmap_destroy(s_bitmap);	
	int face=rand()%3;	
	switch(face)
	{
		case 0:
			s_bitmap = gbitmap_create_with_resource(DOOMGUYLEFT[doomguy_level]); 
		break;
		case 1:
			s_bitmap = gbitmap_create_with_resource(DOOMGUYCENTER[doomguy_level]); 
		break;
		case 2:
			s_bitmap = gbitmap_create_with_resource(DOOMGUYRIGHT[doomguy_level]); 
		break;
	}		

	
//  s_bitmap = gbitmap_create_with_resource(DOOMGUYLEFT[doomguy_level]); 
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));
	
}

/*

static void load_sequence(int ani) {
  // Free old data
//	update_time();
  counter=0;
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(ani);

  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

  // Begin animation
  timer = app_timer_register(1, timer_handler, NULL);
}

*/

static void update_battery() {  /* mejor será suscribirse a un servicio, esto se hace cada minuto y no mola */
	
	static uint8_t battery_level;
	BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;	
	
	if (battery_level < 10) {
			doomguy_level=6; 
	} else if (battery_level >= 10 && battery_level < 30) {
   		doomguy_level=5; 
	} else if (battery_level >= 30 && battery_level < 50) {
   		doomguy_level=4; 
	} else if (battery_level >= 50 && battery_level < 70) {
   		doomguy_level=3; 
	} else if (battery_level >= 70 && battery_level < 80) {
   		doomguy_level=2; 
	} else if (battery_level >= 80 && battery_level < 90) {
   		doomguy_level=1; 
	} else {
   		doomguy_level=0; 
	}
		
	
	
	
}



static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_battery();
	update_doom_guy();
	update_time();	
}

static void tick_seconds(struct tm *tick_time, TimeUnits units_changed) {
	if (doomguy_animation == 1) update_doom_guy();
}


void timer_callback(void *data) {
	doomguy_animation = 0;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
	doomguy_animation = 1;
	timer = app_timer_register(ANIMATION_DURATION, (AppTimerCallback) timer_callback, NULL);
}


//static void tick_handler_seconds(struct tm *tick_time, TimeUnits units_changed) {

//	update_doom_guy();
	
//}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
	
	srand(time(NULL));
	
	
	//background
  //s_bitmap_layer = bitmap_layer_create(GRect(BACKGROUND_X,BACKGROUND_Y,180,180));
	s_bitmap_layer = bitmap_layer_create(GRect(0 + OFFSET_X, 20 + OFFSET_Y, 144, 168));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));	
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_GUY00);
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	
	
	s_time_layer= text_layer_create(GRect(0 + OFFSET_X, -15 + OFFSET_Y2, 144, 50)); // 0,0,144,84
	text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorOrange);
  text_layer_set_text(s_time_layer, "00:00");


	
	 //Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARCADE_60));
  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	
	
	
	
 // GRect bounds = layer_get_bounds(window_layer);

	window_set_background_color (window, GColorBlack);
	
//	hour_layer_dec = bitmap_layer_create(GRect(0, -84, 72, 84));
	
	
 
 // layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

//	app_timer_cancel(timer);	
//  load_sequence(ANIMATIONS[rand()%6]);
	update_battery();
	update_doom_guy();
	update_time();
}

/*
static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
	load_sequence(ANIMATIONS[rand()%6]);
}

*/

static void main_window_unload(Window *window) {
	fonts_unload_custom_font(s_time_font);
  text_layer_destroy(s_time);
  bitmap_layer_destroy(s_bitmap_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
#ifdef PBL_SDK_2
  window_set_fullscreen(s_main_window, true);
#endif
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	tick_timer_service_subscribe(SECOND_UNIT , tick_seconds);
	//Subscribe to AccelerometerService
  accel_tap_service_subscribe(accel_tap_handler);

}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}


