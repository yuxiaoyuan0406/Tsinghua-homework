import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("/control/car/sudo")
    subscribeList=(
        '/values/car/#', 
        '/control/car/command', 
        '/control/car/dat', 
        '/capability/car/values', 
        '/capibility/car/control', 
        '/control/line/go', 
        '/values/laser/#', 
        '/control/laser/#', 
        '/control/mechanicalarm/#', 
        '/control/mechanicalarm/create', 
        '/values/mechanicalarm/report', 
        '/control/robotarm/#', 
        '/control/robotarm/move', 
        '/control/robotarm/command', 
        '/values/robotarm/report', 
        '/control/warehouse/dat', 
        '/control/warehouse/command', 
        '/control/warehouse/#', 
        '/values/warehouse/#', 
        '/values/warehouse/report'
    )
    for topic in subscribeList:
        client.subscribe(topic, 1)
    '''
    client.subscribe("/control/car/sudo")

    client.subscribe('/control/line/go', 1)

    client.subscribe('/values/laser/report', 1)
    client.subscribe('/value/laser/#', 1)

    client.subscribe('/control/mechanicalarm/create', 1)
    client.subscribe('/values/mechanicalarm/report', 1)

    client.subscribe('/control/robotarm/move', 1)
    client.subscribe('/control/robotarm/command', 1)
    client.subscribe('/values/robotarm/report', 1)


    client.subscribe('/control/Warehouse/open', 1)
    client.subscribe('/values/Warehouse/report', 1)
    '''

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    #print(msg.payload)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.12.1", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
