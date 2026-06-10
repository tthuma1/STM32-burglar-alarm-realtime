# Webserver — Claude Context

This is the webserver component of the VGRS burglar alarm system. It is a standalone Python/Flask application that lives in the `webserver/` subdirectory of the main STM32 firmware repository.

## What This Does

Receives HTTP POST requests from the STM32H750B-DK IoT device when alarm state changes occur, persists them to SQLite, and serves an HTML event log at `GET /`.

## File Structure

```
webserver/
├── app.py              # Flask application — all routes and DB logic
├── requirements.txt    # Flask dependency
├── events.db           # SQLite database (created at runtime, not in git)
└── templates/
    └── index.html      # Jinja2 template for the event log page
```

## Architecture

- **`app.py`** — single-file Flask app. Database connection is managed per-request via Flask's `g` object (`get_db()` / `close_db()`). `init_db()` creates the `events` table on startup if it doesn't exist.
- **`templates/index.html`** — renders the `events` list passed from the `index()` route. Each row is a `sqlite3.Row` with `event_type` and `received_at` fields. CSS class names intentionally match event type strings for automatic color coding.
- **`events.db`** — SQLite file, single table `events(id, event_type, received_at)`. `received_at` is stored as an ISO-8601 UTC string (`YYYY-MM-DD HH:MM:SS`).

## Event Types

The valid values for `event_type` (enforced in `VALID_EVENTS` set in `app.py`):
- `alarm_on` — alarm armed via PIN + RFID
- `alarm_off` — alarm disarmed
- `motion_detected` — PIR sensor triggered on GPIO PG3
- `alarm_triggered` — TIM6 countdown expired, alarm fired

## IoT Device Context

The STM32 firmware (in `../Core/`) uses LwIP over Ethernet. The device has static IP `192.168.1.10`; the server PC is expected at `192.168.1.1`. The firmware sends POST requests with a JSON body `{"type": "<event_type>"}` at state transition points in `main.c` and `stm32h7xx_it.c`.

## Running

```bash
pip install -r requirements.txt
python app.py          # listens on 0.0.0.0:5000
```
