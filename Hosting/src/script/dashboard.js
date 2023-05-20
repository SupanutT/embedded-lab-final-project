import { firebaseConfig } from "./firebase.js";
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.0/firebase-app.js";
import {
  getDatabase,
  get,
  ref,
  child
}
from "https://www.gstatic.com/firebasejs/9.22.0/firebase-database.js"

const spinningFanImg = document.querySelector('#spinning-fan-img')
const fanImg = document.querySelector('#fan-img')
const fanData = document.querySelector('#fan-data')
const humidData = document.querySelector('#humid-data')
const tempData = document.querySelector('#temp-data')
const humidHigh = document.querySelector('#humid-high')
const humidLow = document.querySelector('#humid-low')
const tempHigh = document.querySelector('#temp-high')
const tempLow = document.querySelector('#temp-low')
const latestUpdate = document.querySelector('#latest-update')
const records = [0, 999, 0, 999] //humidHigh, humidLow, tempHigh, tempLow
const app = initializeApp(firebaseConfig);
const db = getDatabase();
const dbref = ref(db);
const options = {
  timeZone: 'Asia/Bangkok',
  hour12: false,
  month: 'long',
  day: 'numeric',
  hour: 'numeric',
  minute: 'numeric'
}

async function initializeRecord() {
  const validateTime = Date.now() - 24 * 60 * 60 * 1000;

  await get(child(dbref, "data/"))
    .then((snapshot) => {
      snapshot.forEach(element => {
        if (element.val().Ts > validateTime) {
          if (element.val().humid > records[0]) {
            records[0] = element.val().humid
          }
          if (element.val().humid < records[1]) {
            records[1] = element.val().humid
          }
          if (element.val().temp > records[2]) {
            records[2] = element.val().temp
          }
          if (element.val().temp < records[3]) {
            records[3] = element.val().temp
          }
        }
      });
    })
    .catch((error) => {
      alert(error)
    })
}

async function findData() {
  await get(child(dbref, "sensors/"))
    .then((snapshot) => {
      if (snapshot.exists()) {
        fanData.innerHTML = snapshot.val().fan;
        humidData.innerHTML = snapshot.val().humid;
        tempData.innerHTML = snapshot.val().temp;
        latestUpdate.innerHTML = new Date(snapshot.val().timestamp).toLocaleString('en-GB', options);
        if (snapshot.val().humid > records[0]) {
          records[0] = snapshot.val().humid
          humidHigh.innerHTML = snapshot.val().humid
        }
        if (snapshot.val().humid < records[1]) {
          records[1] = snapshot.val().humid
          humidLow.innerHTML = snapshot.val().humid
        }
        if (snapshot.val().temp > records[2]) {
          records[2] = snapshot.val().temp
          tempHigh.innerHTML = snapshot.val().temp
        }
        if (snapshot.val().temp < records[3]) {
          records[3] = snapshot.val().temp
          tempLow.innerHTML = snapshot.val().temp
        }
        if (snapshot.val().fan) {
          fanImg.setAttribute('hidden', 'hidden')
          spinningFanImg.removeAttribute('hidden')
        } else {
          fanImg.removeAttribute('hidden')
          spinningFanImg.setAttribute('hidden', 'hidden')
        }
      } else {
        alert("No data found");
      }
    })
    .catch((error) => {
      alert(error)
    })
}

await findData()
await initializeRecord();

humidHigh.innerHTML = records[0]
humidLow.innerHTML = records[1]
tempHigh.innerHTML = records[2]
tempLow.innerHTML = records[3]

const mainIntervalId = setInterval(async () => {
  await findData()
}, 1000)