import "bulma/css/bulma.min.css"
import { createApp } from "vue";
import { Icon } from '@iconify/vue'
import App from "./App.vue";

const app = createApp(App);

// Register the Icon component globally so you can use <Icon icon="mdi:gauge" /> anywhere
app.component('Icon', Icon);

app.mount("#app");
