// Function to send a message to the Pebble
function sendMessage(dictionary) {
  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log('Message sent to Pebble successfully!');
    },
    function(e) {
      console.log('Error sending message to Pebble!');
    }
  );
}

// --- Original Weather Fetching Logic ---
// (This will be used when you have a real device)
var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  // Construct URL
  var url = 'https://api.openweathermap.org/data/2.5/weather?lat=' +
      pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=YOUR_API_KEY'; // Replace with your key

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      var json = JSON.parse(responseText);
      var temperature = Math.round(json.main.temp - 273.15); // Kelvin to Celsius
      var conditions = json.weather[0].main;      
      
      var dictionary = {
        'KEY_TEMPERATURE': temperature + '°',
        'KEY_CONDITIONS': conditions
      };
      sendMessage(dictionary);
    }      
  );
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

// --- Emulator Simulation ---
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');

    // Immediately send fake data for the emulator
    var dictionary = {
      'KEY_TEMPERATURE': '72°',
      'KEY_CONDITIONS': 'Sunny'
    };
    
    // CRASH FIX: Use the correct function 'sendAppMessage'
    sendMessage(dictionary);
  }
);

// Listen for when an AppMessage is received from the C side
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received from watch, fetching real weather...');
    // When you have a real device, this will trigger a real weather update
    // getWeather(); 
  }                     
);