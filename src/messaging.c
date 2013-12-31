#include <pebble.h>
#include <time.h>
#include <string.h>

#include "data.h"
#include "messaging.h"

static int message_ttl = 0;

static void secret_handler(DictionaryIterator *received) {
	Tuple *index_t, *label_t, *key_t;
	int index;

	index_t = dict_find(received, AKEY_SECRET_INDEX);
	label_t = dict_find(received, AKEY_SECRET_LABEL);
	key_t = dict_find(received, AKEY_SECRET_KEY);

	index = index_t->value->int32;

	APP_LOG(APP_LOG_LEVEL_INFO, "Received secret %d; label_t length %d (%s), key_t length %d", index, (int)label_t->length, label_t->value->cstring, (int)key_t->length);

	if (index >= MAX_SECRETS) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Secret %d is out of range!", index);
		return;
	}

	strncpy(otplabels[index], label_t->value->cstring, LABEL_MAXSIZE-1);
	if (key_t->length <= KEY_MAXSIZE) {
		memcpy(otpkeys[index], key_t->value->data, key_t->length);
		otpsizes[index] = key_t->length;
		if (num_secrets <= index) {
			num_secrets = index+1;
		}
	} else {
		otpsizes[index] = KERR_MAXSIZE;
	}

	changed = true;
}

static void configuration_handler(DictionaryIterator *received) {
	Tuple *numsecs_t, *zone_t;
	int numsecs;

	numsecs_t = dict_find(received, AKEY_NUM_SECRETS);
	zone_t = dict_find(received, AKEY_TIMEZONE);

	numsecs = numsecs_t->value->int32;

	APP_LOG(APP_LOG_LEVEL_INFO, "Received configuration: %d secrets, UTC%+d", numsecs, (int)zone_t->value->int32);

	if (numsecs > MAX_SECRETS) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "%d secrets is out of range!", numsecs);
		numsecs = MAX_SECRETS;
	}

	num_secrets = numsecs;

	tZone = zone_t->value->int32;
	last_tzone_sync = time(NULL);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbound message sent");
}


static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	Tuple *type_tuple = dict_find(failed, AKEY_MESSAGE_TYPE);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbound message failed: %d", reason);
	if (reason == APP_MSG_SEND_TIMEOUT
			&& message_ttl-- > 0
			&& type_tuple->value->int32 == MSG_GET_CONFIGURATION) {
		APP_LOG(APP_LOG_LEVEL_INFO, "Resending get-config");
		messaging_request_configuration();
	}
}


static void in_received_handler(DictionaryIterator *received, void *context) {
	Tuple *type_tuple = dict_find(received, AKEY_MESSAGE_TYPE);

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbound message received: %d", (int)type_tuple->value->int32);

	switch (type_tuple->value->int32) {
		case MSG_SET_SECRET:
			secret_handler(received);
			break;
		case MSG_SET_CONFIGURATION:
			configuration_handler(received);
			break;
	}
}


static void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbound message dropped: %d", reason);
}

static void send_basic_message(enum MESSAGE_TYPES message_type) {
	DictionaryIterator *iter;
	Tuplet value = TupletInteger(AKEY_MESSAGE_TYPE, message_type);
	AppMessageResult result;

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending basic message %d", message_type);
	app_message_outbox_begin(&iter);
	dict_write_tuplet(iter, &value);
	result = app_message_outbox_send();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "  -- message status: %d", result);
	message_ttl = 5;
}

void messaging_init() {
	const uint32_t inbound_size = 64;
	const uint32_t outbound_size = 64;

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Init messaging");
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(inbound_size, outbound_size);
}

void messaging_request_configuration() {
	send_basic_message(MSG_GET_CONFIGURATION);
}
