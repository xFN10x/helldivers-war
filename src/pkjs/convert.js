//@ts-check

/** @param {import(".").HBMap} */
function convertJsonToHNMapData(obj) {
    const json = obj.data;
    const mapData0 = new Int16Array(4)
    const mapData1 = new Int32Array(8)

    //uint16_t war;
    mapData0[0] = json.season
    
    let bugPlanetData = 0b0000000000000000;
    for (const planet of json.fronts.bugs) {
        if (planet.status === "captured")
        bugPlanetData |= (0b1000000000000000 >> (planet.id - 1))

        if (planet.id == 11 && planet.status === "active") {
            bugPlanetData |= 0b0000000000000001;
        }
    }
    console.log("Bug data is: " + bugPlanetData.toString(2).padEnd(16,"0") + ` ${bugPlanetData}`)

    let botPlanetData = 0b0000000000000000;
    for (const planet of json.fronts.cyborgs) {
        if (planet.status === "captured")
        botPlanetData |= (0b1000000000000000 >> (planet.id - 1))

        if (planet.id == 11 && planet.status === "active") {
            botPlanetData |= 0b0000000000000001;
        }
    }
    console.log("Bot data is: " + botPlanetData.toString(2).padEnd(16, "0") + ` ${botPlanetData}`)

    let illumPlanetData = 0b0000000000000000;
    for (const planet of json.fronts.illuminate) {
        if (planet.status === "captured")
        illumPlanetData |= (0b1000000000000000 >> (planet.id - 1))

        if (planet.id == 11 && planet.status === "active") {
            illumPlanetData |= 0b0000000000000001;
        }
    }
    console.log("Illuminate data is: " + illumPlanetData.toString(2).padEnd(16, "0") + ` ${illumPlanetData}`)

    //uint16_t bugs;
    mapData0[1] = bugPlanetData
    //uint16_t bots;
    mapData0[2] = botPlanetData
    //uint16_t illum;
    mapData0[3] = illumPlanetData
}

module.exports.convertJsonToHNMapData = convertJsonToHNMapData;