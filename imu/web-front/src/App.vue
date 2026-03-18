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
              <div class="visual-container mb-5">
                <svg viewBox="0 0 100 100" class="imu-svg">
                  <line x1="10" y1="50" x2="90" y2="50" stroke="#eee" stroke-width="2" stroke-dasharray="4" />
                  <circle cx="50" cy="50" r="48" fill="none" stroke="#f5f5f5" stroke-width="2" />
                  <g :style="{ transform: `rotate(${imu.roll}deg)`, transformOrigin: '50px 50px' }">
                    <line x1="15" y1="50" x2="85" y2="50" stroke="#485fc7" stroke-width="8" stroke-linecap="round" />
                    <path d="M 50 15 L 50 5" stroke="#485fc7" stroke-width="4" fill="none" />
                  </g>
                  <circle cx="50" cy="50" r="4" fill="#485fc7" />
                </svg>
              </div>
              <p class="title is-2 has-text-black">{{ imu.roll.toFixed(1) }}°</p>
            </div>
          </div>
        </div>

        <!-- PITCH COLUMN (Sky/Ground ADI) -->
        <div class="column is-4">
          <div class="card has-background-white has-text-centered shadow-card">
            <div class="card-content px-2 py-5">
              <p class="heading has-text-weight-bold has-text-black mb-4">PITCH</p>
              <div class="visual-container mb-5 horizon-box">
                <svg viewBox="0 0 100 100" class="imu-svg">
                  <g :style="{ transform: `translateY(${imu.pitch * 0.5}px)`, transition: 'transform 0.1s linear' }">
                    <rect x="-50" y="-100" width="200" height="150" fill="#87CEEB" />
                    <rect x="-50" y="50" width="200" height="150" fill="#8B4513" />
                    <line x1="0" y1="50" x2="100" y2="50" stroke="white" stroke-width="3" />
                  </g>
                  <path d="M 35 50 L 45 50 M 55 50 L 65 50 M 50 45 L 50 55" stroke="yellow" stroke-width="3" />
                </svg>
              </div>
              <p class="title is-2 has-text-black">{{ imu.pitch.toFixed(1) }}°</p>
            </div>
          </div>
        </div>

        <!-- YAW COLUMN (Compass) -->
        <div class="column is-4">
          <div class="card has-background-white has-text-centered shadow-card">
            <div class="card-content px-2 py-5">
              <p class="heading has-text-weight-bold has-text-black mb-4">YAW</p>
              <div class="visual-container mb-5">
                <svg viewBox="0 0 100 100" class="imu-svg">
                  <circle cx="50" cy="50" r="48" fill="none" stroke="#f5f5f5" stroke-width="2" />
                  <g :style="{ transform: `rotate(${-imu.yaw}deg)`, transformOrigin: '50px 50px' }">
                    <text x="50" y="20" text-anchor="middle" font-size="10" font-weight="bold">N</text>
                    <text x="80" y="53" text-anchor="middle" font-size="10">E</text>
                    <text x="50" y="87" text-anchor="middle" font-size="10">S</text>
                    <text x="20" y="53" text-anchor="middle" font-size="10">W</text>
                  </g>
                  <path d="M 50 10 L 45 25 L 55 25 Z" fill="red" />
                </svg>
              </div>
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

const imu = ref({ roll: 0.0, pitch: 0.0, yaw: 0.0 })
const chartRef = ref(null)
let uplotInstance = null
let timer = null

const data = [
  Array.from({length: 100}, (_, i) => i),
  Array(100).fill(0),
  Array(100).fill(0),
  Array(100).fill(0)
]

onMounted(() => {
  const opts = {
    width: chartRef.value.offsetWidth,
    height: 250,
    series: [
      {},
      { stroke: "#485fc7", width: 2, label: "Roll" },
      { stroke: "#ff3860", width: 2, label: "Pitch" },
      { stroke: "#ffdd57", width: 2, label: "Yaw" },
    ],
    axes: [
      { grid: { stroke: "#f0f0f0", width: 1 } },
      { 
        grid: { stroke: "#f0f0f0", width: 1 },
        values: (u, vals) => vals.map(v => v + "°") 
      }
    ],
    cursor: { show: false }
  };

  uplotInstance = new uPlot(opts, data, chartRef.value);

  timer = setInterval(() => {
    // 1-second simulation
    imu.value.roll = Math.sin(Date.now() / 3000) * 45;
    imu.value.pitch = Math.cos(Date.now() / 4000) * 45;
    imu.value.yaw = (imu.value.yaw + 2) % 360;

    data[0].push(data[0][data[0].length - 1] + 1); data[0].shift();
    data[1].push(imu.value.roll); data[1].shift();
    data[2].push(imu.value.pitch); data[2].shift();
    data[3].push(imu.value.yaw); data[3].shift();

    uplotInstance.setData(data);
  }, 100);
});

onUnmounted(() => {
  clearInterval(timer);
  if (uplotInstance) uplotInstance.destroy();
});
</script>

<style scoped>
.shadow-card { border-radius: 16px; box-shadow: 0 10px 25px rgba(0,0,0,0.1); border: 1px solid #efefef; }
.visual-container { width: 120px; height: 120px; margin: 0 auto; display: flex; align-items: center; justify-content: center; position: relative; }
.horizon-box { border-radius: 50%; overflow: hidden; border: 2px solid #333; }
.imu-svg { width: 100%; height: 100%; overflow: visible; }
.title.is-2 { font-size: 2.5rem !important; font-weight: 900; color: #000 !important; }
.uplot-wrapper { width: 100%; display: flex; justify-content: center; background-color: #ffffff; }
:deep(.uplot) { margin: 0 auto; }
</style>
