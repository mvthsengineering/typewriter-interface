from flask import Flask, request
from twilio.twiml.messaging_response import Message, MessagingResponse
from datetime import datetime
import serial
import atexit

port = serial.Serial('/dev/ttyACM0', 9600)

def OnExit(user):
    port.close()

atexit.register(OnExit, user='sam')

app = Flask(__name__)

@app.route('/', methods=['POST'])

def sms():
    now = datetime.now()
    current_time = now.strftime("%D %I:%M%p ")


    number = request.form['From']
    message_body = request.form['Body']

    current_time = current_time.encode()
    #message = message_body.encode()

    port.write(current_time)
    m1 = message_body[0:60]
    port.write(m1.encode())
    if len(message_body) > 60:
        port.write(b'-')
        port.write(b'\r')
    #if len(message_body) > 60:
        #m2 = message_body[60:]
        m2 = message_body[60:]

        #message = m2.encode()
        port.write(m2.encode())
        #port.write(b'\r')
    
    port.write(b'\r')
    print(current_time)
    print(number)
    print(message_body)
    return message_body

if __name__ == '__main__':
    app.run()
