from time import sleep

import paho.mqtt.client as mqtt

command_topic='/values/laser/files'
watch_topic='/values/laser/#'

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

# The callback for when a PUBLISH message is received from the server.


def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

shit = [
    b'ok.txt',
    b'shit.html',
    b'hello.cpp',
    b'main'
]

def send_shit(client):
    client.publish(command_topic, b'_upload')
    i=0
    for item in shit:
        client.publish(command_topic, b'%d - '%(i) + item)
        i += 1
        sleep(0.1)
    client.publish(command_topic, b'_end')

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect("192.168.12.1", 1883, 60)
client.loop_start()
sleep(0.1)
command = '0'
client.subscribe(watch_topic)
# client.publish(command_topic, b'end', qos=1)
send_shit(client)
while True:
    sleep(0.1)
    command = input('input command\n')
    #client.publish('/control/car/do', command)
    #client.publish('/values/laser/report', command)
    if command == 1:
        command = b'upload'
    elif command == 0:
        command = b'end'
    
    client.publish(command_topic, command, qos=1)

