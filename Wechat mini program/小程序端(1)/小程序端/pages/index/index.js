import mqtt from'../../utils/mqtt.js';
const aliyunOpt = require('../../utils/aliyun/aliyun_connect.js');

let that = null;
Page({
  //初始数据
    data:{
    temperature1: "--",
    humidity1: "--",
    current1: "--",
    voltage1: "--",
    key1: "--",
    GPS1: "--",

    temperature2: "--",
    humidity2: "--",
    current2: "--",
    voltage2: "--",
    key2: "--",
    GPS2: "--",

    temperature3: "--",
    humidity3: "--",
    current3: "--",
    voltage3: "--",
    key3: "--",
    GPS3: "--",

    temperature4: "--",
    humidity4: "--",
    current4: "--",
    voltage4: "--",
    key4: "--",
    GPS4: "--",

   
    activeIndex: 0,

     client:null,//记录重连的次数
     reconnectCounts:0,//MQTT连接的配置
     options:{
        protocolVersion: 4, //MQTT连接协议版本
        clean: false,
        reconnectPeriod: 1000, //1000毫秒，两次重新连接之间的间隔
        connectTimeout: 30 * 1000, //1000毫秒，两次重新连接之间的间隔
        resubscribe: true, //如果连接断开并重新连接，则会再次自动订阅已订阅的主题（默认true）
        clientId: 'ie44FqhJNgx.weixin-device|securemode=2,signmethod=hmacsha256,timestamp=1680709115731|',
        password: '149443ffda56cba2732febb9cfa6fdce8c43da213f8ed4801bec34e9ae4efca4',
        username: 'weixin-device&ie44FqhJNgx',
      },

      aliyunInfo: {
        productKey: 'ie44FqhJNgx', //阿里云连接的三元组 
        deviceName: 'weixin-device', //阿里云连接的三元组 
        deviceSecret: '6a326feb8981ad972b53c6063eef51fe', //阿里云连接的三元组 
        regionId: 'cn-shanghai', //阿里云连接的三元组
        pubTopic: '/ie44FqhJNgx/weixin-device/user/topic', //发布消息的主题
        subTopic: '/ie44FqhJNgx/weixin-device/user/topic', //订阅消息的主题
      },
    },
  onLoad: function(options) {if (options.activeIndex) {
    this.setData({
      activeIndex: Number(options.activeIndex),
    });
  }},
  onLoad:function(){
    that = this;
    let clientOpt = aliyunOpt.getAliyunIotMqttClient({
      productKey: that.data.aliyunInfo.productKey,
      deviceName: that.data.aliyunInfo.deviceName,
      deviceSecret: that.data.aliyunInfo.deviceSecret,
      regionId: that.data.aliyunInfo.regionId,
      port: that.data.aliyunInfo.port,
    });

    console.log("get data:" + JSON.stringify(clientOpt));
    let host = 'wxs://' + clientOpt.host;
    
    this.setData({
      'options.clientId': clientOpt.clientId,
      'options.password': clientOpt.password,
      'options.username': clientOpt.username,
    })
    console.log("this.data.options host:" + host);
    console.log("this.data.options data:" + JSON.stringify(this.data.options));

    //访问服务器
    this.data.client = mqtt.connect(host, this.data.options);
    this.data.client.on('connect', function (connack) {
      wx.showToast({
        title: '连接成功'
      })
      console.log("连接成功");
    })

    //接收消息监听
    this.data.client.on("message", function (topic, payload) {
      console.log(" 收到 topic:" + topic + " , payload :" + payload);
      that.setData({
        //转换成JSON格式的数据进行读取
        temperature1:JSON.parse(payload).temperature1,
        humidity1:JSON.parse(payload).Humidity1,
        current1:JSON.parse(payload).RMSCurrent1,
        voltage1:JSON.parse(payload).RMSVoltage1,
        key1:JSON.parse(payload).NO1,
        GPS1:JSON.parse(payload).GeoLocation1,

        temperature2:JSON.parse(payload).temperature2,
        humidity2:JSON.parse(payload).Humidity2,
        current2:JSON.parse(payload).RMSCurrent2,
        voltage2:JSON.parse(payload).RMSVoltage2,
        key2:JSON.parse(payload).NO2,
        GPS2:JSON.parse(payload).GeoLocation2,

        temperature3:JSON.parse(payload).temperature3,
        humidity3:JSON.parse(payload).Humidity3,
        current3:JSON.parse(payload).RMSCurrent3,
        voltage3:JSON.parse(payload).RMSVoltage3,
        key3:JSON.parse(payload).NO3,
        GPS3:JSON.parse(payload).GeoLocation3,

        temperature4:JSON.parse(payload).temperature4,
        humidity4:JSON.parse(payload).Humidity4,
        current4:JSON.parse(payload).RMSCurrent4,
        voltage4:JSON.parse(payload).RMSVoltage4,
        key4:JSON.parse(payload).NO4,
        GPS4:JSON.parse(payload).GeoLocation4,
      })

/*       wx.showModal({
        content: " 收到topic:[" + topic + "], payload :[" + payload + "]",
        showCancel: false,
      }); */
    })

    //服务器连接异常的回调
    that.data.client.on("error", function (error) {
      console.log(" 服务器 error 的回调" + error)

    })
    //服务器重连连接异常的回调
    that.data.client.on("reconnect", function () {
      console.log(" 服务器 reconnect的回调")

    })
    //服务器连接异常的回调
    that.data.client.on("offline", function (errr) {
      console.log(" 服务器offline的回调")
    })
  },
  
  onClickOpen() {
    that.sendCommond('set', 1);
  },
  onClickOff() {
    that.sendCommond('set', 0);
  },
  sendCommond(cmd, data) {
    let sendData = {
      cmd: cmd,
      data: data,
    };



   this.data.client.subscribe(this.data.aliyunInfo.subTopic,function(err){
      if(!err){
        console.log("订阅成功");
      };
      wx.showModal({
        content: "订阅成功",
        showCancel: false,
      })
    })  
    

    //发布消息
    if (this.data.client && this.data.client.connected) {
      this.data.client.publish(this.data.aliyunInfo.pubTopic, JSON.stringify(sendData));
      console.log("************************")
      console.log(this.data.aliyunInfo.pubTopic)
      console.log(JSON.stringify(sendData))
    } else {
      wx.showToast({
        title: '请先连接服务器',
        icon: 'none',
        duration: 2000
      })
    }
  },
  //定义按钮的事件处理函数
  keyon1(){ 
    that.sendCommond('NO1', 1);
  },
  keyoff1(){ 
    that.sendCommond('NO1', 0);
  },
  keyon2(){ 
    that.sendCommond('NO2', 1);
  },
  keyoff2(){ 
    that.sendCommond('NO1', 0);
  },
  keyon3(){ 
    that.sendCommond('NO3', 1);
  },
  keyoff3(){ 
    that.sendCommond('NO1', 0);
  },
  keyon4(){ 
    that.sendCommond('NO4', 1);
  },
  keyoff4(){ 
    that.sendCommond('NO1', 0);
  },
  
  sendCommond(cmd, data) {
    let sendData = {
      cmd:cmd,
      data:data,
    };
    if (this.data.client && this.data.client.connected) {
      this.data.client.publish(this.data.aliyunInfo.pubTopic, JSON.stringify(sendData));

    } else {
      wx.showToast({
        title: '请先连接服务器',
        icon: 'none',
        duration: 2000
      })
    }
  },
  handleTabbarChange: function(event) {
    const { index } = event.currentTarget.dataset;
    this.setData({
      activeIndex: index,
    });
  },
  onShow: function() {
    const pages = getCurrentPages();
    const currentPage = pages[pages.length - 1];
    const activeIndex = currentPage.options.activeIndex || 0;
    this.setData({
      activeIndex: Number(activeIndex),
    });
  },
})