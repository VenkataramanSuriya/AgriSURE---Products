import paho.mqtt.client as mqtt
#import MySQLdb
import csv
import re
import pandas as pd
from sklearn.neighbors import KNeighborsClassifier

import time
import Adafruit_GPIO.SPI as SPI
import Adafruit_SSD1306
from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont
import subprocess

from datetime import date
#from datetime import time
from datetime import datetime

import RPi.GPIO as GPIO
import time

#GPIO.setmode(GPIO.BOARD)
Relay = 16
#GPIO.setup(Relay, GPIO.OUT)
#GPIO.output(Relay, False)

MQTT_SERVER = "192.168.168.209"
MQTT_PATH = "Soilmoisture"
MQTT_PUBTOPIC = "waterpump"


def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
   
    
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(MQTT_PATH)
# Publish Message to Switch on Motor

def on_publish(client,userdat,result):
    print("data Published \n")

            
# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
        
    print(msg.topic+" "+str(msg.payload))
    message = str(msg.payload)
    message1 = message.replace('b','').replace("'","")
    data = message1.split('#');
    print("SOIL MOISTURE:" + data[0])
    print("TEMPERATURE:" + data[1])
    
    ssm = data[0]
    temp= data[1]
    trimdata = [ssm,temp]
    print(trimdata)
    #write data to csv
    myFields = ['Soil','temp']
    myfile = open('test.csv','w+')
    with myfile:
        writer = csv.writer(myfile)
        writer.writerow(myFields)
        writer.writerow(trimdata)
        
    print("wrting complete")
    try:
        
        #predict
        df = pd.read_csv('soilmoisture.csv')
        X = df.loc[:,'Soil':'temp']
        Y = df.loc[:,'class']
        knn= KNeighborsClassifier()

        knn.fit(X,Y)

        X_test = pd.read_csv('test.csv')
        prediction = knn.predict(X_test)
        print(prediction[0])
        
        soilstat = str(prediction[0])

        print(soilstat)
        #write data to csv
        now = datetime.now() 
        t = now.strftime("%a,%x,%I:%M %p")
        status = [data[0], data[1],soilstat,t]
        myFields = ['soil','temp','status','time']
        myfile = open('soilstatus.csv','a')
        with myfile:
            writer = csv.writer(myfile)
            #writer.writerow(myFields)
            writer.writerow(status)
        print("wrting complete")

        
        print("Beffore CLient1 created")
        client1 = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1);
        client1.on_publish = on_publish
        client1.connect(MQTT_SERVER,1883)
        print("CLient1 created")
        if  soilstat == "dry" or soilstat == "lil dry":
            flag = 1
            #GPIO.output(Relay, False)
            #time.sleep(2)
            ret = client1.publish("waterpump","ON")
            print("ON Message Published")
            #write data to csv
            #irrigationstat = ["irrigation started",t]
            #myFields = ['irrigation','time']
            #myfile = open('irrigation.csv','a')
            #with myfile:
             #   writer = csv.writer(myfile)
                #writer.writerow(myFields)
              #  writer.writerow(irrigationstat)
        
        else:
            
            ret = client1.publish("waterpump","OFF")
            print("OFF Message Published")
            #GPIO.output(Relay, True)
            #time.sleep(2)
            #write data to csv
            #irrigationstat = ["Irrigation is not needed",t]
            #myFields = ['irrigation','time']
            #myfile = open('irrigation.csv','a')
            #with myfile:
             #   writer = csv.writer(myfile)
                #writer.writerow(myFields)
              #  writer.writerow(irrigationstat)
        
    except:
        print("wait for temp value")



    
    # Uncomment after connecting with internet
##    try:
##        #connect to db
##        db = MySQLdb.connect("173.194.230.225","selvam","selma2630","soil_data" )
##        #setup cursor
##        cursor = db.cursor()
##    except:
##        print("DB not Connected")
##    #insert to table
##    try:
##        cursor.execute("""INSERT INTO soil_data VALUES (%s,%s,%s)""",(data[0],data[1],prediction[0]))
##        db.commit()
##        print("Data inserted")
##    except:     
##        db.rollback()
##        print("Data not inserted")
##    #show table
##    cursor.execute("""SELECT * FROM soil_data;""")
##    print cursor.fetchall()
##    db.close()

    # more callbacks, etc

# client = mqtt.Client()
# client = mqtt.Client(client_id="", clean_session=True, userdata=None, protocol=mqtt.MQTTv311, transport="tcp")

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION1)
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_SERVER,1883,60)


# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

client.loop_forever()
