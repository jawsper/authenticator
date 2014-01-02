#include <pebble.h>
#include <time.h>

#include "sha1.h"
#include "messaging.h"
#include "data.h"

int num_secrets = 0;
char otplabels[MAX_SECRETS][LABEL_MAXSIZE] = {};
unsigned char otpkeys[MAX_SECRETS][KEY_MAXSIZE] = {};
int otpsizes[MAX_SECRETS] = {};
time_t last_tzone_sync = 0;

// Truncate n decimal digits to 2^n for 6 digits
#define DIGITS_TRUNCATE 1000000

#define SHA1_SIZE 20

Window *window;

TextLayer *label;
TextLayer *token;
TextLayer *ticker;
int curToken = 0;
int utc_offset = 0;
bool changed;

char* itoa(int val, int base){
	static char buf[32] = {0};
	int i = 30;
	for(; val && i ; --i, val /= base)
		buf[i] = "0123456789abcdef"[val % base];
	return &buf[i+1];
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {

	(void)tick_time;
	(void)units_changed;

	static char tokenText[] = "RYRYRY"; // Needs to be static because it's used by the system later.

	sha1nfo s;
	uint8_t ofs;
	uint32_t otp;
	int i;
	int curSeconds;

	uint32_t unix_time;
	char sha1_time[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	unix_time = time(NULL) - utc_offset;
	curSeconds = unix_time % 30;

	if (curToken >= num_secrets && num_secrets > 0) {
		curToken = num_secrets - 1;
	}

	if (num_secrets == 0) {
		text_layer_set_text(label, "Loading...");
		text_layer_set_text(token, "------");
	} else if (otpsizes[curToken] <= 0) {
		if (strlen(otplabels[curToken])) {
			text_layer_set_text(label, otplabels[curToken]);
		} else {
			text_layer_set_text(label, "Loading...");
		}
		switch(otpsizes[curToken]) {
			case KERR_UNINITIALIZED:
				text_layer_set_text(token,"------");
				break;
			case KERR_MAXSIZE:
				text_layer_set_text(token,"-----.");
				break;
			default:
				text_layer_set_text(token,"......");
				break;
		}
	} else if (curSeconds == 0 || changed) {
		changed = false;

		// TOTP uses seconds since epoch in the upper half of an 8 byte payload
		// TOTP is HOTP with a time based payload
		// HOTP is HMAC with a truncation function to get a short decimal key
		unix_time /= 30;
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
		otp = 0;
		otp = ((s.state.b[ofs] & 0x7f) << 24) |
			((s.state.b[ofs + 1] & 0xff) << 16) |
			((s.state.b[ofs + 2] & 0xff) << 8) |
			(s.state.b[ofs + 3] & 0xff);
		otp %= DIGITS_TRUNCATE;
		
		// Convert result into a string.  Sure wish we had working snprintf...
		for(i = 0; i < 6; i++) {
			tokenText[5-i] = 0x30 + (otp % 10);
			otp /= 10;
		}
		tokenText[6]=0;

		char *labelText = otplabels[curToken];

		text_layer_set_text(label, labelText);
		text_layer_set_text(token, tokenText);
	}

	text_layer_set_text(ticker, itoa((30-curSeconds),10));
}

void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (num_secrets == 0) {
		curToken = 0;
	} else if (curToken==0) {
		curToken=num_secrets-1;
	} else {
		curToken--;
	};
	changed = true;
	handle_second_tick(NULL,0);
}

void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  (void)recognizer;
  (void)window;
	if (num_secrets == 0) {
		curToken = 0;
	} else if ((curToken+1)==num_secrets) {
		curToken=0;
	} else {
		curToken++;
	};
	changed = true;
	handle_second_tick(NULL,0);
}

void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
	num_secrets = 0;
	messaging_request_configuration();
}

void click_config_provider(void *context) {
  (void)context;

  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_single_click_handler);

  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_single_click_handler);

  window_long_click_subscribe(BUTTON_ID_SELECT, 0, select_long_click_handler, NULL);
}

void handle_deinit() {
	text_layer_destroy(ticker);
	text_layer_destroy(token);
	text_layer_destroy(label);
	window_destroy(window);
}

void handle_init() {
	tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

	messaging_init();

	changed = true;

	window = window_create();
	window_stack_push(window, true /* Animated */);
	window_set_background_color(window, GColorBlack);

	// Init the identifier label
	label = text_layer_create(GRect(5, 30, 144-4, 168-44));
	text_layer_set_text_color(label, GColorWhite);
	text_layer_set_background_color(label, GColorClear);
	text_layer_set_font(label, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));

	// Init the token label
	token = text_layer_create(GRect(10, 60, 144-4 /* width */, 168-44 /* height */));
	text_layer_set_text_color(token, GColorWhite);
	text_layer_set_background_color(token, GColorClear);
	text_layer_set_font(token, fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS));

	// Init the second ticker
	ticker = text_layer_create(GRect(60, 120, 144-4 /* width */, 168-44 /* height */));
	text_layer_set_text_color(ticker, GColorWhite);
	text_layer_set_background_color(ticker, GColorClear);
	text_layer_set_font(ticker, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

	handle_second_tick(NULL, 0);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(label));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(token));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(ticker));

	window_set_click_config_provider(window, click_config_provider);

	if (time(NULL) > last_tzone_sync + 3600 || num_secrets == 0) {
		messaging_request_configuration();
	}
}


int main() {
	handle_init();
	app_event_loop();
	handle_deinit();
}
