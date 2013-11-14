var initialized = false;

Pebble.addEventListener("ready", function() {
                        console.log("ready called!");
                        initialized = true;
                        });

function appMessageAck(e) {
    console.log("options sent to Pebble successfully");
}

function appMessageNack(e) {
    console.log("options not sent to Pebble: " + e.error.message);
}

function showConfiguration() {
    console.log("showing configuration");
    Pebble.openURL('http://darenzana.free.fr/Pebble/Authenticator/index-dev.html');
}
Pebble.addEventListener("showConfiguration", showConfiguration);

function webViewClosed(e) {
    console.log("configuration closed");
    // webview closed
    if (e.response != '') {
        var options = JSON.parse(decodeURIComponent(e.response));
        console.log("storing options: " + JSON.stringify(options));
        
        //TODO: check options
        window.localStorage.setItem('options', JSON.stringify(options));
        Pebble.sendAppMessage(options, appMessageAck, appMessageNack);
    } else {
        console.log("no options received");
    }
}
Pebble.addEventListener("webviewclosed", webViewClosed);
