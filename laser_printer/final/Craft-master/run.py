
# coding: utf-8

# In[256]:


import paho.mqtt.client as mqtt
import sys


# In[253]:


fd = open("command.txt",'r')

# In[ ]:


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

x = 0

# In[ ]:


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    global x
    if x < 3:
        x = x + 1
        return
    print(msg.topic+" "+str(msg.payload))
    list_of_all_the_lines = fd.readline()
    if list_of_all_the_lines:
        sp = list_of_all_the_lines.split()
        print(sp[0], sp[1])
        client.publish(sp[0], sp[1])
    else:
        fd.close()
        sys.exit(0)


# In[14]:


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("192.168.12.1", 1883, 60)

# In[ ]:


# Subscribing in on_connect() means that if we lose the connection and
# reconnect then subscriptions will be renewed.
client.subscribe("/values/+/actionok")

# In[ ]:


client.loop_start()


# In[ ]:


client.publish("/values/car/actionok", '0')


# In[ ]:


try:  
    while 1:  
        pass  
except KeyboardInterrupt:  
    pass 

