import { reactive, readonly, watch } from 'vue'

const isMobile = /Android|iPhone|iPad|iPod/i.test(navigator.userAgent) || 
                 (navigator.maxTouchPoints > 0 && window.innerWidth <= 1024)

// 1. INITIALIZE PERSISTENCE (Load from LocalStorage)
const savedPacketDrop = localStorage.getItem('imu_packetDropMs')
const savedDisplaySync = localStorage.getItem('imu_displayUpdateMs')

const state = reactive({
  packetDropMs: savedPacketDrop !== null ? parseInt(savedPacketDrop) : (isMobile ? 100 : 0),
  displayUpdateMs: savedDisplaySync !== null ? parseInt(savedDisplaySync) : (isMobile ? 0 : 0),

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
  },
  settings: {
    calibration: {
      accel_off: [0,0,0], accel_scale: [1,1,1],
      gyro_off: [0,0,0], mag_bias: [0,0,0],
      mag_scale: [1,1,1], mag_declination: 0.0
    }
  }
})

// --- HIGH-SPEED BUFFERS & LOGIC ---
let bufIMU = { r: 0, p: 0, y: 0, g: null, a: null, m: null }
let bufStats = { cpu: 0, rate: 0, tx: 0, fail: 0 }
let lastUiSyncTime = 0

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

const updateIMU = (r, p, y, g, a, m) => {
  bufIMU = { r, p, y, g, a, m }
  state.stats.packetsReceived++
  const now = performance.now()
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

const fetchSettings = async () => {
  try {
    const response = await fetch('/api/settings')
    if (!response.ok) throw new Error('Network error')
    const data = await response.json()
    if (data.calibration) Object.assign(state.settings.calibration, data.calibration)
  } catch (error) { console.error("Fetch Settings Failed:", error) }
}

// 2. WATCHERS FOR PERSISTENCE
watch(() => state.packetDropMs, (val) => localStorage.setItem('imu_packetDropMs', val))
watch(() => state.displayUpdateMs, (val) => localStorage.setItem('imu_displayUpdateMs', val))

// --- EXPORT ---
export const useIMUStore = () => ({
  state: readonly(state),
  updateIMU,
  updateSystemStats,
  fetchSettings,
  setPacketDrop: (ms) => { state.packetDropMs = ms },
  setDisplayRate: (ms) => { state.displayUpdateMs = ms },
  setConnected: (s) => { state.stats.connected = s },
  setCalibrating: (s) => { state.isCalibrating = s },
  setDefaults: () => {
    state.packetDropMs = isMobile ? 100 : 0
    state.displayUpdateMs = isMobile ? 0 : 0
  }
})
