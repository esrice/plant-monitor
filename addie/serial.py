import spidev

commands = {
    'checksum': 1,
    'temp_int': 2,
    'temp_dec': 3,
    'hum_int': 4,
    'hum_dec': 5,
    'moisture_high': 6,
    'moisture_low': 7,
    'light_high': 8,
    'light_low': 9,
}


class BadChecksumError(Exception):
    pass


class Monitor:
    def __init__(self, bus=0, device=0, speed=5000):
        self.spi = spidev.SpiDev()
        self.spi.open(bus, device)
        self.spi.max_speed_hz = speed

    def read_all(self):
        """
        Read everything from the monitor there is to read. For now,
        that's just temperature and humidity. Check the checksum to
        make sure the transmission is good, and raise a
        BadChecksumError if that check fails.

        Returns:
            temp_c: temperature in degrees C
            humidity: relative humidity in %
            moisture: moisture in % of full conductivity (water alone
                is ~90%, for reference)
            light: light intensity, out of 1024
        """
        # send a series of commands and receive the results. Add an
        # arbitrary command at the end because the result for a command
        # comes with the next transfer
        results = self.spi.xfer([
            commands['checksum'],
            commands['temp_int'],
            commands['temp_dec'],
            commands['hum_int'],
            commands['hum_dec'],
            commands['moisture_high'],
            commands['moisture_low'],
            commands['light_high'],
            commands['light_low'],
            0
        ])

        checksum = results[1]
        temp_int, temp_dec = results[2:4]
        hum_int, hum_dec = results[4:6]
        moisture_high, moisture_low = results[6:8]
        light_high, light_low = results[8:10]

        if checksum != ((temp_int + temp_dec + hum_int + hum_dec) & 0xFF):
            raise(BadChecksumError())

        temp_c = (((temp_int & 0x7F) << 8) | temp_dec) * 0.1
        if temp_int & 0x80:
            temp *= -1
        humidity = ((hum_int << 8) | hum_dec) * 0.1
        moisture = ((moisture_high << 8) | moisture_low) / 1024 * 100
        light = ((light_high << 8) | light_low)

        return temp_c, humidity, moisture, light

