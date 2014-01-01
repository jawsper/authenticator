var secrets = [];

var timezone = -5;

var MSG_GET_CONFIGURATION = 0;
var MSG_SET_CONFIGURATION = 1;
var MSG_SET_SECRET = 2;

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
      secretKey: b32decode(secrets[s].key)
    }
    sendMessage(message);
  }
  sendMessage({
    messageType: MSG_SET_CONFIGURATION,
    numSecrets: secrets.length,
    timeZone: timezone
  });
  if (secrets.length == 0) {
    var message = {
      messageType: MSG_SET_SECRET,
      secretIndex: 0,
      secretLabel: "(no accounts)",
      secretKey: []
    }
    sendMessage(message);
  }
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

function showConfigPanel() {
  var configInfo = {
    "secrets": secrets
  };
  var configJson = JSON.stringify(configInfo);
  while (configJson.length % 3) {
    configJson += ' ';
  }
  var url = "data:text/html;base64,"+htmlpre+btoa(configJson)+htmlpost;
  Pebble.openURL(url);
}

function configPanelReturn(e) {
  var response = JSON.parse(e.response);
  if (!response || !response.secrets) return;
  secrets = response.secrets;
  for (var s = 0; s < secrets.length; s++) {
    if (secrets[s].key.trim().length == 0 || secrets[s].label.trim().length == 0) {
      secrets.splice(s, 1);
      s--;
    }
  }
  sendConfiguration();
  localStorage.secrets = JSON.stringify(secrets);
}

Pebble.addEventListener("ready",
  function(e) {
    Pebble.addEventListener("appmessage", appMessageListener);
    Pebble.addEventListener("showConfiguration", showConfigPanel);
    Pebble.addEventListener("webviewclosed", configPanelReturn);
    if (localStorage && localStorage.secrets) {
      secrets = JSON.parse(localStorage.secrets) || [];
    }
    console.log("JavaScript app ready and running!");
  }
);

var b32decode = (function() {
  var alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567';
  var alias = { 0:'O', 1:'L', 8:'B' };

  var lookup = {}
  for (var i = 0; i < alphabet.length; i++) {
      lookup[alphabet[i]] = i
  }
  for (var key in alias) {
      if (!alias.hasOwnProperty(key)) continue
      lookup[key] = lookup['' + alias[key]]
  }

  function b32decode(input) {
    var bits = 0;
    var value = 0;
    var output = [];

    for (var i = 0; i < input.length; i++) {
      var cval = lookup[input[i].toUpperCase()];
      if (cval == null) continue;
      bits += 5;
      value <<= 5;
      value |= cval;
      if (bits >= 8) {
        bits -= 8;
        output.push(value >>> bits);
        value %= (1 << bits);
      }
    }

    return output;
  }

  return b32decode;
})();
