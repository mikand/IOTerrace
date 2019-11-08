import time
import math
import requests
from requests.utils import requote_uri

SERVER='http://localhost:5000'

PORTS=list(range(8))

t = 0
while True:
    v = math.sin(t/10)
    requests.get(requote_uri(SERVER+'/add_reading/1/'+str(v)))
    t += 1
    time.sleep(1)
