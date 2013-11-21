var initialized = false;

//! Event Listener : "ready"
//! Called at initialisation of the app, this is where initalization must be done.
function readyEventListener() {
    console.log("ready called!");
    initialized=true ;
    
    var options={} ;
    
    //Load config
    var timezone = parseInt(window.localStorage.getItem('timezone'));
    if (!isNaN(timezone)) {
        options["timezone"]=timezone;
        console.log("added " + timezone + " to options");
    }

    console.log("sending options:" + JSON.stringify(options));
    Pebble.sendAppMessage(options, appMessageAck, appMessageNack) ;
}
Pebble.addEventListener("ready", readyEventListener);

//! Event Listener: "showConfiguration"
//! called when the user presses the config button on the phone app.
function showConfigurationEventListener() {
    console.log("showing configuration");
    Pebble.openURL('http://darenzana.free.fr/Pebble/Authenticator/index-dev.html');
}
Pebble.addEventListener("showConfiguration", showConfigurationEventListener);

// called after a successful send
function appMessageAck(e) {
    console.log("options sent to Pebble successfully");
}

// called after a failed send
function appMessageNack(e) {
    console.log("options not sent to Pebble: " + e.error.message);
}

//store an option to localstorage
function storeOption(key, value) {
    window.localStorage.setItem(key, value.toString());
    console.log("stored " + key + ": " + value.toString());
}

//! webviewclosed
//! called when user presses the 'done' button on the settings screen
function webviewclosedEventListener(e) {
    console.log("configuration closed");
    // webview closed
    if (e.response != '') {
        var options = JSON.parse(decodeURIComponent(e.response));
        console.log("storing options: " + JSON.stringify(options));
        
        // store options
        if (options.hasOwnProperty('timezone')) {
            storeOption('timezone', options.timezone);
        }
        Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
    } else {
        console.log("no options received");
    }
}
Pebble.addEventListener("webviewclosed", webviewclosedEventListener);

//! Event Listener: "appmessage"
//! called when the pebble app sends a message to the phone app.
//! @param e
function appmessageEventListener(e) {
    console.log("Received message" + JSON.stringify(e.payload)) ;
    if (e.payload.hasOwnProperty('timezone')) {
        storeOption('timezone', e.payload.timezone);
    }
}
Pebble.addEventListener("appmessage", appmessageEventListener);