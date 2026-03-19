import { reactive, readonly } from 'vue'

const state = reactive({
  roll: 0.0,
  pitch: 0.0,
  yaw: 0.0,
  // System Metrics
  cpuUsage: 0,
  samplingRate: 0,
  i2cTransactions: 0,
  i2cFailed: 0,
  stats: {
    packetsReceived: 0,
    errors: 0,
    connected: false,
    startTime: Date.now()
  }
})

// High-frequency orientation (The "Old Way")
const updateIMU = (r, p, y) => {
  state.roll = r
  state.pitch = p
  state.yaw = y
  state.stats.packetsReceived++
}

// System health metrics (The new hardware data)
const updateSystemStats = (cpu, rate, tx, fail) => {
  state.cpuUsage = cpu
  state.samplingRate = rate
  state.i2cTransactions = Number(tx)
  state.i2cFailed = Number(fail)
}

const setConnected = (status) => {
  state.stats.connected = status
}

export const useIMUStore = () => {
  return {
    state: readonly(state),
    updateIMU,
    updateSystemStats,
    setConnected
  }
}
