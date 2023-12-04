# python 3.5
# prueba de emqx con python - recepcion

import random
import datetime
import json
import time

from paho.mqtt import client as mqtt_client
from paho.mqtt.properties import Properties
from paho.mqtt.packettypes import PacketTypes

broker = '163.10.3.73'
port = 1883
topic = 'emqx/test'
client_id = 'netbook'
username = 'taller2g2'
password = 'taller2g2'

def on_connect(client,userdata,flags,rc):
	if rc == 0:
		print("Conectado al broker "+broker+":"+str(port))
	else:
		print("Conexion a "+broker+":"+str(port)+" fallo, codigo "+str(rc))
	client.subscribe("emqx/test")


def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

def run():
	client = mqtt_client.Client()

	client.on_connect = on_connect
	client.on_message = on_message

	client.username_pw_set(username,password)

	client.connect(broker,port,60)

	client.loop_forever()

if __name__ == '__main__':
	run()
