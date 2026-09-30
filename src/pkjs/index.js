//@ts- check
const env = require("./env");
const convert = require('./convert');
// https://helldivers.bot/docs/api
const api = "https://helldivers.bot/api/";


function sendMessage(dict) {
    console.log(`Sending message: \n${JSON.stringify(dict)}`);

    Pebble.sendAppMessage(dict);
}

function onReady() {
    console.log("Helldivers Notifier ready!");
    sendMessage({
        ready: 1,
    });

    return;
}

/** @param {any} */
function onMessage(event) {
    // Get the dictionary from the message
    var dict = event.payload;

    console.log("Got message: " + JSON.stringify(dict));

    for (const key in dict) {
        switch (key) {
            case "HBPing":
                console.log("Pinging!");
                var req = new XMLHttpRequest();
                req.onload = function () {
                    console.log(`Seems like helldivers bot is online; got ${this.responseType}`)
                    sendMessage({ "HBPing": 1 })
                }

                req.open("HEAD", api + "h1/campaign");
                req.send();
                break;
                
            case "HBMapUpdated":
                console.log("Getting most recent map data...");
                var req = new XMLHttpRequest();
                req.onload = function () {
                    convert.convertJsonToHNMapData(JSON.parse(this.responseText))
                }
                req.open("GET", api + "v1/h1/map");
                req.setRequestHeader("Authorization", "Bearer " + env.api_key)
                req.send();
                break;

            default:
                break;
        }
    }
}

console.log(env.api_key);

Pebble.addEventListener("appmessage", onMessage);
Pebble.addEventListener("ready", onReady);
