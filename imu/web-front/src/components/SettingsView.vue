<template>
  <div class="settings-view px-2">
    <!-- PERFORMANCE CONTROL CARD -->
    <div class="column is-12 p-0">
      <div class="card has-background-white shadow-card">
        <div class="card-content px-5 py-5">
          <p class="heading has-text-weight-bold has-text-black mb-5">PERFORMANCE & THROTTLING</p>
          
          <div class="columns is-variable is-8 is-vcentered">
            <!-- Packet Drop -->
            <div class="column is-6">
              <div class="is-flex is-justify-content-between mb-2">
                <span class="is-size-7 has-text-grey-light uppercase tracking-wide">Packet Drop</span>
                <span class="is-family-monospace is-size-5 has-text-weight-bold has-text-link">
                  {{ packetDrop }}<span class="is-size-7">ms</span>
                </span>
              </div>
              <input class="slider is-fullwidth is-link m-0" 
                     type="range" min="0" max="500" step="10" 
                     v-model.number="packetDrop">
            </div>

            <!-- Display Sync -->
            <div class="column is-6">
              <div class="is-flex is-justify-content-between mb-2">
                <span class="is-size-7 has-text-grey-light uppercase tracking-wide">Display Sync</span>
                <span class="is-family-monospace is-size-5 has-text-weight-bold has-text-link">
                  {{ displaySync }}<span class="is-size-7">ms</span>
                </span>
              </div>
              <input class="slider is-fullwidth is-link m-0" 
                     type="range" min="0" max="1000" step="50" 
                     v-model.number="displaySync">
            </div>
          </div>

          <div class="is-flex is-justify-content-center mt-5">
            <button class="button is-ghost is-small has-text-grey" @click="imuStore.setDefaults()">
              RESTORE ENGINE DEFAULTS
            </button>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useIMUStore } from '../store/imuStore'

const imuStore = useIMUStore()

// These computed properties handle the read/write to the readonly store
const packetDrop = computed({
  get: () => imuStore.state.packetDropMs,
  set: (val) => imuStore.setPacketDrop(val)
})

const displaySync = computed({
  get: () => imuStore.state.displayUpdateMs,
  set: (val) => imuStore.setDisplayRate(val)
})
</script>

<style scoped>
.shadow-card {
  box-shadow: 0 2px 15px rgba(0,0,0,0.05);
  border-radius: 8px;
}
.slider {
  cursor: pointer;
  height: 8px;
}
.is-family-monospace {
  font-family: 'Courier New', Courier, monospace;
}
.uppercase {
  text-transform: uppercase;
}
.tracking-wide {
  letter-spacing: 0.1em;
}
.border-right {
  border-right: 1px solid #f0f0f0;
}
</style>
