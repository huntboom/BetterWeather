#include "pebble.h"

#define NUM_MENU_SECTIONS 1
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_HOURS_IN_DAY 24
#define NUM_DAYS_IN_WEEK 7

static Window *s_main_window;
static Window *s_forecast_window;
static Window *s_hourly_window;
static Window *s_weekly_window;  // New window for the 7-day forecast
static MenuLayer *s_menu_layer;
static MenuLayer *s_forecast_menu_layer;
static MenuLayer *s_weekly_menu_layer;  // Menu layer for the 7-day forecast

// Function prototypes
static void forecast_window_load(Window *window);
static void forecast_window_unload(Window *window);
static void hourly_window_load(Window *window);
static void hourly_window_unload(Window *window);
static void weekly_window_load(Window *window);
static void weekly_window_unload(Window *window);
static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);
static void forecast_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);
static void weekly_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data);

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case 0:
      return NUM_FIRST_MENU_ITEMS;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Draw title text in the section header
  menu_cell_basic_header_draw(ctx, cell_layer, "Current Location");
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  // Determine which section we're going to draw in
  switch (cell_index->section) {
    case 0:
      // Use the row to specify which item we'll draw
      switch (cell_index->row) {
        case 0:
          // This is a basic menu item with a title and subtitle
          menu_cell_basic_draw(ctx, cell_layer, "Today's Forecast", "Hourly", NULL);
          break;
        case 1:
          menu_cell_basic_draw(ctx, cell_layer, "7 Day Forecast", "Starts Sunday", NULL);
          break;
      }
      break;
  }
}

static uint16_t forecast_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t forecast_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_HOURS_IN_DAY;
}

static void forecast_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  static char s_buff[16];
  int hour = cell_index->row % 12;
  if (hour == 0) {
    hour = 12;
  }
  snprintf(s_buff, sizeof(s_buff), "%d:00 %s", hour, (cell_index->row < 12) ? "AM" : "PM");
  menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

static uint16_t weekly_menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t weekly_menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_DAYS_IN_WEEK;
}

static void weekly_menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  static const char *days_of_week[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
  };
  menu_cell_basic_draw(ctx, cell_layer, days_of_week[cell_index->row], NULL, NULL);
}

static void main_window_load(Window *window) {
  // Now we prepare to initialize the menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the menu layer
  s_menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = PBL_IF_RECT_ELSE(menu_get_header_height_callback, NULL),
    .draw_header = PBL_IF_RECT_ELSE(menu_draw_header_callback, NULL),
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback  // Add select click handler
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void main_window_unload(Window *window) {
  // Destroy the menu layer
  menu_layer_destroy(s_menu_layer);
}

static void forecast_window_load(Window *window) {
  // Now we prepare to initialize the forecast menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the forecast menu layer
  s_forecast_menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(s_forecast_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = forecast_menu_get_num_sections_callback,
    .get_num_rows = forecast_menu_get_num_rows_callback,
    .draw_row = forecast_menu_draw_row_callback,
    .select_click = forecast_menu_select_callback  // Add select click handler
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_forecast_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_forecast_menu_layer));
}

static void forecast_window_unload(Window *window) {
  // Destroy the forecast menu layer
  menu_layer_destroy(s_forecast_menu_layer);
}

static void hourly_window_load(Window *window) {
  // Load window contents (if any) here
  // You can add text layers or other UI elements here for the hourly window
}

static void hourly_window_unload(Window *window) {
  // Unload window contents (if any) here
}

static void weekly_window_load(Window *window) {
  // Now we prepare to initialize the weekly menu layer
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  // Create the weekly menu layer
  s_weekly_menu_layer = menu_layer_create(bounds);

  menu_layer_set_callbacks(s_weekly_menu_layer, NULL, (MenuLayerCallbacks){
    .get_num_sections = weekly_menu_get_num_sections_callback,
    .get_num_rows = weekly_menu_get_num_rows_callback,
    .draw_row = weekly_menu_draw_row_callback,
    .select_click = weekly_menu_select_callback  // Add select click handler
  });

  // Bind the menu layer's click config provider to the window for interactivity
  menu_layer_set_click_config_onto_window(s_weekly_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_weekly_menu_layer));
}

static void weekly_window_unload(Window *window) {
  // Destroy the weekly menu layer
  menu_layer_destroy(s_weekly_menu_layer);
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == 0) {
    if (cell_index->row == 0) {
      // Create the forecast window
      s_forecast_window = window_create();
      window_set_window_handlers(s_forecast_window, (WindowHandlers) {
        .load = forecast_window_load,
        .unload = forecast_window_unload,
      });
      window_stack_push(s_forecast_window, true);
    } else if (cell_index->row == 1) {
      // Create the weekly forecast window
      s_weekly_window = window_create();
      window_set_window_handlers(s_weekly_window, (WindowHandlers) {
        .load = weekly_window_load,
        .unload = weekly_window_unload,
      });
      window_stack_push(s_weekly_window, true);
    }
  }
}

static void forecast_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Create a new window for the selected hour
  s_hourly_window = window_create();
  window_set_window_handlers(s_hourly_window, (WindowHandlers) {
    .load = hourly_window_load,
    .unload = hourly_window_unload,
  });
  window_stack_push(s_hourly_window, true);
}

static void weekly_menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  // Create a new window for the selected day (blank for now)
  s_hourly_window = window_create();
  window_set_window_handlers(s_hourly_window, (WindowHandlers) {
    .load = hourly_window_load,
    .unload = hourly_window_unload,
  });
  window_stack_push(s_hourly_window, true);
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
  if (s_forecast_window) {
    window_destroy(s_forecast_window);
  }
  if (s_hourly_window) {
    window_destroy(s_hourly_window);
  }
  if (s_weekly_window) {
    window_destroy(s_weekly_window);
  }
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
