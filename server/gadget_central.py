#!/usr/bin/env python3

import bottle
import argparse
import os

app = bottle.Bottle()
script_dir = os.path.dirname(__file__)


@app.route('/')
@bottle.view('index')
def index():
    return {}


@app.route('/update/<device>')
def update(device):
    """Get latest firmware for the device"""
    return "Unknown device: " + device
    #return bottle.static_file(filename, root='/path/to/your/static/files')


@app.route('/control/<device>')
def control(device):
    """Get commands for the device"""
    bottle.response.content_type = 'text/plain; charset=UTF-8'
    bottle.response.set_header('X-Device', device)

    try:
        with open(script_dir + '/commands/' + device + '/seq') as f:
            seq = f.readline().strip()
    except:
        bottle.abort(404, "Device not known: " + device)
    try:
        with open(script_dir + '/commands/' + device + '/' + seq) as f:
            commands = f.read()
    except:
        bottle.abort(404, "No commands for device: " + device)

    # Sequence number, this is incremented with each new command set
    # The device reads current Seq on startup, then checks for a change.
    # Whenever the number changes, the device executes the commands.
    bottle.response.set_header('X-Seq', seq)
    return commands


@app.route('/control/<device>', method='DELETE')
def control(device):
    """Acknowledge processing of commands"""
    seq = bottle.request.query.seq
    try:
        os.unlink(script_dir + '/commands/' + device + '/' + seq)
    except OSError:
        bottle.abort(404, "No commands found.")
    return 'ACK'


@app.route('/write', method='POST')
def write():
    """Forward sensor data to InfluxDB"""
    db = bottle.request.forms.get('db')
    return "Not implemented."


@app.error(404)
def error404(error):
    return error.body


if __name__ == '__main__':
    ap = argparse.ArgumentParser()
    ap.add_argument('--debug', action='store_true', help='debug mode')
    ap.add_argument('--reload', action='store_true', help='auto-reload')
    args = ap.parse_args()
    bottle.debug(args.debug)
    bottle.run(app, host='0.0.0.0', port=8086, reloader=args.reload)
