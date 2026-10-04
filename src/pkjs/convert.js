//@ts-check

/** @param {import("./").HBMap} */
function convertJsonToHNMapData(obj) {
    const data = _convertJsonToHNMapData(obj);
    const array = Array.from(data[0]).concat(Array.from(data[1]));
    console.log(JSON.stringify(array))
    return array;
}

/** @param {import("./").HBMap} */
function convertJsonToHNMapData_bytes(obj) {
    const data = _convertJsonToHNMapData(obj);
    const mapdata0 = data[0]
    const mapdata1 = data[1]
    const mapdata0bytes = new Uint8Array(mapdata0.buffer, mapdata0.byteOffset, mapdata0.byteLength)
    const mapdata1bytes = new Uint8Array(mapdata1.buffer, mapdata1.byteOffset, mapdata1.byteLength)
    const array = Array.from(mapdata0bytes).concat(Array.from(mapdata1bytes));
    console.log(JSON.stringify(array))
    return array;
}

/** @param {import("./").HBMap} */
function _convertJsonToHNMapData(obj) {
    const json = obj.data;
    const mapData0 = new Uint16Array(4)
    const mapData1 = new Uint32Array(8)

    //uint16_t war;
    mapData0[0] = json.season

    let bugPointsMax = 0;
    let botPointsMax = 0;
    let illumPointsMax = 0;

    let bugPointsTaken = 0;
    let botPointsTaken = 0;
    let illumPointsTaken = 0;

    let bugPlanetData = 0b0000000000000000;
    for (const planet of json.fronts.bugs) {
        if (planet.status === "captured")
            bugPlanetData |= (0b1000000000000000 >> (planet.id - 1))

        if (planet.id == 11 && planet.status === "active") {
            bugPlanetData |= 0b0000000000000001;
        }
        if (planet.status === "in_progress") {
            bugPointsTaken = planet.points;
            bugPointsMax = planet.pointsMax;
        }
    }
    console.log("Bug data is: " + bugPlanetData.toString(2).padEnd(16, "0") + ` ${bugPlanetData}`)

    let botPlanetData = 0b0000000000000000;
    for (const planet of json.fronts.cyborgs) {
        if (planet.status === "captured")
            botPlanetData |= (0b1000000000000000 >> (planet.id - 1))

        if (planet.id == 11 && planet.status === "active") {
            botPlanetData |= 0b0000000000000001;
        }
        if (planet.status === "in_progress") {
            botPointsTaken = planet.points;
            botPointsMax = planet.pointsMax;
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
        if (planet.status === "in_progress") {
            illumPointsTaken = planet.points;
            illumPointsMax = planet.pointsMax;
        }
    }
    console.log("Illuminate data is: " + illumPlanetData.toString(2).padEnd(16, "0") + ` ${illumPlanetData}`)

    //uint16_t bugs;
    mapData0[1] = bugPlanetData
    //uint16_t bots;
    mapData0[2] = botPlanetData
    //uint16_t illum;
    mapData0[3] = illumPlanetData

    mapData1[0] = bugPointsMax;
    mapData1[1] = bugPointsTaken;

    mapData1[2] = botPointsMax;
    mapData1[3] = botPointsTaken;

    mapData1[4] = illumPointsMax;
    mapData1[5] = illumPointsTaken;

    mapData1[6] = json.fronts.superEarth[0].pointsMax;
    mapData1[7] = json.fronts.superEarth[0].points;

    let returning = {}
    returning[0] = mapData0;
    returning[1] = mapData1;
    return returning;
}

module.exports.convertJsonToHNMapData = convertJsonToHNMapData;
module.exports.convertJsonToHNMapData_bytes = convertJsonToHNMapData_bytes;