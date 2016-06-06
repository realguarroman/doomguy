
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

#ifdef PBL_PLATFORM_APLITE
  #define OFFSET_X 0
  #define OFFSET_Y 0
	#define OFFSET_Y2 0
#endif

#define ANIMATION_DURATION 10000

#define KEY_BEHAVIOUR 1
#define KEY_FONT_COLOR 2


#define BEHAVIOUR_BATTERY 1
#define BEHAVIOUR_HEALTH 2

static Window *s_main_window;

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static TextLayer *s_time_layer;


static TextLayer *s_time;
static GFont s_time_font;
//static GColor font_color;

static int doomguy_level = 0;
static int doomguy_animation = 0;

static int doomguy_behaviour = BEHAVIOUR_BATTERY;


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



static void update_battery() {  /* mejor será suscribirse a un servicio, esto se hace cada minuto y no mola */
	
	static uint8_t battery_level;
	BatteryChargeState initial = battery_state_service_peek();
  battery_level = initial.charge_percent;	
	
	if (battery_level == 0)  {
			doomguy_level=6; 
	} else if ((battery_level == 10) || (battery_level == 20)) {
   		doomguy_level=5; 
	} else if ((battery_level == 30) || (battery_level == 40)) {
   		doomguy_level=4; 
	} else if ((battery_level == 50) || (battery_level == 60)) {
   		doomguy_level=3; 
	} else if ((battery_level == 70) || (battery_level == 80)) {
   		doomguy_level=2; 
	} else if (battery_level == 90) {
   		doomguy_level=1; 
	} else {
   		doomguy_level=0;  //GOD
	}
		

}


static void update_health() {

		HealthValue steps_today = health_service_sum_today(HealthMetricStepCount);
  //  APP_LOG(APP_LOG_LEVEL_INFO, "Average step count: %d steps", (int)steps_today);
		
	
		
		
		// Define query parameters
		const HealthMetric metric = HealthMetricStepCount;
		const HealthServiceTimeScope scope = HealthServiceTimeScopeDaily;

		// Use the average daily value from midnight to the current time
		const time_t start = time_start_of_today();
		const time_t end = time(NULL);
		
		// Check that an averaged value is accessible
		HealthServiceAccessibilityMask mask = health_service_metric_averaged_accessible(metric, start, end, scope);
		if(mask & HealthServiceAccessibilityMaskAvailable) {
		// Average is available, read it
			HealthValue average = health_service_sum_averaged(metric, start, end, scope);

		//	APP_LOG(APP_LOG_LEVEL_INFO, "Average step count: %d steps", (int)average);
			
			
			int diferencia;
			diferencia = (average - steps_today);
	
		
			if (diferencia < -1000)  {
					doomguy_level=0; //GOD
			} else if ((diferencia >= -1000) && (diferencia < 0)) {
					doomguy_level=1; 
			} else if ((diferencia >= 0) && (diferencia < 500)) {
					doomguy_level=2; 
			} else if ((diferencia >= 500) && (diferencia < 1000)) {
					doomguy_level=3; 
			} else if ((diferencia >= 1000) && (diferencia < 1500)) {
					doomguy_level=4; 
			} else if ((diferencia >= 1500) && (diferencia < 2000)) {
					doomguy_level=5; 
			} else if (diferencia >= 2000) {
					doomguy_level=6;  
			}
			
			
		//	APP_LOG(APP_LOG_LEVEL_INFO, "Diferencia: %d", diferencia);
		//	APP_LOG(APP_LOG_LEVEL_INFO, "Level: %d", doomguy_level);
				
		}
	
	
}



static void tick_seconds(struct tm *tick_time, TimeUnits units_changed) {
	int seconds = tick_time->tm_sec;
	if (doomguy_animation == 1) update_doom_guy();
	if(seconds == 0) {
		
		switch(doomguy_behaviour) {	
		  	
			case BEHAVIOUR_BATTERY:
				update_battery();
			break;
			case BEHAVIOUR_HEALTH:
				update_health();
			break;
		}
		
		
		update_doom_guy();
		update_time();	
	} 
}


void timer_callback(void *data) {
	doomguy_animation = 0;
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction)
{
	doomguy_animation = 1;
	timer = app_timer_register(ANIMATION_DURATION, (AppTimerCallback) timer_callback, NULL);
}




void process_tuple(Tuple *t)
{
	
  //Get key
	

  int key = t->key;
	
  //Decide what to do
  switch(key) {	
		  	
			case KEY_BEHAVIOUR:
			
			
				if(strcmp(t->value->cstring, "b") == 0)
				{
					//Set and save
					doomguy_behaviour = BEHAVIOUR_BATTERY;
					persist_write_int(KEY_BEHAVIOUR, BEHAVIOUR_BATTERY);
					update_battery();
				}
				else if(strcmp(t->value->cstring, "h") == 0)
				{
					//Set and save 
					doomguy_behaviour = BEHAVIOUR_HEALTH;
					persist_write_int(KEY_BEHAVIOUR, BEHAVIOUR_HEALTH);
					update_health();
				}
				update_doom_guy();
			//	APP_LOG(APP_LOG_LEVEL_INFO, " value received");
      break;
		
		
			case KEY_FONT_COLOR:
					
					#ifdef PBL_PLATFORM_APLITE
						text_layer_set_text_color(s_time_layer, GColorWhite);
					#else
						text_layer_set_text_color(s_time_layer, GColorFromHEX(t->value->int32));
					#endif
					persist_write_int(KEY_FONT_COLOR, t->value->int32);
				
			
      break;
	}
}


static void in_received_handler(DictionaryIterator *iter, void *context) 
{
		
    (void) context;
    //Get data
    Tuple *t = dict_read_first(iter);
    while(t != NULL)
    {		
			process_tuple(t);      
      //Get next
      t = dict_read_next(iter);
    }
}



static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void main_window_load(Window *window) {
 // Layer *window_layer = window_get_root_layer(window);
	
	srand(time(NULL));
	
	
	
	if (persist_exists(KEY_BEHAVIOUR)) {
		doomguy_behaviour = persist_read_int(KEY_BEHAVIOUR);		
	} else {
		doomguy_behaviour = BEHAVIOUR_BATTERY;
	}
	
	
	#ifdef PBL_PLATFORM_APLITE
		
	#else
		GColor font_color;
		if (persist_exists(KEY_FONT_COLOR)) {
			//int fontcolor_ = persist_read_int(KEY_FONT_COLOR);
			font_color = GColorFromHEX(persist_read_int(KEY_FONT_COLOR));		
		} else {
			font_color = GColorOrange;			
		}
	#endif
	
	
	
	//background
	s_bitmap_layer = bitmap_layer_create(GRect(0 + OFFSET_X, 20 + OFFSET_Y, 144, 168));
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bitmap_layer));	
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_GUY00);
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	
	
	s_time_layer= text_layer_create(GRect(0 + OFFSET_X, -15 + OFFSET_Y2, 144, 50)); // 0,0,144,84
	text_layer_set_background_color(s_time_layer, GColorBlack);
	
	#ifdef PBL_PLATFORM_APLITE
		text_layer_set_text_color(s_time_layer, GColorWhite);
	#else
		text_layer_set_text_color(s_time_layer, font_color);
	#endif
	
  
  text_layer_set_text(s_time_layer, "00:00");
	
	 //Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_ARCADE_60));
  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	


	window_set_background_color (window, GColorBlack);
	
	
	

	
	switch(doomguy_behaviour) {	
		  	
			case BEHAVIOUR_BATTERY:
				update_battery();
			break;
			case BEHAVIOUR_HEALTH:
				update_health();
			break;
	}
	
	
	update_doom_guy();
	update_time();
}



static void main_window_unload(Window *window) {
	fonts_unload_custom_font(s_time_font);
  text_layer_destroy(s_time);
  bitmap_layer_destroy(s_bitmap_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
	tick_timer_service_subscribe(SECOND_UNIT , tick_seconds);
	//Subscribe to AccelerometerService
  accel_tap_service_subscribe(accel_tap_handler);

  // Register callbacks
	app_message_register_inbox_received((AppMessageInboxReceived) in_received_handler);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	app_message_open(784, 784);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}


