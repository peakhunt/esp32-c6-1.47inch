// src/composables/useDevice.js
import { ref, onMounted } from 'vue'

export function useDevice() {
  const isMobile = ref(false)

  onMounted(() => {
    isMobile.value = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) || 
                     (window.innerWidth <= 768 && window.matchMedia("(pointer: coarse)").matches)
  })

  return { isMobile }
}
