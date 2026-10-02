//@ts-check
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
                    let json = dict[key]
                    /**@type {import("./").HBMap} */
                    let res = JSON.parse(this.responseText)
                    let resData = res.data;
                    const hndata = convert.convertJsonToHNMapData(JSON.parse(this.responseText));
                    const season = json[1] << 8 | json[0]
                    const bugs = json[3] << 8 | json[2];
                    const bots = json[5] << 8 | json[4];
                    const illum = json[7] << 8 | json[6];
                    //00000101 10000000
                    //el. 0       1
                    //10000000 00000101

                    console.log("War is: " + (season))
                    console.log("Bugs is: " + (bugs))
                    console.log("New Bugs is: " + (hndata[1]))
                    console.log("Bots is: " + (bots))
                    console.log("New Bots is: " + (hndata[2]))
                    console.log("Illum is: " + (illum))
                    console.log("New Illum is: " + (hndata[3]))
                    
                    const changed = (season != resData.season ||
                        bugs != hndata[1] ||
                        bots != hndata[2] ||
                        illum != hndata[3]
                    )
                    sendMessage({"HBMapUpdated": changed})
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

//console.log(env.api_key);

Pebble.addEventListener("appmessage", onMessage);
Pebble.addEventListener("ready", onReady);
