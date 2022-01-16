var interval
var progress
var calculated
var files
var filesVal
var eta
var time

window.onload = init

function init() {
	interval = window.setInterval(update, 500)

	progress = document.getElementById('progress')
	calculated = document.getElementById('calculated')
	files = document.getElementById('files')
	file = document.getElementById('file')
	time = document.getElementById('time')

	filesVal = Number(files.innerText)
}

function update() {
	fetch('calculations.json', { method: 'get' })
		.then((res) => {
			return res.json();
		})
		.then((out) => {
			progress.value = calculated.innerText = out.calculated
			time.innerText = format_time(out.time)
			file.innerText = out.file

			if (out.calculated == filesVal) {
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
