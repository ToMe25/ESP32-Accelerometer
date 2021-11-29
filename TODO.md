## Note
Some of these things may never get implemented.
And even if they do, there is no guarantee that these things are the next things I will implement.

## TODO
 * Switch sensor connection to hardware SPI?(if it is using software SPI atm)
 * Split sensor reading and data storage to different Tasks?(might not be worth it)
 * Add client with live graphs(Java standalone or Javascript in the web interface)
 * Generate all download files once and cache their size to show it on the downloads page
 * Automatically scale up sensor ranges if the read value is close to the current max
 * Show the actual measurement rate on the download page
 * Try using Stream for web response to send downloads in a single transfer?(might not be possible, requires cached file size)
 * Add SD data storage(might make extra task for data storage more worth it)
