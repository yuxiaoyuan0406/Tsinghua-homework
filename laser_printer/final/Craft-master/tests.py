import paho.mqtt.client as mqtt
import paho.mqtt.subscribe as subscribe

# def on_robo_actionok(client, userdata, message):
#     print("%s %s" % (message.topic, message.payload))

# subscribe.callback(on_robo_actionok, "/values/robo/actionok", hostname="192.168.12.1")

# def on_robo_actionok(client, userdata, message):
#     print("%s %s" % (message.topic, message.payload))

# subscribe.callback(on_robo_actionok, "/values/robo/actionok", hostname="192.168.12.1")


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("/values/+/actionok")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    
    print(msg.topic+" "+str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.12.1", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_start()
client.publish("/control/robo/do", "0")

input("hhh")