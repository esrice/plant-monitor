import sqlite3

import flask

from addie import app


def get_config():
    with app.open_resource('config.json', 'r') as config_file:
        return flask.json.load(config_file)


@app.route('/')
def index():
    return flask.render_template('index.html')


@app.route('/get_data')
def get_data():
    """
    Get a complete table dump in json.
    """
    config = get_config()
    conn = sqlite3.connect(config['db'])
    c = conn.cursor()
    c.execute('PRAGMA table_info(dht_readings)')
    colnames = [r[1] for r in c.fetchall()]
    c.execute('SELECT * FROM dht_readings')
    rows = c.fetchall()
    return flask.jsonify(
        [{colnames[i]: x for i, x in enumerate(row)} for row in rows])

