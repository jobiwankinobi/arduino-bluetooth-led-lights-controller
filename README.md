# arduino-bluetooth-led-lights-controller
This project represents an Arduino implementation of a bluetooth-controlled LED light strip.  The sketch allows communication between bluetooth module and Arduino through both hardware and software serial options.  As the code notes, the software serial option is not ideal for strips longer than 120 lights.  However, use of the hardware serial may require that the bluetooth module be disconnected while uploading the sketch to the Arduino.

It was inspired by http://funkboxing.com/wordpress/?p=1366.  Changes were made to reduce the memory footprint and incorporate additional LED light strip functions.

An Android app is being developed to drive the controller, and should be available in the near future.  An extension of this code to enable uploading a sketch wirelessly to the Arduino using the bluetooth module is also under development.
