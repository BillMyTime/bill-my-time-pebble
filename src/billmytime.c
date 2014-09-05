

#include "pebble.h"

static Window *window;

static Window *menu_window;

static TextLayer *text_layer;

static TextLayer *timer_layer;

static AppTimer *timer;

static SimpleMenuLayer *menu_list_layer;

// Set 1 minute time interval for loop on timer
static const uint32_t timeout_ms = 60000;

// Time values
static int elapsed_time; // diff for now - start time
static int start_time;

// Bool flag for running timer
static bool running;

// fonts
static GFont timer_font;
static GFont text_font;

// holders for menu entries
static SimpleMenuSection list_menu_sections[1];
static SimpleMenuItem* list_menu_items;
static char* menu_action;
static int page;

// Function decs
void timer_callback(struct tm *tick_time, TimeUnits units_changed);
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
static void in_received_handler(DictionaryIterator *received, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);
void select_menu_callback (int index, void *context);



// check if timer is running, and update display if so, always restart the timer loop
void timer_callback(struct tm *tick_time, TimeUnits units_changed) {
	if(running) {
		static int now;
		now = time(NULL);
		elapsed_time = now - start_time;
		update_timer_layer();
	}
}


//Update the display
void update_timer_layer() {
	static char time_display[] = "00h 00m";
	// Get hours and minutes for display
	int minutes = (int)elapsed_time / 60 % 60;
	int hours = (int)elapsed_time / 3600;
	snprintf(time_display, 8, "%02dh %02dm", hours, minutes);
	text_layer_set_text(timer_layer, time_display);
}


void start_timer() {
	running = true;
	if(start_time == 0) { // shouldn't be needed, just in case of unhandled exceptions
		start_time = time(NULL);
	}
	tick_timer_service_subscribe(MINUTE_UNIT, timer_callback);
}

void stop_timer() {
    running = false;
			tick_timer_service_unsubscribe();
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
			text_layer_set_text(text_layer, "toggle triggered stop" );
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
		stop_timer();

	text_layer_set_text(text_layer, "Cancel Pressed, with running timer");
	} else {
		elapsed_time = 0;
		start_time = 0;
		update_timer_layer();
		text_layer_set_text(text_layer, "Cancel Pressed when stopped");
	}
}
// Select another task for the current project, or create a new (unnamed) task
void change_task_click(ClickRecognizerRef recognizer, Window *window) {
	// TODO: retrive task list via JSON and fill menu
	static DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	Tuplet value = TupletCString(0, "getTasks");
	dict_write_tuplet(iter, &value);
	Tuplet pager = TupletInteger(1, page);
	dict_write_tuplet(iter, &pager);
	app_message_outbox_send();
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

// Appmessage callbacks
void out_sent_handler(DictionaryIterator *sent, void *context) {
   // outgoing message was delivered
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
 }


 void in_received_handler(DictionaryIterator *received, void *context) {
	int count = 0;
	Tuple *tuple = dict_read_first(received);
	static char* section_title;
	while(tuple){
			count ++;
			if (tuple->key == 0) {
				menu_action = tuple->value->cstring;
			}
			tuple = dict_read_next(received);
	}
	int number_of_entries = count - 1;
	list_menu_items = malloc(number_of_entries * sizeof(SimpleMenuItem));
	int i = 0;
	tuple = dict_read_first(received);
	while (tuple) {
			if(tuple->key != 0){
					list_menu_items[i] = (SimpleMenuItem){
							.title = tuple->value->cstring,
							.callback = select_menu_callback
					};
					i++;
			}
			tuple = dict_read_next(received);
	}
	if  (strcmp(menu_action, "c") == 0) {
			section_title = "Clients";
	} else if(strcmp(menu_action, "t") == 0) {
			section_title = "Tasks";
	} else if (strcmp(menu_action, "p") == 0) {
			section_title = "Projects";
	} else {
			section_title = "Select";
	}
	list_menu_sections[0] = (SimpleMenuSection){
			.title = section_title,
			.num_items = number_of_entries,
			.items = list_menu_items
	};
	window_stack_push(menu_window, true);

	Layer *window_layer = window_get_root_layer(menu_window);
	menu_list_layer = simple_menu_layer_create(GRect(0, 5, 144, 163), menu_window, list_menu_sections, 1, NULL);
	//layer_set_hidden(text_layer_get_layer(timer_layer));
	//layer_set_hidden(text_layer_get_layer(text_layer));
	layer_add_child(window_layer, simple_menu_layer_get_layer(menu_list_layer));
 }


 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }


 // Menu callback
 void select_menu_callback(int index, void *context) {
	// placeholder
 }

static void init(void) {
	// Set initial value to track timer start/stop
	running = false;
	// Set variables to use for animated windows
	const bool animated = true;

	// Set up the base window
  window = window_create();
  Layer *window_layer = window_get_root_layer(window);
	window_set_background_color(window, GColorWhite);
  window_set_fullscreen(window, false);
	window_stack_push(window, animated);
	page = 1; // set to 1 for initial paging on menus

	// Bind input for base window
	window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);

	// Set up layers for base window
	timer_font = fonts_get_system_font(FONT_KEY_DROID_SERIF_28_BOLD);
	timer_layer = text_layer_create(GRect(0, 80, 144, 84));
	text_layer_set_background_color(timer_layer, GColorBlack);
  text_layer_set_font(timer_layer, timer_font);
	text_layer_set_text_color(timer_layer, GColorWhite);
	text_layer_set_text(timer_layer, "00h 00m");
	text_layer_set_text_alignment(timer_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(timer_layer));

	text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
	text_layer = text_layer_create(GRect(0, 0, 144, 80));
	text_layer_set_background_color(text_layer, GColorWhite);
  text_layer_set_font(text_layer, text_font);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text(text_layer, "Not Running");
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

	// Set up menu window, but don't push to stack yet
	menu_window = window_create();
	window_set_background_color(menu_window, GColorWhite);
  window_set_fullscreen(menu_window, true);

	// Bind appmessage callbacks
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);
	// Configure inbox/outbot for appmessage
	const uint32_t inbound_size = app_message_inbox_size_maximum();
	const uint32_t outbound_size = app_message_outbox_size_maximum();
	app_message_open(inbound_size, outbound_size);
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
