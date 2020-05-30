function getDataAndMakeCharts() {
    fetch('get_data')
        .then(response => response.json())
        .then(response => makeCharts(response));
}

function makeCharts(jsonData) {
	makeTemperatureChart(jsonData);
	makeHumidityChart(jsonData);
	makeMoistureChart(jsonData);
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
                borderColor: 'rgba(255, 0, 0, 1)',
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
				borderColor: 'rgba(0, 255, 0, 1)',
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
				borderColor: 'rgba(0, 0, 255, 1)',
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
