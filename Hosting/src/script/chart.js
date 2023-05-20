import { firebaseConfig } from "./firebase.js";
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.0/firebase-app.js";
import {
  getDatabase,
  get,
  ref,
  child
}
from "https://www.gstatic.com/firebasejs/9.22.0/firebase-database.js"

const app = initializeApp(firebaseConfig);
const db = getDatabase();
const dbref = ref(db);

const humidityCheckBox = document.querySelector('#humidity-check-box')
const tempCheckBox = document.querySelector('#temp-check-box')
const btnradio1 = document.querySelector("#btnradio1")
const btnradio2 = document.querySelector("#btnradio2")
const btnradio3 = document.querySelector("#btnradio3")
const rangeInput = document.querySelector('#range-input')
const numberOfData = document.querySelector('#number-of-data')

const twentyFourHoursAgo = Date.now() - 24 * 60 * 60 * 1000;
const sevenDaysAgo = Date.now() - 7 * 24 * 60 * 60 * 1000;
const tenminutesAgo = Date.now() - 10 * 60 * 1000;

let validateTime = twentyFourHoursAgo;
let numData = 17;
let myChart;
let dataToday = [];
let extractedLabel = [];
let extractedHumidData = [];
let extractedTempData = [];

rangeInput.addEventListener("input", function () {
  numberOfData.innerHTML = rangeInput.value;
})


rangeInput.addEventListener("change", async () => {
  numData = rangeInput.value;
  await reset();
})

btnradio1.addEventListener('click', async () => {
  validateTime = sevenDaysAgo;
  await reset();
})

btnradio2.addEventListener('click', async () => {
  validateTime = twentyFourHoursAgo;
  await reset();
})

btnradio3.addEventListener('click', async () => {
  validateTime = tenminutesAgo;
  await reset();
})

humidityCheckBox.addEventListener('change', async () => {
  await reset();
})

tempCheckBox.addEventListener('change', async () => {
  await reset();
})

async function reset() {
  myChart.destroy();
  dataToday = [];
  extractedLabel = [];
  extractedHumidData = [];
  extractedTempData = [];
  await createGraph();
}

async function findAllDataToday() {
  await get(child(dbref, "data/"))
    .then((snapshot) => {
      snapshot.forEach(element => {
        if (element.val().Ts > validateTime) {
          dataToday.push(element.val())
        }
      });
    })
    .catch((error) => {
      alert(error)
    })
  return dataToday
}

async function createGraph() {
  await findAllDataToday();

  const label = dataToday.map(obj => obj.Ts)
  const humidData = dataToday.map(obj => obj.humid)
  const tempData = dataToday.map(obj => obj.temp)
  const spacing = Math.floor(label.length / numData);

  for (let i = 0; extractedLabel.length < numData; i += spacing) {
    const date = new Date(label[i]);
    let options = {
      timeZone: 'Asia/Bangkok',
      hour12: false,
      hour: 'numeric',
      minute: 'numeric'
    };

    if (validateTime === sevenDaysAgo) {
      options = {
        timeZone: 'Asia/Bangkok',
        hour12: false,
        month: 'numeric',
        day: 'numeric'
      }
    }

    const formattedTime = date.toLocaleString('en-EN', options);

    extractedLabel.push(formattedTime)
    extractedHumidData.push(humidData[i])
    extractedTempData.push(tempData[i])
  }

  let scales = {};

  if (!tempCheckBox.checked) {
    extractedTempData = []
    scales = {
      yAxes: [{
        ticks: {
          suggestedMin: 50
        }
      }]
    }
  }

  if (!humidityCheckBox.checked) {
    extractedHumidData = []
    scales = {
      yAxes: [{
        ticks: {
          suggestedMin: 15,
          suggestedMax: 50
        }
      }]
    }
  }

  myChart = new Chart(document.getElementById("line-chart"), {
    type: 'line',
    data: {
      labels: extractedLabel,
      datasets: [{
        data: extractedHumidData,
        label: "Humidity",
        borderColor: "#3e95cd",
        fill: false
      }, {
        data: extractedTempData,
        label: "Temperature",
        borderColor: "#8e5ea2",
        fill: false
      }]
    },
    options: {
      scales: scales
    }
  });
}

await createGraph();