from time import sleep

import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

# The callback for when a PUBLISH message is received from the server.


def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("192.168.1.100", 1883, 60)
client.loop_start()
sleep(0.1)
command = '0'
watch_topic='/values/laser/report'
command_topic='/control/laser/cmd'
while True:
    sleep(0.1)
    command = input('input command\n')
    #client.publish('/control/car/do', command)
    #client.publish('/values/laser/report', command)
    client.subscribe(watch_topic)
    client.publish(command_topic, command, qos=1)

