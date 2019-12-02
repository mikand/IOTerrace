import os.path

from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy import Column, Integer, DateTime, String, ForeignKey, Boolean
from sqlalchemy import create_engine, Float
from sqlalchemy.orm import relationship, sessionmaker

wd = os.path.dirname(os.path.realpath(__file__))
dbpath = os.path.join(wd, 'database.db')
engine = create_engine('sqlite:///' + dbpath)

Base = declarative_base()


class Sensor(Base):
    __tablename__ = 'sensors'

    id = Column(Integer, primary_key=True)
    name = Column(String(255), nullable=False)
    port = Column(Integer, nullable=False)
    kind = Column(String(255), nullable=False)
    enabled = Column(Boolean, nullable=True)


class Reading(Base):
    __tablename__ = 'readings'

    id = Column(Integer, primary_key=True)
    sensor_id = Column(Integer, ForeignKey('sensors.id'))
    sensor = relationship("Sensor", back_populates="readings")
    time = Column(DateTime, nullable=False)
    value = Column(Float, nullable=False)

Sensor.readings = relationship('Reading',
                               back_populates='sensor',
                               order_by=Reading.time)


_created = False
if not os.path.exists(dbpath):
    Base.metadata.create_all(engine)
    _created = True


Session = sessionmaker()
Session.configure(bind=engine)

if _created:
    _session = Session()
    for i in range(8):
        _session.add(Sensor(name='Sensor %s' % i, port=i, kind='humidity'))
    _session.add(Sensor(name='Temperature', port=8, kind='temperature'))
    _session.add(Sensor(name='Air Humidity', port=9, kind='humidity'))
    _session.commit()
