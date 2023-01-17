from flask import Flask, request
from twilio.twiml.messaging_response import Message, MessagingResponse
from datetime import datetime
#from pandas import read_csv
from serial import Serial
#import serial
import atexit
#import csv

port = Serial('/dev/ttyACM0', 9600)

def OnExit(user):
    port.close()

atexit.register(OnExit, user='sam')

app = Flask(__name__)

@app.route('/', methods=['POST'])

def sms():
    now = datetime.now()
    
    #current_time = now.strftime("%D %I:%M%p ")
    current_time = now.strftime("%I:%M%p ")


    number = request.form['From']
    message_body = request.form['Body']

    #numbers = read_csv('phone_numbers.csv')

    #from IPython import embed; embed()
    #numbers = numbers.set_index('number').to_dict()['name']
    LINE_WIDTH = 70

    text_to_write = current_time + message_body
   
    while(text_to_write):
        port.write(text_to_write[:LINE_WIDTH].encode())
        text_to_write = text_to_write[LINE_WIDTH:]
        if text_to_write:
            port.write(b'-')
        port.write(b'\r')

    print(current_time)
    print(number)
    print(message_body)
   # with open('numbers.csv', mode='a') as csv_file:
   #@     writer = csv.writer(csv_file)
    #    writer.writerow(number)
    return message_body

if __name__ == '__main__':
    app.run()
