function datetimeToDate(dtString) {
	var t = dtString.split(/[- :]/);
	t[1] -= 1;
	return new Date(Date.UTC(...t));
}

function showTemperature() {
    fetch('get_data')
        .then(response => response.json())
        .then(function(json) {
            let mostRecentJson = json.slice(-1)[0];
            setTemperatureString(mostRecentJson);
        });
}

function setTemperatureString(reading) {
    let temperatureP = document.getElementById('temperature');
    let timeOfLastReading = datetimeToDate(reading.time);
    temperatureP.innerHTML = 'Temperature was ' + reading.temperature.toFixed(1)
        + '°C, humidity was ' + reading.humidity.toFixed(1) + '%, and soil '
        + 'moisture was ' + reading.moisture.toFixed(1) + ' at '
        + timeOfLastReading + '.';
}

