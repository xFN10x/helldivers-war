export type HBMap = {
  time: number
  code: number
  message: string
  data: {
    season: number
    bucket: number
    events: string
    fronts: {
      bugs: Array<{
        id: number
        region: string
        capital: string
        points: number
        pointsMax: number
        percent: number
        status: string
        event: string
      }>
      cyborgs: Array<{
        id: number
        region: string
        capital: string
        points: number
        pointsMax: number
        percent: number
        status: string
        event: string
      }>
      illuminate: Array<{
        id: number
        region: string
        capital: string
        points: number
        pointsMax: number
        percent: number
        status: string
        event: string
      }>
      superEarth: Array<{
        id: number
        region: string
        capital: string
        points: number
        pointsMax: number
        percent: number
        status: string
        event: string
      }>
    }
    activeEvents: Array<{
      type: string
      enemy: string
      enemyId: number
      region: number
      status: string
      points: number
      pointsMax: number
      startTime: number
      endTime: number
    }>
  }
}


declare var Pebble: any
