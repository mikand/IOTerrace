import datetime

from flask import Flask, render_template, jsonify, request, redirect, url_for

import data_model as dm


app = Flask(__name__)

session = dm.Session()

@app.route('/')
def index():
    all_sensors = session.query(dm.Sensor).all()
    sensors = [s for s in all_sensors if s.enabled]
    return render_template('index.html',
                           all_sensors=all_sensors,
                           sensors=sensors,
                           active_sensors=', '.join(str(s.port) for s in sensors),
                           num_sensors=len(sensors))


@app.route('/get_readings/<int:sensor_port>')
@app.route('/get_readings/<int:sensor_port>/<float:since>')
@app.route('/get_readings/<int:sensor_port>/<int:since>')
def get_readings(sensor_port, since=0):
    res = {}
    sensor = session.query(dm.Sensor).filter_by(port=sensor_port).first()
    if sensor is None:
        res['status'] = 'No such sensor'
    else:
        res['status'] = 'OK'
        res['port'] = sensor.port
        res['data'] = [{"time" : x.time.timestamp(),
                        "value" : x.value}
                       for x in sensor.readings if x.time.timestamp() > since]
        res['data'] = res['data'][-48:]
    return jsonify(res)


@app.route('/add_reading/<int:sensor_port>/<string:value>')
def add_reading(sensor_port, value):
    sensor = session.query(dm.Sensor).filter_by(port=sensor_port).first()
    sensor.readings.append(dm.Reading(time=datetime.datetime.now(), value=float(value)))
    session.add(sensor)
    session.commit()
    return jsonify({'status': 'OK'})


@app.route('/set_options', methods=['GET', 'POST'])
def set_options():
    all_sensors = session.query(dm.Sensor).all()
    for s in all_sensors:
        nk = 'optval_%s' % s.id
        ek = 'enabled_%s' % s.id
        name = request.form[nk]
        enabled = ek in request.form and request.form[ek] == 'enabled'
        if name:
            s.name = name
        s.enabled = enabled
        session.add(s)
    session.commit()
    return redirect(url_for('index'))





if __name__ == '__main__':
    # import os
    # import logging
    # import logging.handlers
    # import gzip

    # class GZipRotator:
    #     def __call__(self, source, dest):
    #         os.rename(source, dest)
    #         f_in = open(dest, 'rb')
    #         f_out = gzip.open("%s.gz" % dest, 'wb')
    #         f_out.writelines(f_in)
    #         f_out.close()
    #         f_in.close()
    #         os.remove(dest)

    # wd = os.path.dirname(os.path.realpath(__file__))
    # log_filename = os.path.join(wd, 'logs', 'epb_gui.log')

    # logformatter = logging.Formatter('%(asctime)s;%(levelname)s; %(message)s')
    # log = logging.handlers.TimedRotatingFileHandler(log_filename, 'midnight', 1, backupCount=10)
    # log.setLevel(logging.DEBUG)
    # log.setFormatter(logformatter)
    # log.rotator = GZipRotator()

    # logger = logging.getLogger('werkzeug')
    # logger.addHandler(log)
    # logger.setLevel(logging.DEBUG)

    app.run(debug=True, host='0.0.0.0', port=5000)
