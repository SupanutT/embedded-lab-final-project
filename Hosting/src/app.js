const express = require('express')
const ejsMate = require('ejs-mate')
const path = require('path')
const functions = require('firebase-functions')

const app = express();

app.engine('ejs', ejsMate)
app.set('view engine', 'ejs')
app.set('views', path.join(__dirname, 'views'))
app.use(express.urlencoded({
    extended: true
}))

app.use('/script', express.static(path.join(__dirname, 'script')))

app.get('/home', (req, res) => {
    res.render('home')
})

app.get('/dashboard', (req, res) => {
    res.render('dashboard')
})

app.get('/graph', (req, res) => {
    res.render('graph')
})

app.listen(3000, () => {
    console.log("LISTENING ON PORT 3000")
})

exports.api = functions.https.onRequest(app)
