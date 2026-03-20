<template>
  <div class="mag-cloud-wrapper" @mousedown="startDrag">
    <canvas ref="glCanvas" class="gl-canvas"></canvas>
    <div class="points-count is-family-monospace">{{ count }} PTS</div>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import * as twgl from 'twgl.js'

const glCanvas = ref(null)
const count = ref(0)
const MAX_POINTS = 1500
const points = new Float32Array(MAX_POINTS * 3)

let gl, programInfo, bufferInfo, axisBufferInfo 
let lon = 45, lat = 35, isDragging = false, lastX = 0, lastY = 0

const vs = `
  attribute vec3 position;
  attribute vec4 color;
  uniform mat4 u_matrix;
  varying vec4 v_color;
  void main() {
    gl_Position = u_matrix * vec4(position, 1.0);
    gl_PointSize = 3.0;
    v_color = color;
  }
`;

const fs = `
  precision mediump float;
  varying vec4 v_color;
  void main() { gl_FragColor = v_color; }
`;

const render = () => {
  if (!gl || !axisBufferInfo || !programInfo) return;
  twgl.resizeCanvasToDisplaySize(gl.canvas);
  gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  const aspect = gl.canvas.width / gl.canvas.height;
  const projection = twgl.m4.perspective(35 * Math.PI / 180, aspect, 0.1, 100);

  // Spherical Coordinates for the Camera Eye
  const p = lon * Math.PI / 180;
  const t = (90 - lat) * Math.PI / 180;
  const r = 6.0;
  const eye = [r * Math.sin(t) * Math.cos(p), r * Math.cos(t), r * Math.sin(t) * Math.sin(p)];
  
  const camera = twgl.m4.lookAt(eye, [0, 0, 0], [0, 1, 0]);
  const viewProj = twgl.m4.multiply(projection, twgl.m4.inverse(camera));

  gl.useProgram(programInfo.program);
  twgl.setUniforms(programInfo, { u_matrix: viewProj });

  // Draw RGB Axes
  twgl.setBuffersAndAttributes(gl, programInfo, axisBufferInfo);
  gl.drawArrays(gl.LINES, 0, 6); 

  // Draw Point Cloud
  twgl.setBuffersAndAttributes(gl, programInfo, bufferInfo);
  const cLoc = programInfo.attribSetters.color.location;
  gl.disableVertexAttribArray(cLoc); 
  gl.vertexAttrib4f(cLoc, 1.0, 0.86, 0.34, 1.0); // Mag Yellow
  gl.drawArrays(gl.POINTS, 0, count.value);
  gl.enableVertexAttribArray(cLoc);
}

const startDrag = (e) => { isDragging = true; lastX = e.clientX; lastY = e.clientY; }
const doDrag = (e) => {
  if (!isDragging) return;
  lon -= (e.clientX - lastX) * 0.4;
  lat = Math.max(-85, Math.min(85, lat + (e.clientY - lastY) * 0.4));
  lastX = e.clientX; lastY = e.clientY;
  render();
}
const stopDrag = () => { isDragging = false; }

onMounted(() => {
  gl = glCanvas.value.getContext("webgl");
  programInfo = twgl.createProgramInfo(gl, [vs, fs]);
  const s = 2.0;
  axisBufferInfo = twgl.createBufferInfoFromArrays(gl, {
    position: [0,0,0, s,0,0, 0,0,0, 0,s,0, 0,0,0, 0,0,s],
    color: [1,0,0,1, 1,0,0,1, 0,1,0,1, 0,1,0,1, 0,0,1,1, 0,0,1,1]
  });
  bufferInfo = twgl.createBufferInfoFromArrays(gl, {
    position: { numComponents: 3, data: points, drawType: gl.DYNAMIC_DRAW }
  });
  window.addEventListener('mousemove', doDrag);
  window.addEventListener('mouseup', stopDrag);
  render();
})

onUnmounted(() => {
  window.removeEventListener('mousemove', doDrag);
  window.removeEventListener('mouseup', stopDrag);
})

defineExpose({ 
    addPoint: (x, y, z) => {
    // 1. If we are full, shift everything left by 3 floats (1 point)
    if (count.value >= MAX_POINTS) {
      // Move points from index 3 to the end, into index 0
      points.set(points.subarray(3));
      // The last point is now at (MAX_POINTS - 1) * 3
      const i = (MAX_POINTS - 1) * 3;
      points[i] = x / 50; points[i + 1] = y / 50; points[i + 2] = z / 50;
    } else {
      // 2. Still filling up, just append at the end
      const i = count.value * 3;
      points[i] = x / 50; points[i + 1] = y / 50; points[i + 2] = z / 50;
      count.value++;
    }

    // 3. Push the whole window to the GPU
    twgl.setAttribInfoBufferFromArray(gl, bufferInfo.attribs.position, points);
    render();
  },
  reset: () => { count.value = 0; points.fill(0); render(); }
})
</script>

<style scoped>
.mag-cloud-wrapper { background: #080808; border-radius: 8px; position: relative; aspect-ratio: 1/1; cursor: grab; overflow: hidden; }
.mag-cloud-wrapper:active { cursor: grabbing; }
.gl-canvas { width: 100%; height: 100%; display: block; }
.points-count { position: absolute; top: 4px; right: 8px; font-size: 0.6rem; color: #00d1b2; background: rgba(0,0,0,0.5); padding: 2px 4px; border-radius: 4px; pointer-events: none; }
</style>
