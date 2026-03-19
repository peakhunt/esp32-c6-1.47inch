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
    imu.value.yaw = (imu.value.yaw - 2) % 360;

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
.title.is-2 { font-size: 2.5rem !important; font-weight: 900; color: #000 !important; }
.uplot-wrapper { width: 100%; display: flex; justify-content: center; background-color: #ffffff; }
:deep(.uplot) { margin: 0 auto; }
</style>
