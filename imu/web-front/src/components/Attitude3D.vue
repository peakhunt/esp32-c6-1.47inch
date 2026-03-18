<template>
  <div class="card has-background-white shadow-card mt-5">
    <div class="card-content p-2">
      <div class="canvas-wrapper">
        <!-- HUD Overlay: Top Left -->
        <div class="hud-overlay">
          <div class="hud-line"><span class="hud-label">R:</span> {{ props.roll.toFixed(1) }}°</div>
          <div class="hud-line"><span class="hud-label">P:</span> {{ props.pitch.toFixed(1) }}°</div>
          <div class="hud-line"><span class="hud-label">Y:</span> {{ props.yaw.toFixed(1) }}°</div>
        </div>

        <!-- DYNAMIC 3D LABELS -->
        <div class="label-3d n-label" :style="labelStyles.n">N</div>
        <div class="label-3d e-label" :style="labelStyles.e">E</div>
        <div class="label-3d d-label" :style="labelStyles.d">D</div>

        <p class="heading has-text-centered has-text-black has-text-weight-bold mb-2 pt-2">
          3D ATTITUDE (NED)
        </p>
        <canvas ref="glCanvas" class="attitude-canvas"></canvas>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, onMounted, reactive } from 'vue'
import * as twgl from 'twgl.js'

const props = defineProps(['roll', 'pitch', 'yaw'])
const glCanvas = ref(null)
const m4 = twgl.m4

const labelStyles = reactive({
  n: { transform: 'translate(-100px, -100px)' },
  e: { transform: 'translate(-100px, -100px)' },
  d: { transform: 'translate(-100px, -100px)' }
})

onMounted(() => {
  const gl = glCanvas.value.getContext("webgl")
  const programInfo = twgl.createProgramInfo(gl, [vs, fs])

  const cubeBufferInfo = twgl.primitives.createCubeBufferInfo(gl, 1)
  const lineBufferInfo = twgl.createBufferInfoFromArrays(gl, { position: [0,0,0, 1,0,0] })
  const coneBufferInfo = twgl.primitives.createTruncatedConeBufferInfo(gl, 0.15, 0, 0.4, 12, 1)

  function render() {
    twgl.resizeCanvasToDisplaySize(gl.canvas)
    gl.viewport(0, 0, gl.canvas.width, gl.canvas.height)
    gl.enable(gl.DEPTH_TEST)
    gl.clearColor(0.98, 0.98, 0.98, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)

    const projection = m4.perspective(45 * Math.PI / 180, gl.canvas.clientWidth / gl.canvas.clientHeight, 0.1, 50)
    
    // Standard WebGL Camera (Looking at origin from behind/above)
    const cameraPos = [0, 5, 15] 
    const target = [0, 0, 0]
    const up = [0, 1, 0] 
    const camera = m4.lookAt(cameraPos, target, up)
    const viewProj = m4.multiply(projection, m4.inverse(camera))

    // YOUR CORRECT ROTATION LOGIC
    let world = m4.identity()
    world = m4.axisRotate(world, [0, 1, 0], -props.yaw * Math.PI / 180)   // Yaw (Up)
    world = m4.axisRotate(world, [1, 0, 0], props.pitch * Math.PI / 180)  // Pitch (Right)
    world = m4.axisRotate(world, [0, 0, 1], -props.roll * Math.PI / 180)  // Roll (Depth)

    // Helper: Project 3D coordinate to 2D Screen pixel
    const projectLabel = (localPos) => {
      const tip3d = m4.transformPoint(world, localPos)
      const clipSpace = m4.transformPoint(viewProj, tip3d)
      const x = (clipSpace[0] * 0.5 + 0.5) * gl.canvas.clientWidth
      const y = (clipSpace[1] * -0.5 + 0.5) * gl.canvas.clientHeight
      return `translate(${x}px, ${y}px)`
    }

    labelStyles.n.transform = projectLabel([0, 0, -4.5]) // N (-Z)
    labelStyles.e.transform = projectLabel([3.5, 0, 0])  // E (+X)
    labelStyles.d.transform = projectLabel([0, -3.0, 0]) // D (-Y)

    gl.useProgram(programInfo.program)

    // 1. RECTANGLE BODY
    let rectMat = m4.multiply(viewProj, world)
    rectMat = m4.scale(rectMat, [2.0, 0.6, 3.5]) 
    twgl.setBuffersAndAttributes(gl, programInfo, cubeBufferInfo)
    twgl.setUniforms(programInfo, { u_matrix: rectMat, u_color: [0.1, 0.14, 0.49, 1.0], u_isBody: true })
    twgl.drawBufferInfo(gl, cubeBufferInfo)

    // 2. ARROWS
    const drawArrow = (direction, length, color) => {
      let mat = m4.multiply(viewProj, world)
      if (direction === 'N') mat = m4.axisRotate(mat, [0, 1, 0], Math.PI / 2) // Point toward -Z
      if (direction === 'D') mat = m4.axisRotate(mat, [0, 0, 1], -Math.PI / 2) // Point toward -Y
      
      let lMat = m4.scale(mat, [length, 1, 1])
      twgl.setBuffersAndAttributes(gl, programInfo, lineBufferInfo)
      twgl.setUniforms(programInfo, { u_matrix: lMat, u_color: color, u_isBody: false })
      twgl.drawBufferInfo(gl, lineBufferInfo, gl.LINES)

      let cMat = m4.translate(mat, [length, 0, 0])
      cMat = m4.axisRotate(cMat, [0, 0, 1], -Math.PI / 2)
      twgl.setBuffersAndAttributes(gl, programInfo, coneBufferInfo)
      twgl.setUniforms(programInfo, { u_matrix: cMat, u_color: color, u_isBody: false })
      twgl.drawBufferInfo(gl, coneBufferInfo)
    }

    drawArrow('N', 4.0, [1, 0, 0, 1])          // Red: Forward (North)
    drawArrow(null, 3.0, [0, 0, 1, 1])         // Blue: Right (East)
    drawArrow('D', 2.5, [0, 0.8, 0, 1])        // Green: Down (Down)
    
    requestAnimationFrame(render)
  }
  requestAnimationFrame(render)
})

const vs = `attribute vec4 position; uniform mat4 u_matrix; varying float v_y; void main() { gl_Position = u_matrix * position; v_y = position.y; }`;
const fs = `precision mediump float; uniform vec4 u_color; uniform bool u_isBody; varying float v_y; void main() { if (u_isBody) { float highlight = (v_y > 0.0) ? 0.15 : 0.0; gl_FragColor = vec4(u_color.rgb + highlight, u_color.a); } else { gl_FragColor = u_color; } }`;
</script>

<style scoped>
.canvas-wrapper { position: relative; width: 100%; height: 400px; background: #fdfdfd; border-radius: 12px; overflow: hidden; }
.hud-overlay { position: absolute; top: 15px; left: 15px; z-index: 10; background: rgba(255, 255, 255, 0.7); padding: 8px 12px; border-radius: 6px; font-family: monospace; border-left: 3px solid #485fc7; pointer-events: none; }
.label-3d { position: absolute; top: 0; left: 0; z-index: 5; font-weight: 900; font-size: 1.1rem; pointer-events: none; text-shadow: 1px 1px 1px white; }
.n-label { color: #ff0000; } .e-label { color: #0000ff; } .d-label { color: #008000; }
.attitude-canvas { width: 100%; height: 100%; display: block; }
.shadow-card { border-radius: 16px; box-shadow: 0 10px 25px rgba(0,0,0,0.1); border: 1px solid #efefef; }
</style>
