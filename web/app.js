const BROKER_URL = 'wss://d8f5ab7e7e184546bcc2d7bae591bab1.s1.eu.hivemq.cloud:8884/mqtt';

const OPTIONS = {
  clientId: 'web-' + Math.random().toString(16).slice(2),
  username: 'stm32',
  password: 'Virajstm32',
  clean: true,
  connectTimeout: 4000
};

const TOPIC = 'led/control';

let ledState = 0; // assume device starts OFF

const client = mqtt.connect(BROKER_URL, OPTIONS);

client.on('connect', () => {
  console.log('Connected to HiveMQ Cloud');
  updateButton();
});

client.on('error', (err) => {
  console.error('MQTT error', err);
});

const button = document.getElementById('toggleBtn');

function updateButton() {
  button.textContent = ledState === 0 ? 'Turn ON' : 'Turn OFF';
}

button.addEventListener('click', () => {
  // toggle desired state
  ledState = ledState === 0 ? 1 : 0;

  const payload = JSON.stringify({
    cmd: 'led',
    state: ledState
  });

  client.publish(TOPIC, payload, { qos: 0 });

  updateButton();
});
