#include <pebble.h>

extern int tZone;
extern bool changed;

Window *setZoneW;
TextLayer *setZoneW_zone;
TextLayer *setZoneW_label;
TextLayer *setZoneW_disclaim;

char gmt[7];

char* itoa2(int valIN, int base){ // 2 in the morning hack
	static char buf2[32] = {0};
	int i = 30;
	int val = abs(valIN);

	for(; val && i ; --i, val /= base)
		buf2[i] = "0123456789abcdef"[val % base];
	if(valIN<0)
		buf2[i] = '-';
	else if(valIN>0)
		buf2[i] = '+';
	if(valIN == 0)
		return &buf2[i+1];
	return &buf2[i];
	
}

void zone_up(ClickRecognizerRef recognizer, void *context) {
	(void)recognizer;
	(void)context;

	if(tZone<24)
		tZone++;
	strcpy(gmt+3, itoa2(tZone,10));
	text_layer_set_text(setZoneW_zone, gmt);
	changed = true;
}

void zone_down(ClickRecognizerRef recognizer, void *context) {
	(void)recognizer;
	(void)context;

	if(tZone>-24)
		tZone--;
	strcpy(gmt+3, itoa2(tZone,10));
	text_layer_set_text(setZoneW_zone, gmt);
	changed = true;
}

void zone_click_config_provider(void *context) {
	(void)context;

	window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, zone_up);
		
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, zone_down);
}

void showEditTimeZone()
{
	setZoneW = window_create();
	window_set_background_color(setZoneW, GColorBlack);

	strcpy(gmt, "UTC");
	strcpy(gmt+3, itoa2(tZone,10));

	setZoneW_zone = text_layer_create(GRect(0,50,144,48));
	text_layer_set_text(setZoneW_zone, gmt);
	text_layer_set_font(setZoneW_zone, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(setZoneW_zone, GTextAlignmentCenter);
	text_layer_set_text_color(setZoneW_zone, GColorWhite);
	text_layer_set_background_color(setZoneW_zone, GColorBlack);
	layer_add_child(window_get_root_layer(setZoneW), text_layer_get_layer(setZoneW_zone));
	
	setZoneW_label = text_layer_create(GRect(0,5,144,48));
	text_layer_set_text(setZoneW_label, "Change Time Zone");
	text_layer_set_font(setZoneW_label, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(setZoneW_label, GTextAlignmentCenter);
	text_layer_set_text_color(setZoneW_label, GColorWhite);
	text_layer_set_background_color(setZoneW_label, GColorBlack);
	layer_add_child(window_get_root_layer(setZoneW), text_layer_get_layer(setZoneW_label));

	setZoneW_disclaim = text_layer_create(GRect(0,168-31,144,30));
	text_layer_set_text(setZoneW_disclaim, "Not Persistent");
	text_layer_set_font(setZoneW_disclaim, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
	text_layer_set_text_alignment(setZoneW_disclaim, GTextAlignmentCenter);
	text_layer_set_text_color(setZoneW_disclaim, GColorWhite);
	text_layer_set_background_color(setZoneW_disclaim, GColorBlack);
	layer_add_child(window_get_root_layer(setZoneW), text_layer_get_layer(setZoneW_disclaim));

	window_set_click_config_provider(setZoneW, zone_click_config_provider);
	window_stack_push(setZoneW, true);

	changed = true;
}
