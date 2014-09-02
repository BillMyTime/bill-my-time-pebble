

#include "pebble.h"

static Window *window;

static TextLayer *text_layer;

static TextLayer *timer_layer;

static AppTimer *timer;

// Set 1 minute time interval for loop on timer
static const uint32_t timeout_ms = 60000;

// Time values
static int elapsed_time; // diff for now - start time
static int start_time;

// Bool flag for running timer
static bool running;

// fonts
static GFont timer_font;

// Function decs
void timer_callback(void *context);
void update_timer_layer();
void start_timer();
void stop_timer();
static void click_config_provider(void* context);
void toggle_timer_click(ClickRecognizerRef recognizer, Window *window);
void cancel_timer_click(ClickRecognizerRef recognizer, Window *window);
void change_task_click(ClickRecognizerRef recognizer, Window *window);
void change_project_click(ClickRecognizerRef recognizer, Window *window);
void change_client_click(ClickRecognizerRef recognizer, Window *window);
void submit_time_to_task(ClickRecognizerRef recognizer, Window *window);
static void init(void);
static void deinit(void);
int main(void);

// check if timer is running, and update display if so, always restart the timer loop
void timer_callback(void *context) {
	if(running) {
		static int now;
		now = time(NULL);
		elapsed_time = now - start_time;
		update_timer_layer();
	}
  timer = app_timer_register(timeout_ms, timer_callback, NULL);
}


//Update the display
void update_timer_layer() {
	static char time_display[] = "00:00";
	if (elapsed_time > 0 ) {
		// Get hours and minutes for display
		int minutes = (int)elapsed_time / 60 % 60;
    int hours = (int)elapsed_time / 3600;
		snprintf(time_display, 6, "%02d:%02d", hours, minutes);
	}
	text_layer_set_text(timer_layer, time_display);
	
}


void start_timer() {
	running = true;
	if(start_time == 0) { // shouldn't be needed, just in case of unhandled exceptions
		start_time = time(NULL);
	}
  timer = app_timer_register(timeout_ms, timer_callback, NULL);
}

void stop_timer() {
    running = false;
    if(timer != NULL) {
			app_timer_cancel(timer);
			timer = NULL;
    }
}

// set up button clicks
static void click_config_provider(void* context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler)toggle_timer_click);
	window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)cancel_timer_click);
	window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)change_task_click);
	window_long_click_subscribe(BUTTON_ID_DOWN, 700, (ClickHandler)change_project_click, NULL);
	window_long_click_subscribe(BUTTON_ID_UP, 700, (ClickHandler)change_client_click, NULL);
	window_long_click_subscribe(BUTTON_ID_SELECT, 700, (ClickHandler)submit_time_to_task, NULL);
}

// Button handlers
void toggle_timer_click(ClickRecognizerRef recognizer, Window *window) {
    if(running) {
			static char textdebug[] = "toggle triggered stop";
			text_layer_set_text(text_layer, textdebug );
			stop_timer();
    } else {
			text_layer_set_text(text_layer, "toggle triggered start");
			start_timer();
    }
}

// Cancel current time entry
void cancel_timer_click(ClickRecognizerRef recognizer, Window *window) {
	if (running) {
		// currently just acts as timer stop, will later change this to present a are you sure prompt, as it likely is an incorrect click
	} else {
		elapsed_time = 0;
		update_timer_layer();
	}
}
// Select another task for the current project, or create a new (unnamed) task
void change_task_click(ClickRecognizerRef recognizer, Window *window) {
	// TODO: retrive task list via JSON and fill menu
}
// Select another project for current client
void change_project_click(ClickRecognizerRef recognizer, Window *window) {
	// TODO: retrieve project list and present in menu
}
// Change to another client
void change_client_click(ClickRecognizerRef recognizer, Window *window) {
	// TODO: return filtered list of clients (grouped by first letter)
}

void submit_time_to_task(ClickRecognizerRef recognizer, Window *window) {
	//TODO: JSON post to record time interval
}

static void init(void) {
  window = window_create();
	running = false;
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  const bool animated = true;
	window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, false);
	window_stack_push(window, animated);

	// Arrange for user input.
	window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

	timer_font = fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS);
	timer_layer = text_layer_create(GRect(0, 80, 144, 84));
	text_layer_set_background_color(timer_layer, GColorBlack);
  text_layer_set_font(timer_layer, timer_font);
	text_layer_set_text_color(timer_layer, GColorWhite);
	text_layer_set_text(timer_layer, "00:00");
	text_layer_set_text_alignment(timer_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, (Layer*)timer_layer);

	text_layer = text_layer_create(GRect(0, 0, 144, 80));
	text_layer_set_background_color(text_layer, GColorWhite);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text(text_layer, "Not Running");
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, (Layer*)text_layer);
  //timer = app_timer_register(timeout_ms, timer_callback, NULL);
}

static void deinit(void) {

  text_layer_destroy(timer_layer);
  text_layer_destroy(text_layer);
  window_destroy(window);
}

int main(void) {
  init();

  app_event_loop();

  deinit();
}
