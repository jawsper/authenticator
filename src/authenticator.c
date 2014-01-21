#include <pebble.h>

#include "configuration.h"
#include "constants.h"
#include "sha1.h"
#include "utils.h"

// defined in editTzone.c
extern void showEditTimeZone();
extern void destroyEditTimeZone();

// Truncate n decimal digits to 2^n for 6 digits
#define DIGITS_TRUNCATE 1000000

#define SHA1_SIZE 20

Window *window;

TextLayer *label;
TextLayer *token;
TextLayer *ticker;
InverterLayer *bar;

static PropertyAnimation *bar_animation=NULL;

int curToken;
int curToken_orig;

int tZone; // timeZone in minutes

bool changed;


/** Creates progress bar animation
 * @param second the ticker second (0 < s <= 30)
 * @see animate_bar()
 * @return the newly created *PropertyAnimation
 */
PropertyAnimation * create_bar_animation(int second) {

    PropertyAnimation *bar_animation;
    GRect from_frame = layer_get_frame(text_layer_get_layer(ticker));
    from_frame.origin.x=0;
    from_frame.origin.y=5;
    from_frame.size.h -= 10;
    from_frame.size.w  = second * from_frame.size.w / 30;
    
    GRect to_frame = from_frame ;
    to_frame.size.w=0 ;
    
    bar_animation = property_animation_create_layer_frame(inverter_layer_get_layer(bar), &from_frame, &to_frame);
    animation_set_duration((Animation *)bar_animation, second * 1000);
    animation_set_delay((Animation *)bar_animation, 0);
    animation_set_curve((Animation *)bar_animation, AnimationCurveLinear);
    return bar_animation;
}

/** Animates progress bar
 * @param second the ticker second (0 < s <= 30)
 */
void animate_bar(int second) {
    
    static bool fullanimation=false ;
    
    //first call, create and run partial bar
    if (bar_animation == NULL) {
        bar_animation = create_bar_animation(second);
        if (second ==30) fullanimation=true ;
        animation_schedule((Animation *)bar_animation);
        return;
    }
    
    // we only need to relaunch animation every 30 seconds.
    if (second != 30) return ;
    
    // partial animation is finished, now create full animation
    if (!fullanimation) {
        property_animation_destroy(bar_animation) ;
        bar_animation = create_bar_animation(30);
        fullanimation = true ;
    }
    
    animation_schedule((Animation *)bar_animation);
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

    (void) units_changed;
	static char tokenText[] = "RYRYRY"; // Needs to be static because it's used by the system later.
    static char tickerText[3];
	sha1nfo s;
	uint8_t ofs;
	uint32_t otp;
	uint32_t unix_time;
    time_t current_time ;
	char sha1_time[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int curSeconds;

    current_time=time(NULL);
    unix_time=current_time + ((0-tZone)*60) ; //still needed because time() is not GMT
    unix_time /= 30;
    if (tick_time == NULL) {
        tick_time = localtime(&current_time);
        // currently (SDK 2.0-B2), gmtime() and localtime() are the same
    }
	curSeconds = tick_time->tm_sec;

	if(curSeconds == 0 || curSeconds == 30 || changed)
	{
		changed = false;

		// TOTP uses seconds since epoch in the upper half of an 8 byte payload
		// TOTP is HOTP with a time based payload
		// HOTP is HMAC with a truncation function to get a short decimal key
		sha1_time[4] = (unix_time >> 24) & 0xFF;
		sha1_time[5] = (unix_time >> 16) & 0xFF;
		sha1_time[6] = (unix_time >> 8) & 0xFF;
		sha1_time[7] = unix_time & 0xFF;

		// First get the HMAC hash of the time payload with the shared key
		sha1_initHmac(&s, otpkeys[curToken], otpsizes[curToken]);
		sha1_write(&s, sha1_time, 8);
		sha1_resultHmac(&s);
		
		// Then do the HOTP truncation.  HOTP pulls its result from a 31-bit byte
		// aligned window in the HMAC result, then lops off digits to the left
		// over 6 digits.
		ofs=s.state.b[SHA1_SIZE-1] & 0xf;

		otp = ((s.state.b[ofs] & 0x7f) << 24) |
			((s.state.b[ofs + 1] & 0xff) << 16) |
			((s.state.b[ofs + 2] & 0xff) << 8) |
			(s.state.b[ofs + 3] & 0xff);
		otp %= DIGITS_TRUNCATE;
		
		// Convert result into a string.
		snprintf(tokenText, 7, "%06lu", otp);

		text_layer_set_text(label, otplabels[curToken]);
		text_layer_set_text(token, tokenText);
        
        //set backlight on for a short period of time
        light_enable_interaction();
	}
    
    int tickerInt= curSeconds < 30 ? 30 - curSeconds : 60 - curSeconds ;
    
    snprintf(tickerText, 3, "%02d", tickerInt) ;
    text_layer_set_text(ticker, tickerText);
    
    // progress bar animation
    animate_bar(tickerInt);
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    (void)recognizer ;
    (void)context ;
    
	if (curToken==0) {
		curToken=NUM_SECRETS-1;
	} else {
		curToken--;
	};
	changed = true;
	handle_second_tick(NULL,SECOND_UNIT);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
    (void)recognizer;
    (void)context;
    
    curToken = (curToken + 1 ) % NUM_SECRETS ;
	changed = true;
	handle_second_tick(NULL,SECOND_UNIT);
}

void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	(void)recognizer;
	(void)context;

	showEditTimeZone();
}

void click_config_provider(void *context) {
    window_single_repeating_click_subscribe(BUTTON_ID_UP,   100,   up_single_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_single_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}

// Callbaks for App messages
void out_sent_handler(DictionaryIterator *sent, void *context) {
    // outgoing message was delivered
    log_handler_called("out_sent");
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    // outgoing message failed
    log_handler_called("out_failed");
}

void in_received_handler(DictionaryIterator *received, void *context) {
    // incoming message received
    log_handler_called("in_received");
    
    // Check for fields you expect to receive
    Tuple *timezone_tuple = dict_find(received, AKEY_TIMEZONE);
    
    // Act on the found fields received
    if (timezone_tuple) {
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "Timezone Tuple: length=%d, type=%d", timezone_tuple->length, timezone_tuple->type);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "received Timezone: %ld", timezone_tuple->value->int32);
        tZone=timezone_tuple->value->int32 ;
        changed=true;
        handle_second_tick(NULL, SECOND_UNIT);
    }
}

void in_dropped_handler(AppMessageResult reason, void *context) {
    // incoming message dropped
    log_handler_called("in_dropped");
}


void init(void) {
    // get timezone from persistent storage, if found
	tZone = persist_exists(KEY_TZONE) ? persist_read_int(KEY_TZONE) : DEFAULT_TIME_ZONE;
    
    // get saved current token
    curToken = persist_exists(KEY_CURTOKEN) ? persist_read_int(KEY_CURTOKEN) : 0 ;
    curToken_orig = curToken;
    
	changed = true;

	window = window_create();
    window_stack_push(window, true /* Animated */);
	window_set_background_color(window, GColorBlack);
    Layer *window_layer=window_get_root_layer(window);

	// Init the identifier label
    label=text_layer_create_with_options(window_layer,
                             GRect(5, 30, 144-4, 168-44),
                             GColorWhite, GColorClear, GTextAlignmentLeft,
                             FONT_KEY_GOTHIC_28_BOLD);
	// Init the token label
	token=text_layer_create_with_options(window_layer,
                             GRect(10, 60, 144-4, 168-44),
                             GColorWhite, GColorClear, GTextAlignmentLeft,
                             FONT_KEY_BITHAM_34_MEDIUM_NUMBERS);
	// Init the second ticker
	ticker=text_layer_create_with_options(window_layer,
                             GRect(0, 120, 144, 24),
                             GColorWhite, GColorBlack, GTextAlignmentCenter,
                             FONT_KEY_GOTHIC_18_BOLD);
    
    //Init inverter layer for the progress bar
    struct GRect barRect=layer_get_frame(text_layer_get_layer(ticker));
    barRect.origin=GPointZero;
    bar=inverter_layer_create(barRect);
    layer_add_child(text_layer_get_layer(ticker), inverter_layer_get_layer(bar));
    
	handle_second_tick(NULL, SECOND_UNIT);

    window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
    
    // setup communication with the phone (for configuration)
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);
    const uint32_t inbound_size = 64;
    const uint32_t outbound_size = 64;
    app_message_open(inbound_size, outbound_size);
}

void deinit(void) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "deinit called");
    
    //save selected token
    if (curToken != curToken_orig) {
        if (curToken == 0) persist_delete(KEY_CURTOKEN);
        else persist_write_int(KEY_CURTOKEN, curToken);
    }
    
    destroyEditTimeZone();
    tick_timer_service_unsubscribe();
    property_animation_destroy(bar_animation);
    text_layer_destroy(label) ;
    text_layer_destroy(token) ;
    text_layer_destroy(ticker) ;
    inverter_layer_destroy(bar) ;
    window_destroy(window);
    app_message_deregister_callbacks();
}

int main() {
    init();
    tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
    app_event_loop() ;
    deinit() ;
}
