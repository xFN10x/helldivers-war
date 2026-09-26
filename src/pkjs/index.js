//@ts-check
/** var Pebble: any */

const env = require('./env');

function onReady() {
    console.log("Helldivers Notifier ready!")

    Pebble.sendAppMessage({
        "ready": 1
    })
    return;
}

/** @param {any} */
function onMessage(event) {
// Get the dictionary from the message
  var dict = event.payload;

  console.log('Got message: ' + JSON.stringify(dict));

  for (const key in dict) {
    switch (key) {
        case "HBPing":
            
            break;
    
        default:
            break;
    }
    
    
  }
}

console.log(env.api_key)

Pebble.addEventListener('appmessage', onMessage);
Pebble.addEventListener('ready', onReady)