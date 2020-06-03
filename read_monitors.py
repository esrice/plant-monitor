#!/usr/bin/env python3
"""
Read all current conditions from monitors, output them to stdout, and
write it all to the database.
"""

import argparse
import datetime
import sqlite3
import sys
import time

import addie.serial as serial


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-d', '--db-file', default='addie.db',
                        help='sqlite3 database file')
    parser.add_argument('-t', '--tries', type=int, default=3,
                        help='# of read attempts to make before giving up')
    parser.add_argument('-s', '--seconds-between-tries', type=int, default=10,
                        help='seconds to pause between tries')
    return parser.parse_args()


def main():
    args = parse_args()
    print('Readings monitors at {}.'.format(datetime.datetime.now()))

    monitor = serial.Monitor()
    tries, reading = 0, None
    while tries < args.tries and reading is None:
        try:
            reading = monitor.read_all()
        except serial.BadChecksumError:
            print('Got bad checksum on attempt #{}'.format(tries + 1),
                  file=sys.stderr)
            tries += 1
            time.sleep(args.seconds_between_tries)

    if reading is None:
        print('Failed to read sensors after {} tries. Goodbye.'.format(tries))
        sys.exit(1)

    temperature, humidity, moisture, light = reading
    print('Successfully read DHT22.')
    print('Temperature: {:.1f}°C'.format(temperature))
    print('Relative humidity: {:.1f}%'.format(humidity))
    print('Soil moisture: {:.1f}'.format(moisture))
    print('Light intensity: {}'.format(light))
    print('Goodbye!')

    conn = sqlite3.connect(args.db_file)
    c = conn.cursor()
    c.execute('INSERT INTO dht_readings '
              '(temperature, humidity, moisture, light) '
              'VALUES (?, ?, ?, ?)', (temperature, humidity, moisture, light))
    conn.commit()
    conn.close()


if __name__ == '__main__':
    main()

