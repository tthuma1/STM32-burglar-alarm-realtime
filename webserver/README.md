# VGRS Burglar Alarm — Webserver

Receives state change events from the STM32H750B-DK burglar alarm IoT device and displays them in a live event log.

## Requirements

- Python 3.8+

## Setup

- Set your IP to 192.168.1.1.

```bash
pip install -r requirements.txt
```

## Running

```bash
python app.py
```

The server listens on `0.0.0.0:5000`. Open [http://localhost:5000](http://localhost:5000) in a browser to view the event log.

## API

### POST /event

Receive a state change event from the IoT device.

**Request body (JSON):**
```json
{ "type": "alarm_on" }
```

**Valid event types:**
| Type | Description |
|---|---|
| `alarm_on` | Alarm was armed (PIN + RFID authenticated) |
| `alarm_off` | Alarm was disarmed |
| `motion_detected` | PIR sensor triggered while alarm armed |
| `alarm_triggered` | 10-second countdown expired, alarm fired |

**Responses:** `200 OK` on success, `400` on unknown or missing type.

### GET /

HTML page listing all recorded events with timestamps, newest first.

## Testing

### Simulation script

Sends a realistic sequence of events (arm → motion → trigger → disarm → arm → disarm):

```bash
python test_events.py
```

Optional flags:

```bash
python test_events.py --host 192.168.1.1 --port 5000 --delay 1.0
```

### Manual curl

```bash
curl -X POST http://localhost:5000/event \
  -H "Content-Type: application/json" \
  -d '{"type":"alarm_on"}'

curl -X POST http://localhost:5000/event \
  -H "Content-Type: application/json" \
  -d '{"type":"motion_detected"}'

curl http://localhost:5000/
```

## Storage

Events are stored in `events.db` (SQLite), created automatically on first run in the working directory.
