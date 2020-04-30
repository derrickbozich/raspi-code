import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)

RGB = [22,23,24]
for pin in RGB:
    GPIO.setup(pin, GPIO.OUT)
    GPIO.output(pin, 0)
    
try:
    while(True):
        request = input("RGB-->")
        if (len(request) == 3):
            GPIO.output(23, int(request[0]))
            GPIO.output(22, int(request[1]))
            GPIO.output(24, int(request[2]))
            
except KeyboardInterrupt:
    GPIO.cleanup()


