const BROKER_URL = 'wss://d8f5ab7e7e184546bcc2d7bae591bab1.s1.eu.hivemq.cloud:8884/mqtt';

const OPTIONS = {
  clientId: 'web-' + Math.random().toString(16).slice(2),
  username: 'stm32',
  password: 'Virajstm32',
  clean: true,
  connectTimeout: 4000
};

const TOPIC = 'led/control';


const client = mqtt.connect(BROKER_URL, OPTIONS);

client.on('connect', () => {
  console.log('Connected to HiveMQ Cloud');
});

client.on('error', (err) => {
  console.error('MQTT error', err);
});

const slider = document.getElementById("slider");
const valueEl = document.getElementById("value");

// ---------- helpers ----------
function updateSliderUI(value) {
  valueEl.textContent = value;

  const percent = (value / slider.max) * 100;
  slider.style.background =
    `linear-gradient(to right, #38bdf8 ${percent}%, #334155 ${percent}%)`;
}

function publishBrightness(value) {
  const payload = {
    cmd: "led",
    level: Number(value)
  };

  client.publish(
    TOPIC,
    JSON.stringify(payload)
  );
}


slider.addEventListener("input", (e) => {
  const value = Number(e.target.value);
  updateSliderUI(value);
  publishBrightness(value);
});

updateSliderUI(slider.value);
