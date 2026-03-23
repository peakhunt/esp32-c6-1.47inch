import { reactive, readonly } from 'vue'

const isMobile = /Android|iPhone|iPad|iPod/i.test(navigator.userAgent) || 
                 (navigator.maxTouchPoints > 0 && window.innerWidth <= 1024)

const state = reactive({
  // --- PERFORMANCE CONFIG ---
  // Mobile: Hard drop to 10Hz to save CPU. Desktop: Keep all 50Hz.
  packetDropMs: isMobile ? 100 : 0, 
  // Mobile: Instant update (since we already dropped). Desktop: 5Hz readable UI.
  displayUpdateMs: isMobile ? 0 : 0, 

  roll: 0.0, pitch: 0.0, yaw: 0.0,
  gyro: { x: 0, y: 0, z: 0 },
  accel: { x: 0, y: 0, z: 0 },
  mag: { x: 0, y: 0, z: 0 },
  cpuUsage: 0,
  samplingRate: 0,
  i2cTransactions: 0,
  i2cFailed: 0,
  isCalibrating: false,
  stats: {
    packetsReceived: 0,
    connected: false,
  }
})

// Internal Buffers for the "Display Sync" logic
let bufIMU = { r: 0, p: 0, y: 0, g: null, a: null, m: null }
let bufStats = { cpu: 0, rate: 0, tx: 0, fail: 0 }
let lastPacketTime = 0
let lastUiSyncTime = 0

const updateIMU = (r, p, y, g, a, m) => {
  const now = performance.now()

  // 1. PACKET DROP (Primary Mobile Performance Fix)
  if (state.packetDropMs > 0 && (now - lastPacketTime < state.packetDropMs)) return
  lastPacketTime = now

  // 2. Buffer the data (Always the freshest available)
  bufIMU = { r, p, y, g, a, m }
  state.stats.packetsReceived++

  // 3. DISPLAY SYNC (Primary Desktop Readability Fix)
  // If displayUpdateMs is 0 (Mobile), this fires every packet.
  if (now - lastUiSyncTime >= state.displayUpdateMs) {
    syncToState()
    lastUiSyncTime = now
  }
}

const updateSystemStats = (cpu, rate, tx, fail) => {
  bufStats = { cpu, rate, tx: Number(tx), fail: Number(fail) }
  
  const now = performance.now()
  if (now - lastUiSyncTime >= state.displayUpdateMs) {
    syncToState()
    lastUiSyncTime = now
  }
}

const syncToState = () => {
  state.roll = bufIMU.r; state.pitch = bufIMU.p; state.yaw = bufIMU.y
  if (bufIMU.g) state.gyro = bufIMU.g
  if (bufIMU.a) state.accel = bufIMU.a
  if (bufIMU.m) state.mag = bufIMU.m
  
  state.cpuUsage = bufStats.cpu
  state.samplingRate = bufStats.rate
  state.i2cTransactions = bufStats.tx
  state.i2cFailed = bufStats.fail
}

export const useIMUStore = () => ({
  state: readonly(state),
  setPacketDrop: (ms) => { state.packetDropMs = ms },
  setDisplayRate: (ms) => { state.displayUpdateMs = ms },
  updateIMU,
  updateSystemStats,
  setConnected: (s) => { state.stats.connected = s },
  setCalibrating: (s) => { state.isCalibrating = s }
})
