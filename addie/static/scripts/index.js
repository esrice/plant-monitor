function showTemperature() {
    fetch('get_data')
        .then(response => response.json())
        .then(function(json) {
            let mostRecentJson = json.slice(-1)[0];
            setTemperatureString(mostRecentJson);
        });
}

function setTemperatureString(json) {
    let temperatureP = document.getElementById('temperature');
    timeOfLastReading = Date(json.time + ' UTC')
    temperatureP.innerHTML = 'Temperature was ' + json.temperature.toFixed(1)
        + '°C and humidity was ' + json.humidity.toFixed(1) + '% at '
        + timeOfLastReading + '.';
}

