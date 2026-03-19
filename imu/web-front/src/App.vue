<template>
  <section class="section has-background-light" style="min-height: 100vh;">
    <div class="container">
      
      <div class="mb-6">
        <h1 class="title has-text-centered is-size-3 has-text-black has-text-weight-black">IMU Status</h1>
      </div>

      <div class="columns is-mobile is-variable is-2">
        
        <!-- ROLL COLUMN -->
        <div class="column is-4">
          <div class="card has-background-white has-text-centered shadow-card">
            <div class="card-content px-2 py-5">
              <p class="heading has-text-weight-bold has-text-black mb-4">ROLL</p>
              <RollGauge :roll="imu.roll" />
              <p class="title is-2 has-text-black">{{ imu.roll.toFixed(1) }}°</p>
            </div>
          </div>
        </div>

        <!-- PITCH COLUMN (Sky/Ground ADI) -->
        <div class="column is-4">
          <div class="card has-background-white has-text-centered shadow-card">
            <div class="card-content px-2 py-5">
              <p class="heading has-text-weight-bold has-text-black mb-4">PITCH</p>
              <PitchGauge :pitch="imu.pitch" />
              <p class="title is-2 has-text-black">{{ imu.pitch.toFixed(1) }}°</p>
            </div>
          </div>
        </div>

        <!-- YAW COLUMN (Compass) -->
        <div class="column is-4">
          <div class="card has-background-white has-text-centered shadow-card">
            <div class="card-content px-2 py-5">
              <p class="heading has-text-weight-bold has-text-black mb-4">YAW</p>
              <YawGauge :yaw="imu.yaw" />
              <p class="title is-2 has-text-black">{{ imu.yaw.toFixed(0) }}°</p>
            </div>
          </div>
        </div>
      </div>

      <!-- Real-time Chart in White Card -->
      <div class="column is-12 mt-5 p-0">
        <div class="card has-background-white shadow-card">
          <div class="card-content">
            <p class="heading has-text-weight-bold has-text-black mb-4">REAL-TIME TELEMETRY</p>
            <div ref="chartRef" class="uplot-wrapper"></div>
          </div>
        </div>
      </div>

      <div class="column is-12 mt-5 p-0">
        <div class="card has-background-white shadow-card">
          <div class="card-content">
            <Attitude3D :roll="imu.roll" :pitch="imu.pitch" :yaw="imu.yaw" />
          </div>
        </div>
      </div>


    </div>
  </section>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import uPlot from 'uplot'
import 'uplot/dist/uPlot.min.css'
import Attitude3D from './components/Attitude3D.vue'
import PitchGauge from './components/PitchGauge.vue'
import RollGauge from './components/RollGauge.vue'
import YawGauge from './components/YawGauge.vue'

// --- HARDCORE CONFIG ---
// Set this to true to simulate without the ESP32-C6
const SIM_MODE = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1';

const imu = ref({ roll: 0.0, pitch: 0.0, yaw: 0.0 })
const chartRef = ref(null)
let uplotInstance = null
let socket = null
let simTimer = null

const maxChartData = 1000
const chartData = [
  Array.from({length: maxChartData}, (_, i) => i),
  Array(maxChartData).fill(0),
  Array(maxChartData).fill(0),
  Array(maxChartData).fill(0)
]

// Update logic shared by both WS and SIM
const updateIMU = (r, p, y) => {
  imu.value.roll = r
  imu.value.pitch = p
  imu.value.yaw = y

  chartData[0].push(chartData[0][chartData[0].length - 1] + 1); chartData[0].shift()
  chartData[1].push(r); chartData[1].shift()
  chartData[2].push(p); chartData[2].shift()
  chartData[3].push(y); chartData[3].shift()

  if (uplotInstance) uplotInstance.setData(chartData)
}

const startSimulation = () => {
  console.log("🚀 Running in LOCAL SIMULATION mode");
  simTimer = setInterval(() => {
    const t = Date.now() / 1000;
    const r = Math.sin(t * 1.2) * 90;  // Roll ±90
    const p = Math.cos(t * 0.8) * 90;  // Pitch ±90
    const y = (t * 10) % 360;          // Yaw 0-360
    updateIMU(r, p, y);
  }, 20); // 50Hz
}

const connectWebSocket = () => {
  socket = new WebSocket(`ws://${window.location.hostname}/ws_imu`)
  socket.binaryType = "arraybuffer"

  socket.onmessage = (event) => {
    const view = new Float32Array(event.data)
    updateIMU(view[0], view[1], view[2])
  }

  socket.onclose = () => {
    console.log("C6 Disconnected. Retrying...");
    socket = null;
    if (!SIM_MODE) setTimeout(connectWebSocket, 2000)
  }
}

onMounted(() => {
  const opts = {
    width: chartRef.value.offsetWidth,
    height: 250,
    series: [{}, 
      { stroke: "#485fc7", width: 2, label: "Roll" },
      { stroke: "#ff3860", width: 2, label: "Pitch" },
      { stroke: "#ffdd57", width: 2, label: "Yaw" }
    ],
    axes: [
      { grid: { stroke: "#f0f0f0" } },
      { grid: { stroke: "#f0f0f0" }, values: (u, vals) => vals.map(v => v + "°") }
    ],
    cursor: { show: false }
  };

  uplotInstance = new uPlot(opts, chartData, chartRef.value);

  if (SIM_MODE) {
    startSimulation();
  } else {
    connectWebSocket();
  }
});

onUnmounted(() => {
  if (socket) socket.close();
  if (simTimer) clearInterval(simTimer);
  if (uplotInstance) uplotInstance.destroy();
});
</script>

<style scoped>
.shadow-card { border-radius: 16px; box-shadow: 0 10px 25px rgba(0,0,0,0.1); border: 1px solid #efefef; }
.title.is-2 { font-size: 2.5rem !important; font-weight: 900; color: #000 !important; }
.uplot-wrapper { width: 100%; display: flex; justify-content: center; background-color: #ffffff; }
:deep(.uplot) { margin: 0 auto; }
</style>
