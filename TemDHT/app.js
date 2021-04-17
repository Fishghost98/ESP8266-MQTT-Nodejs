var mqtt = require('mqtt')
var mqttUrl = 'mqtt://39.105.123.55:61613'
const routes = require('./module/routes')

var option = {
  username: 'admin',
  password: 'password',
  clientId: 'NodejsClient',
}

var app = require('http').createServer(handler),
  io = require('socket.io')(app),
  fs = require('fs')

function handler(req, res) {
  routes.static(req, res, 'static')
  res.writeHead(200, { 'Content-Type': 'text/html;charset="utf-8"' })
  res.end()
}

var client = mqtt.connect(mqttUrl, option)
client.subscribe('Mymqtt/module1/willMessage', { qos: 2 })
client.subscribe('Mymqtt/module1/DHT11')
client.subscribe('Mymqtt/module1/light')
client.subscribe('Mymqtt/module2/willMessage', { qos: 2 })
client.subscribe('Mymqtt/module2/rgb', { qos: 2 })
console.log('订阅成功')

global.willMessage1 = '客户端离线'
global.willMessage2 = '客户端离线'
global.light = '**'
global.json = '**'

io.on('connection', function (socket) {
  client.on('message', function (topic, message) {
    if (topic == 'Mymqtt/module1/willMessage') {
      willMessage1 = message.toString()
      console.log('主题：' + topic + '  消息：' + willMessage1)
      socket.emit('module1', { msg: willMessage1 })
    } else if (topic == 'Mymqtt/module1/light') {
      light = message.toString()
      console.log('主题：' + topic + '  消息：' + light)
      socket.emit('light', { msg: light })
    } else if (topic == 'Mymqtt/module1/DHT11') {
      console.log('主题：' + topic + '  消息：' + message.toString())
      socket.emit('dht11', { msg: message.toString() })
    } else if (topic == 'Mymqtt/module2/willMessage') {
      willMessage2 = message.toString()
      console.log('主题：' + topic + '  消息：' + willMessage2)
      socket.emit('module2', { msg: willMessage2 })
    } else if (topic == 'Mymqtt/module2/rgb') {
      console.log('主题：' + topic + '  消息：' + message.toString())
      socket.emit('RGB', { msg: message.toString() })
    }
  })

  socket.on('rgb', function (data) {
    console.log(data)
    var rgbMessage = {}
    rgbMessage = data.toString()
    var rgbTopic = 'Mymqtt/module2/rgbControl'
    client.publish(rgbTopic, rgbMessage, { qos: 1, retain: false })
  })
})

//启动HTTP服务，绑定端口3000
app.listen(3000, function () {
  console.log('listening on *:3000')
})
