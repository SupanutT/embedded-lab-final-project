const { initializeApp } = require("firebase/app");
const {
    getDatabase,
    get,
    ref,
    child
} = require("firebase/database");
const firebaseConfig = {
     /* your firebaseConfig */
};
const firebaseApp = initializeApp(firebaseConfig);
const db = getDatabase();
const dbref = ref(db);

const functions = require('firebase-functions')
const line = require('@line/bot-sdk')
const axios = require('axios').default
const accessToken = '' // your line access token
const secretToken = '' //your line secret token
const express = require('express')
const app = express();

const lineConfig = {
    channelAccessToken: accessToken,
    channelSecret: secretToken
}

const client = new line.Client(lineConfig)

app.post('/webhook', line.middleware(lineConfig), async (req, res) => {
    try {
        const events = req.body.events;
        console.log("event=>>>>", events)
        return events.length > 0 ? await events.map(item => handleEvent(item)) : res.status(200).send("OK")
    } catch (err) {
        res.status(500).end()
    }
})

const handleEvent = async (event) => {
    console.log(event)
    let data = 'ไม่พบข้อมูล'
    if (event.message.text === 'data') {
        await get(child(dbref, "sensors/"))
            .then((snapshot) => {
                const options = {
                    timeZone: 'Asia/Bangkok',
                    hour12: false,
                    month: 'long',
                    day: 'numeric',
                    hour: 'numeric',
                    minute: 'numeric'
                }
                const date = new Date(snapshot.val().timestamp).toLocaleString('en-GB', options);
                data = `ความแรงพัดลม: ${snapshot.val().fan}\nความชื้น: ${snapshot.val().humid}%\nอุณหภูมิ: ${snapshot.val().temp} องศา\nบันทึกล่าสุด: ${date}`
            })
    } else if (event.message.text === 'web') {
        data = 'https://nodemcu-f263e.web.app/'
    } else if (event.message.text === 'help') {
        data = 'พิมพ์ "data" เพื่อเรียกดูข้อมูลล่าสุด\nพิมพ์ "web" เพื่อเรียกดูลิ้งค์เว็ปไซต์งับ'
    } else {
        data = 'พิมพ์ "data" เพื่อเรียกดูข้อมูลล่าสุด\nพิมพ์ "web" เพื่อเรียกดูลิ้งค์เว็ปไซต์งับ'
        const response = await axios.get(`https://openapi.botnoi.ai/service-api/botnoichitchat?keyword=${event.message.text}&styleid=22&botname=น้องนอร์ท`, {
                headers: {
                    'Authorization': 'Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2ODQ5MTEwMjIsImlkIjoiNjQ3NDk3MzMtMWJhNi00Y2RkLTliN2QtMWFkYmNlZjE0NzAwIiwiaXNzIjoiRlpqdlI4MmFrbWJ4Vzk1alg4V1BWZGltVFAzUFBxTVAiLCJuYW1lIjoiOlAiLCJwaWMiOiJodHRwczovL3Byb2ZpbGUubGluZS1zY2RuLm5ldC8waDVsUDc1WnlaYWtOdFRuNk9ZbUlWRkZFTFpDNGFZR3dMRlhvaUxVOUhNSHRJS241Q1YzOHRKa3hQTTNzVGV5NFNXSHgzTEVGT1lTTkcifQ.SSSvf8WOTnt6VQ2I-V2G_7AEexd12ucED7q0i8_uDoA'
                }
            }).then(response => data = response.data.reply)
            .catch(err => console.log(err))
    }
    return client.replyMessage(event.replyToken, {
        type: 'text',
        text: data
    })
}

app.listen(3000, () => {
    console.log("LISTENING ON PORT 3000!")
})

exports.app = functions.https.onRequest(app)