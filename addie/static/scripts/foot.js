function getDataAndMakeCharts() {
    fetch('get_data')
        .then(response => response.json())
        .then(response => makeCharts(response));
}

function makeCharts(jsonData) {
	makeTemperatureChart(jsonData);
	makeHumidityChart(jsonData);
	makeMoistureChart(jsonData);
    makeLightChart(jsonData);
}

function makeTemperatureChart(jsonData) {
    var temperatureChart = new Chart(document.getElementById('temperatureChart'), {
    	type: 'line',
    	data: {
            //labels: ['temperature'],
            datasets: [{
                label: 'temperature',
                data: jsonData.map(measurement => ({
					t: datetimeToDate(measurement.time),
					y: measurement.temperature
				})),
                borderColor: 'rgba(102, 194, 165, 1)',
                backgroundColor: 'rgba(0, 0, 0, 0)'
            }]
        },
    	options: {
			maintainAspectRatio: false,
            scales: {
                xAxes: [{
                    type: 'time'
                }]
            }
    	}
    });
}

function makeHumidityChart(jsonData) {
	var humidityChart = new Chart(document.getElementById('humidityChart'), {
		type: 'line',
		data: {
			datasets: [{
				label: 'humidity',
				data: jsonData.map(measurement => ({
					t: datetimeToDate(measurement.time),
					y: measurement.humidity
				})),
				borderColor: 'rgba(252, 141, 98, 1)',
				backgroundColor: 'rgba(0, 0, 0, 0)'
			}]
		},
		options: {
			maintainAspectRatio: false,
            scales: {
                xAxes: [{
                    type: 'time'
                }]
            }
		}
	});
}

function makeMoistureChart(jsonData) {
	var moistureChart = new Chart(document.getElementById('moistureChart'), {
		type: 'line',
		data: {
			datasets: [{
				label: 'moisture',
				data: jsonData.map(measurement => ({
					t: datetimeToDate(measurement.time),
					y: measurement.moisture
				})),
				borderColor: 'rgba(141, 160, 203, 1)',
				backgroundColor: 'rgba(0, 0, 0, 0)'
			}]
		},
		options: {
			maintainAspectRatio: false,
            scales: {
                xAxes: [{
                    type: 'time'
                }]
            }
		}
	});
}

function makeLightChart(jsonData) {
	var lightChart = new Chart(document.getElementById('lightChart'), {
		type: 'line',
		data: {
			datasets: [{
				label: 'light',
				data: jsonData.map(measurement => ({
					t: datetimeToDate(measurement.time),
					y: measurement.light
				})),
				borderColor: 'rgba(231, 138, 195, 1)',
				backgroundColor: 'rgba(0, 0, 0, 0)'
			}]
		},
		options: {
			maintainAspectRatio: false,
            scales: {
                xAxes: [{
                    type: 'time'
                }]
            }
		}
	});
}

getDataAndMakeCharts();
