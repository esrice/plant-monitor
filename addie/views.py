import sqlite3

import flask

@app.route('/')
def index():
    return flask.render_template('index.html')


@app.route('/get_data'):
    """
    Get a complete table dump in json.
    """
    conn = sqlite3.connect('addie.db')
    c = conn.cursor()
    c.execute('PRAGMA table_info(dht_readings)')
    colnames = [r[1] for r in c.fetchall()]
    c.execute('SELECT * FROM dht_readings')
    rows = c.fetchall()
    return flask.jsonify(
        [{colnames[i]: x for i, x in enumerate(row)} for row in rows])

