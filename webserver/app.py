import sqlite3
from datetime import datetime
from flask import Flask, request, g, render_template, abort

DATABASE = 'events.db'
VALID_EVENTS = {'alarm_on', 'alarm_off', 'motion_detected', 'alarm_triggered'}

app = Flask(__name__)


def get_db():
    db = getattr(g, '_database', None)
    if db is None:
        db = g._database = sqlite3.connect(DATABASE)
        db.row_factory = sqlite3.Row
    return db


@app.teardown_appcontext
def close_db(exception):
    db = getattr(g, '_database', None)
    if db is not None:
        db.close()


def init_db():
    with app.app_context():
        get_db().execute('''
            CREATE TABLE IF NOT EXISTS events (
                id          INTEGER PRIMARY KEY AUTOINCREMENT,
                event_type  TEXT    NOT NULL,
                received_at TEXT    NOT NULL
            )
        ''')
        get_db().commit()


@app.route('/event', methods=['POST'])
def receive_event():
    data = request.get_json(silent=True) or {}
    event_type = data.get('type', '').strip()
    if event_type not in VALID_EVENTS:
        abort(400)
    db = get_db()
    db.execute(
        'INSERT INTO events (event_type, received_at) VALUES (?, ?)',
        (event_type, datetime.now().astimezone().strftime('%Y-%m-%d %H:%M:%S'))
    )
    db.commit()
    return 'OK', 200


@app.route('/')
def index():
    rows = get_db().execute(
        'SELECT event_type, received_at FROM events ORDER BY id DESC'
    ).fetchall()
    return render_template('index.html', events=rows)


if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000, debug=False)
