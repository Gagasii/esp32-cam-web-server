// Get current sensor readings when the page loads and initialize websocket
window.addEventListener("load", getReadings);

//ws.addEventListener("load", );
// Create Temperature Gauge
var tempGauge = new LinearGauge({
  renderTo: "TempReading",
  width: 120,
  height: 400,
  units: "Temperature C",
  minValue: 0,
  startAngle: 90,
  ticksAngle: 180,
  maxValue: 60,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
    "0",
    "5",
    "10",
    "15",
    "20",
    "25",
    "30",
    "35",
    "40",
    "45",
    "50",
    "55",
    "60",
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    {
      from: 45,
      to: 60,
      color: "rgba(200, 50, 50, .75)",
    },
  ],
  colorPlate: "#fff",
  colorBarProgress: "#CC2936",
  colorBarProgressEnd: "#049faa",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
}).draw();

// Create Humidity Gauge
var humGauge = new RadialGauge({
  renderTo: "HumReading",
  width: 300,
  height: 300,
  units: "Humidity (%)",
  minValue: 0,
  maxValue: 100,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 2,
  majorTicks: ["0", "20", "40", "60", "80", "100"],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
    {
      from: 80,
      to: 100,
      color: "#03C0C1",
    },
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
}).draw();

//create voltage gauge
var VGauge = new RadialGauge({
  renderTo: "VReading",
  width: 300,
  height: 300,
  units: " Volatage(V)",
  minValue: 0,
  maxValue: 12,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 2,
  majorTicks: ["0", "2", "4", "6", "8", "10", "12"],
  minorTicks: 1,
  strokeTicks: true,
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
}).draw();

function getReadings() {
  var xhttpr = new XMLHttpRequest();
  xhttpr.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var myObj = JSON.parse(this.responseText);
      console.log(myObj);
      var temp = myObj.temperature;
      var hum = myObj.humidity;
      var volt = myObj.voltage;

      tempGauge.value = temp;
      humGauge.value = hum;
      VGauge.value = volt;
    }
  };
  xhttpr.open("GET", "/readings", true);
  xhttpr.send();
}

//initialize event source protocol and handle events
if (!!window.EventSource) {
  var source = new EventSource("/events");
  //add event listener
  source.addEventListener(
    "open",
    function (e) {
      console.log("Events Connected");
    },
    false
  );
  //handle error and log it
  source.addEventListener(
    "error",
    function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    },
    false
  );
  //log message of the event
  source.addEventListener(
    "message",
    function (e) {
      console.log("message", e.data);
    },
    false
  );
  //get new readings and log them
  source.addEventListener(
    "new_readings",
    function (e) {
      console.log("new_readings", e.data);
      var myObj = JSON.parse(e.data);
      console.log(myObj);
      tempGauge.value = myObj.temperature;
      humGauge.value = myObj.humidity;
      VGauge.value = myObj.voltage;
    },
    false
  );
}
/************/

//Graph for humidity
var humidityChart = new Highcharts.Chart({
  chart: { renderTo: "humChart" },
  title: { text: "Humidity" },
  series: [
    {
      showInLegend: false,
      data: [],
    },
  ],
  plotOptions: {
    line: { animation: false, dataLabels: { enabled: true } },
    series: { color: "#059e8a" },
  },

  xAxis: {
    type: "datetime",
    dateTimeLabelFormats: { second: "%H:%M:%S" },
  },
  yAxis: {
    title: { text: "Humidity (%)" },
  },
  credits: { enabled: false },
});

//plot humidity data
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var x = new Date().getTime(),
        y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if (humidityChart.series[0].data.length > 40) {
        humidityChart.series[0].addPoint([x, y], true, true, true);
      } else {
        humidityChart.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 30000);

//Graph for Temperature
var temperatureChart = new Highcharts.Chart({
  chart: { renderTo: "tempChart" },
  title: { text: "Temperature" },
  series: [
    {
      showInLegend: false,
      data: [],
    },
  ],
  plotOptions: {
    line: { animation: false, dataLabels: { enabled: true } },
    series: { color: "#059e8a" },
  },
  xAxis: { type: "datetime", dateTimeLabelFormats: { second: "%H:%M:%S" } },
  yAxis: {
    title: { text: "Temperature (Celsius)" },
  },
  credits: { enabled: false },
});
//plot temperature
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var x = new Date().getTime(),
        y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if (temperatureChart.series[0].data.length > 40) {
        temperatureChart.series[0].addPoint([x, y], true, true, true);
      } else {
        temperatureChart.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 30000);
//Graph for voltage
var voltageChart = new Highcharts.Chart({
  chart: { renderTo: "voltChart" },
  title: { text: "Voltage" },
  series: [
    {
      showInLegend: false,
      data: [],
    },
  ],
  plotOptions: {
    line: { animation: false, dataLabels: { enabled: true } },
    series: { color: "#059e8a" },
  },
  xAxis: { type: "datetime", dateTimeLabelFormats: { second: "%H:%M:%S" } },
  yAxis: {
    title: { text: "Voltage (V)" },
  },
  credits: { enabled: false },
});
//plot voltage
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status == 200) {
      var x = new Date().getTime(),
        y = parseFloat(this.responseText);
      //console.log(this.responseText);
      if (voltageChart.series[0].data.length > 40) {
        voltageChart.series[0].addPoint([x, y], true, true, true);
      } else {
        voltageChart.series[0].addPoint([x, y], true, false, true);
      }
    }
  };
  xhttp.open("GET", "/voltage", true);
  xhttp.send();
}, 30000);

var deg = -1;
function capturePhoto() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/capture", true);
  xhr.send();
}
function rotatePhoto() {
  var img = document.getElementById("photo");
  deg += 89;
  if (isOdd(deg / 89)) {
    document.getElementById("container").className = "vert";
  } else {
    document.getElementById("container").className = "hori";
  }
  img.style.transform = "rotate(" + deg + "deg)";
}
function isOdd(n) {
  return Math.abs(n % 1) == 1;
}
