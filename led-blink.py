import RPi.GPIO as GPIO
from time import sleep

GPIO.setmode(GPIO.BCM)

blinkCount = 3
count = 0
LEDPin = 17

GPIO.setup(LEDPin, GPIO.OUT)

try:

    while count < blinkCount:
        GPIO.output(LEDPin, True)
        print("LED ON")
        sleep(2)
        GPIO.output(LEDPin, False)
        print("LED OFF")
        sleep(1)
        count += 1
finally:
    # reset GPIO pins to safe state
    GPIO.cleanup()
