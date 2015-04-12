var initialized = false;
var timezone=null ;

//! Event Listener : "ready"
//! Called at initialisation of the app, this is where initalization must be done.
function readyEventListener() {
    console.log("ready called!");
    initialized=true ;
    var found_options=false ;
    var options={} ;
    
    //Load config
    var stored_timezone = parseInt(window.localStorage.getItem('timezone'),10);
    if (!isNaN(stored_timezone)) {
        found_options=true ;
        timezone=stored_timezone;
        options["timezone"]=timezone;
        console.log("added " + timezone + " to options");
    }
    
    if (found_options) {
        console.log("sending options:" + JSON.stringify(options));
        Pebble.sendAppMessage(options, appMessageAck, appMessageNack) ;
    }
}
Pebble.addEventListener("ready", readyEventListener);

//! Event Listener: "showConfiguration"
//! called when the user presses the config button on the phone app.
function showConfigurationEventListener() {
    console.log("showing configuration");
    var confURL="http://jawsper.github.io/authenticator/settings.html" ;
    if (!isNaN(timezone)) confURL += "?timezone=" + timezone ;
    console.log("URL :" + confURL);
    Pebble.openURL(confURL);
}
Pebble.addEventListener("showConfiguration", showConfigurationEventListener);

// called after a successful send
function appMessageAck(e) {
    console.log("options sent to Pebble successfully");
}

// called after a failed send
function appMessageNack(e) {
    console.log("options not sent to Pebble. e is " + JSON.stringify(e)) ;
}

//store an option to localstorage
function storeOption(key, value) {
    window.localStorage.setItem(key, value.toString());
    console.log("stored " + key + ": " + value.toString());
    if (key == 'timezone') timezone=value ;
}

//! Event listener: webviewclosed
//! called when user presses the 'done' button on the settings screen
function webviewclosedEventListener(e) {
    console.log("configuration closed");
    // webview closed
    if (e.response) {
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
