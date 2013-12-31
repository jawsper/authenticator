var secrets = [
  {
    label: "fake",
    key: [ 0x7C, 0x94, 0x50, 0xEA, 0xA7, 0x2A, 0x08, 0x66, 0xA3, 0x47 ]
  }
];

var timezone = -5;

var MSG_GET_CONFIGURATION = 0;
var MSG_SET_CONFIGURATION = 1;
var MSG_SET_SECRET = 2;

function sendReady() {
  Pebble.sendAppMessage({
    messageType: PHO_READY
  }, function(e) {}, function(e) {});
}

var globalMsgIdx = 0;
var messageQueue = [];

function messageQueueSend() {
  if (messageQueue.length == 0) return;
  var curMessage = messageQueue[0];
  var msgIdx = curMessage[0];
  console.log("Sending message from queue "+msgIdx);
  Pebble.sendAppMessage(curMessage[1],
    function(e) {
      console.log("Message send successful "+msgIdx);
      messageQueue.shift();
      messageQueueSend();
    },
    function(e) {
      console.log("Message send failed, retrying "+msgIdx);
      messageQueueSend();
    }
  );
}


function sendMessage(payload) {
  var msgIdx = globalMsgIdx++;
  messageQueue.push([msgIdx, payload]);
  console.log("Queueing message "+msgIdx+" type "+payload.messageType);
  if (messageQueue.length == 1) {
    messageQueueSend();
  }
}


function sendConfiguration() {
  for (var s = 0; s < secrets.length; s++) {
    var message = {
      messageType: MSG_SET_SECRET,
      secretIndex: s,
      secretLabel: secrets[s].label,
      secretKey: secrets[s].key
    }
    sendMessage(message);
  }
  sendMessage({
    messageType: MSG_SET_CONFIGURATION,
    numSecrets: secrets.length,
    timeZone: timezone
  });
}

function appMessageListener(e) {
  console.log("Got message!");
  console.log(e.payload.messageType);
  switch(e.payload.messageType) {
    case MSG_GET_CONFIGURATION:
      sendConfiguration();
      break;
  }
}

Pebble.addEventListener("ready",
  function(e) {
    Pebble.addEventListener("appmessage", appMessageListener);
    console.log("JavaScript app ready and running!");
    sendReady();
  }
);
