#include <pebble.h>

extern int tZone;
extern bool changed;

Window *setZoneW=NULL;
TextLayer *setZoneW_zone;
TextLayer *setZoneW_label;

char gmt[10];

extern char* itoa(int val, int base);

//! Writes the offset using format (+/-)HH:MM into s.
//! Will write '+01H30' in s if val=90
//! @return s
//! @param val offset in minutes
//! @param s   string to copy

char * min_to_hour(char *s,size_t max, int val) {
    if (val==0) {
        *s='\0';
        return s;
    }

    char *p=s;
    // add sign
    *p++= (val < 0) ? '-' : '+' ;
    max--;
    val=abs(val);
    
    struct tm to_convert={.tm_min=val%60,.tm_hour=val/60,
        .tm_isdst=0,.tm_gmtoff=0};
    strftime(p, max, "%H:%M", &to_convert);
    return s;
}

void zone_up(ClickRecognizerRef recognizer, void *context) {
	(void)recognizer;
	(void)context;

	if(tZone<=14*60) {
        tZone+=(click_number_of_clicks_counted(recognizer) > 1) ? 60: 30;
    } else {
        vibes_short_pulse();
    }
    min_to_hour(gmt+3, 7, tZone);
	text_layer_set_text(setZoneW_zone, gmt);
	changed = true;
}

void zone_down(ClickRecognizerRef recognizer, void *context) {
	(void)recognizer;
	(void)context;

	if(tZone>=-11*60) {
		tZone-=(click_number_of_clicks_counted(recognizer) > 1) ? 60: 30;
    } else {
        vibes_short_pulse() ;
    }
	min_to_hour(gmt+3, 7, tZone);
	text_layer_set_text(setZoneW_zone, gmt);
	changed = true;
}

void zone_click_config_provider(void *context) {

	window_single_repeating_click_subscribe(BUTTON_ID_UP,   100, zone_up);
	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, zone_down);

}

void create_setZoneW() {
    setZoneW=window_create();
    Layer *setZoneW_layer=window_get_root_layer(setZoneW);
	window_set_background_color(setZoneW, GColorBlack);
    
	strcpy(gmt, "UTC");
	min_to_hour(gmt+3, 7,tZone);
    
	setZoneW_zone=text_layer_create(GRect(20,50,144,48));
	
    text_layer_set_text(            setZoneW_zone, gmt);
	text_layer_set_font(            setZoneW_zone,
                        fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(  setZoneW_zone, GTextAlignmentLeft);
	text_layer_set_text_color(      setZoneW_zone, GColorWhite);
	text_layer_set_background_color(setZoneW_zone, GColorBlack);
	
    layer_add_child(setZoneW_layer, text_layer_get_layer(setZoneW_zone));
	
	setZoneW_label=text_layer_create(GRect(0,5,144,48));
    
	text_layer_set_text(            setZoneW_label, "Change Time Zone");
	text_layer_set_font(            setZoneW_label,
                        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_text_alignment(  setZoneW_label, GTextAlignmentCenter);
	text_layer_set_text_color(      setZoneW_label, GColorWhite);
	text_layer_set_background_color(setZoneW_label, GColorBlack);
    
	layer_add_child(setZoneW_layer, text_layer_get_layer(setZoneW_label));
    
	window_set_click_config_provider(setZoneW, (ClickConfigProvider) zone_click_config_provider);
}

void destroyEditTimeZone() {
    if (setZoneW == NULL) return ;
    
    text_layer_destroy(setZoneW_label);
    text_layer_destroy(setZoneW_zone);
    window_destroy(setZoneW);
    setZoneW=NULL ;
}

void showEditTimeZone()
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "showEditTimeZone: setZoneW==%p", setZoneW);
    if (setZoneW == NULL) {
        create_setZoneW();
    } else {
        // tZone may have changed via JS-config
        min_to_hour(gmt+3, 7, tZone);
        text_layer_set_text(setZoneW_zone, gmt);
    }
	window_stack_push(setZoneW, true);

	changed = true;
}

