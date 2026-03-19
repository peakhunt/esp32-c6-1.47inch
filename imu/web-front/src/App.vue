<template>
  <section class="section has-background-light" style="min-height: 100vh;">
    <div class="container">
      
      <div class="mb-6">
        <h1 class="title has-text-centered is-size-3 has-text-black has-text-weight-black">IMU Status</h1>
      </div>

      <!-- Main Stats Container -->
      <div class="columns is-mobile is-multiline mb-5 has-background-white shadow-card p-4">

        <!-- STATUS: Half width on mobile, Auto on desktop -->
        <div class="column is-6-mobile has-text-centered">
          <p class="heading">STATUS</p>
          <div :class="['status-indicator', state.stats.connected ? 'is-live' : 'is-off']">
            <span class="tag is-rounded has-text-weight-bold">
              {{ state.stats.connected ? 'LIVE' : 'OFFLINE' }}
            </span>
          </div>
        </div>

        <!-- CPU: Half width on mobile -->
        <div class="column is-6-mobile has-text-centered">
          <p class="heading">CPU</p>
          <p class="title is-5 has-text-black">{{ state.cpuUsage }}%</p>
        </div>

        <!-- LOOP: Half width on mobile -->
        <div class="column is-6-mobile has-text-centered">
          <p class="heading">LOOP</p>
          <p class="title is-5 has-text-black">{{ state.samplingRate }}Hz</p>
        </div>

        <!-- I2C TX: Half width on mobile -->
        <div class="column is-6-mobile has-text-centered">
          <p class="heading">I2C TX</p>
          <p class="title is-5 has-text-black">{{ state.i2cTransactions.toLocaleString() }}</p>
        </div>

        <!-- I2C FAIL: Full width on mobile to emphasize errors -->
        <div class="column is-12-mobile has-text-centered">
          <p class="heading">I2C FAIL</p>
          <p class="title is-5" :class="state.i2cFailed > 0 ? 'has-text-danger' : 'has-text-grey-light'">
          {{ state.i2cFailed }}
          </p>
        </div>
      </div>

      <!-- GAUGES SECTION -->
      <div class="columns is-multiline is-variable is-2">
        <div class="column is-4-desktop is-12-mobile" v-for="type in ['ROLL', 'PITCH', 'YAW']" :key="type">
          <div class="card has-background-white has-text-centered shadow-card">
            <div class="card-content px-2 py-5">
              <p class="heading has-text-weight-bold has-text-black mb-4">{{ type }}</p>
              <component :is="getGauge(type)" :[type.toLowerCase()]="state[type.toLowerCase()]" />
              <p class="title is-2 has-text-black">{{ state[type.toLowerCase()].toFixed(type === 'YAW' ? 0 : 1) }}°</p>
            </div>
          </div>
        </div>
      </div>

      <!-- TELEMETRY CHART -->
      <div class="column is-12 mt-5 p-0">
        <div class="card has-background-white shadow-card">
          <div class="card-content">
            <p class="heading has-text-weight-bold has-text-black mb-4">REAL-TIME TELEMETRY</p>
            <div ref="chartRef" class="uplot-wrapper"></div>
          </div>
        </div>
      </div>

      <!-- 3D ATTITUDE VIEW -->
      <div class="column is-12 mt-5 p-0">
        <div class="card has-background-white shadow-card">
          <div class="card-content">
            <Attitude3D :roll="state.roll" :pitch="state.pitch" :yaw="state.yaw" />
          </div>
        </div>
      </div>

    </div>
  </section>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { useIMUStore } from './store/imuStore'
import uPlot from 'uplot'
import 'uplot/dist/uPlot.min.css'
import Attitude3D from './components/Attitude3D.vue'
import PitchGauge from './components/PitchGauge.vue'
import RollGauge from './components/RollGauge.vue'
import YawGauge from './components/YawGauge.vue'

const { state, updateIMU, updateSystemStats, setConnected } = useIMUStore()
const SIM_MODE = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'

const chartRef = ref(null)
let uplotInstance = null
let socket = null
let simTimer = null

const chartData = [Array.from({length: 1000}, (_, i) => i), Array(1000).fill(0), Array(1000).fill(0), Array(1000).fill(0)]

const getGauge = (type) => ({ ROLL: RollGauge, PITCH: PitchGauge, YAW: YawGauge }[type])

const handleIncomingData = (r, p, y) => {
  updateIMU(r, p, y)
  chartData[0].push(chartData[0][chartData[0].length - 1] + 1); chartData[0].shift()
  chartData[1].push(r); chartData[1].shift()
  chartData[2].push(p); chartData[2].shift()
  chartData[3].push(y); chartData[3].shift()
  if (uplotInstance) uplotInstance.setData(chartData)
}

const connectWebSocket = () => {
  socket = new WebSocket(`ws://${window.location.hostname}/ws_imu`)
  socket.binaryType = "arraybuffer"
  socket.onopen = () => setConnected(true)
  socket.onmessage = (event) => {
    const v = new DataView(event.data)
    const r = v.getFloat32(0, true), p = v.getFloat32(4, true), y = v.getFloat32(8, true)
    updateSystemStats(v.getInt32(12, true), v.getInt32(16, true), v.getBigUint64(20, true), v.getBigUint64(28, true))
    handleIncomingData(r, p, y)
  }
  socket.onclose = () => {
    setConnected(false); socket = null
    if (!SIM_MODE) setTimeout(connectWebSocket, 2000)
  }
}

onMounted(() => {
  uplotInstance = new uPlot({
    width: chartRef.value.offsetWidth, height: 250,
    series: [{}, { stroke: "#485fc7", label: "Roll" }, { stroke: "#ff3860", label: "Pitch" }, { stroke: "#ffdd57", label: "Yaw" }],
    axes: [{ grid: { stroke: "#f0f0f0" } }, { grid: { stroke: "#f0f0f0" }, values: (u, vals) => vals.map(v => v + "°") }],
    cursor: { show: false }
  }, chartData, chartRef.value)

  const resizeObserver = new ResizeObserver(entries => {
    for (let entry of entries) {
      // Get the current width of the parent container
      const newWidth = entry.contentRect.width;
      
      // Update uPlot size (Keep height fixed at 250 for readability)
      if (uplotInstance && newWidth > 0) {
        uplotInstance.setSize({ width: newWidth, height: 250 })
      }
    }
  })

  // Start watching the chart wrapper
  if (chartRef.value) {
    resizeObserver.observe(chartRef.value)
  }

  // Save observer for cleanup
  uplotInstance._observer = resizeObserver

  if (SIM_MODE) {
    setConnected(true)
    simTimer = setInterval(() => handleIncomingData(Math.sin(Date.now()/1000)*45, Math.cos(Date.now()/1000)*45, (Date.now()/100)%360), 20)
  } else connectWebSocket()
})

onUnmounted(() => {
  if (socket) socket.close()
  if (simTimer) clearInterval(simTimer)
  if (uplotInstance) {
    // Stop the observer before destroying the instance
    if (uplotInstance._observer) {
      uplotInstance._observer.disconnect()
    }
    uplotInstance.destroy()
  }
})
</script>

<style scoped>
.shadow-card { border-radius: 16px; box-shadow: 0 10px 25px rgba(0,0,0,0.1); border: 1px solid #efefef; }
.uplot-wrapper { width: 100%; display: flex; justify-content: center; background: #fff; }
:deep(.uplot) { margin: 0 auto; }

.status-indicator { display: inline-flex; align-items: center; justify-content: center; position: relative; }
.status-indicator .tag { min-width: 85px; transition: all 0.3s ease; position: relative; z-index: 2; }
.is-live .tag { background-color: #00d1b2 !important; color: white !important; animation: hardcore-pulse 2s infinite; }
.is-off .tag { background-color: #ff3860 !important; color: white !important; }

/* Force legend labels to pure black for maximum contrast */
:deep(.u-legend .u-label) {
  color: #000 !important;
  font-weight: 800 !important;
}

/* Make the values next to labels darker */
:deep(.u-legend .u-value) {
  color: #333 !important;
  font-weight: 600 !important;
}

/* Ensure axis labels on the canvas are crisp */
:deep(.u-axis .u-value) {
  color: #222 !important;
}

@keyframes hardcore-pulse {
  0% { box-shadow: 0 0 0 0 rgba(0, 209, 178, 0.7); }
  70% { box-shadow: 0 0 0 15px rgba(0, 209, 178, 0); }
  100% { box-shadow: 0 0 0 0 rgba(0, 209, 178, 0); }
}
</style>
