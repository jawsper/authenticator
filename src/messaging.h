#ifndef MESSAGING_H
#define MESSAGING_H

enum APP_KEYS {
	AKEY_MESSAGE_TYPE = 0,
	AKEY_SECRET_INDEX = 1,
	AKEY_SECRET_LABEL = 2,
	AKEY_SECRET_KEY = 3,
	AKEY_NUM_SECRETS = 4,
	AKEY_TIMEZONE = 5
};

enum MESSAGE_TYPES {
	MSG_GET_CONFIGURATION = 0,
	MSG_SET_CONFIGURATION,
	MSG_SET_SECRET
};

void messaging_init();
void messaging_request_configuration();

#endif
