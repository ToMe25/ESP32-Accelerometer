var interval
var measurements
var recorded
var recordings
var recordingsVal
var eta
var time

window.onload = init

function init() {
	interval = window.setInterval(update, 500)

	measurements = document.getElementById('measurements')
	recorded = document.getElementById('recorded')
	recordings = document.getElementById('recordings')
	eta = document.getElementById('eta')
	time = document.getElementById('time')

	recordingsVal = Number(recordings.innerText)

	update()
}

function update() {
	fetch('measurements.json', { method: 'get' })
		.then((res) => {
			return res.json();
		})
		.then((out) => {
			measurements.value = recorded.innerText = out.measurements
			time.innerText = format_time(out.time)
			var recordTime = out.time / out.measurements
			eta.innerText = format_time(recordTime * (recordingsVal - out.measurements))

			if (out.measurements == recordingsVal) {
				location = location // reload website. location.reload asks the user to resend the form.
			}
		})
		.catch((err) => {
			throw err
		})
}

function format_time(time_ms) {
	var result = ""
	var printed = 0
	var h = Math.floor(time_ms / 3600000);
	var m
	var s
	var ms

	if (h == 0) {
		m = Math.floor(time_ms / 60000) % 60;
	} else {
		result += h + "h "
		m = Math.round(time_ms / 60000) % 60
		printed++
	}

	if (m == 0) {
		s = Math.floor(time_ms / 1000) % 60
	} else {
		result += m + "m "
		s = Math.round(time_ms / 1000) % 60
		printed++
	}

	if (s != 0 && printed < 2) {
		result += s + "s "
		printed++
	}

	ms = Math.round(time_ms) % 1000

	if (ms != 0 && printed < 2) {
		result += ms + "ms "
	}

	return result.trim()
}
