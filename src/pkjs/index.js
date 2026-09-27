//@ts- check
const env = require("./env")

function sendMessage(dict) {
    console.log(
        `Sending message at size ${new TextEncoder().encode(JSON.stringify(dict)).length} bytes: \n${dict}`
    )

    Pebble.sendMessage(dict)
}

function onReady() {
    console.log("Helldivers Notifier ready!")
    sendMessage({
        ready: 1,
    })

    return;
}

/** @param {any} */
function onMessage(event) {
    // Get the dictionary from the message
    var dict = event.payload;

    console.log("Got message: " + JSON.stringify(dict))

    for (const key in dict) {
        switch (key) {
            case "HBPing":
                console.log("Pinging!")
                break

            default:
                break
        }
    }
}

console.log(env.api_key)

Pebble.addEventListener("appmessage", onMessage)
Pebble.addEventListener("ready", onReady)
