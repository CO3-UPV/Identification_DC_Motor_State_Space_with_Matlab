# Identification of a DC Motor in State Space with Simulink

## Componentes previos

Queriamos identificar el siguiente motor, Pololu Metal Gearmotors - 70:1 Metal Gearmotor 37Dx70L mm 12V with 64 CPR Encoder (https://www.pololu.com/product/4754/specs), este motor cuenta con una reductora de 70 a 1 con varios engranajes.

<p align="center">
<img src='img/motor.jpg' width='250'>
</p>

El modelo que vamos a emplear esta resumido por Paulo Loma Marconi en su blog, enlaces abajo, esta explicado en detalle en el libro dado como refencia. En la carpeta docs podemos encontrar una copia en Español e Inglés.
### Modelo de referencia - Reference model
**Español:**
https://paulomarconi.github.io/es/blog/DCmotor/

**Ingles:**
https://paulomarconi.github.io/blog/DCmotor/

El motor al final del eje tiene cogido al eje un rueda mecanum de 100 mm de diametro y 50 mm de ancho. (https://eu.robotshop.com/es/products/100mm-mecanum-wheel-set) como la de la siguiente imagen.

<p align="center">
<img src='img/100mm-mecanum-wheel-set_1_600x.webp' width='250'>
</p>

Pongamos manos a la obra, para la identificación del modelo vamos a utilizar el siguiente circuito y librerias. El esquematico esta realizado con un herramienta llamada Fritzing.

<p align="center">
<img src='img/Sketch_bb.png' width='400'>
</p>

El circuito es sencillo tenemos un driver para controlar el motor, en este caso, Adafruit Motor Shield V2, un microcontrolador como un Arduino Nano, podría ser cualquier otro, para acelerar el proceso se recomienda un micro que tenga ya las liberias implementadas para los sensores que vayamos a utilizar. Por último, tenemos un Vatímetro como el Gravity: I2C Digital Wattmeter, con este podemos medir voltaje e intensidad consumida. Ambos sensores funcionan a través de I2C lo cual permite una fácil adapatación y uso en cualquier MCU o single board computer como Raspberry.

- Adafruit Motor Shield V2 - https://learn.adafruit.com/adafruit-motor-shield-v2-for-arduino/overview
- Gravity: I2C Digital Wattmeter de DFRobot - https://wiki.dfrobot.com/Gravity:%20I2C%20Digital%20Wattmeter%20SKU:%20SEN0291


## Código Arduino

Para la identificación debemos realizar una serie de experimentos con Arduino, aplicando una entrada al motor y ver su respuesta. Como todos los motores no son iguales, y disponemos de cuatro motores, realizaremos el mismo experimento a los cuatros motores para ver si hay diferencias y como afecta esto al modelo.

Para poder leer la lectura en velocidad necesito descodificar los pulsos de cuadratura que me proporciona el encoder en velocidad angular, para ello, emplearemos una librería que se encarga de darnos el número de pulsos (counts), con este valor y sabiendo el numero de Cuentas Por Revolución (Counts Per Revolution - CPR) dado por el fabricante, podremos saber la velocidad haciendo una regla de tres.

- Encoder Library - https://github.com/PaulStoffregen/Encoder

Para el vatímetro y el driver el fabricante nos proporciona liberías para poder leer los sensores.

- Gravity I2C digital power meter - https://github.com/DFRobot/DFRobot_INA219
- Adafruit Motor Shield V2 Library - https://github.com/adafruit/Adafruit_Motor_Shield_V2_Library

Todas ellas se pueden instalar desde el gestor de librerias de Arduino.

Hay una característica observada de la medida del sensor del vatímetro, este sensor no permite la medición de corriente negativa o de vuelta ya sea porque el driver no  permite la no permite que regrese corriente o el sensor no es capaz de medirla en la otra dirección, es por ello que los resultados se aconseja realizarlos con los valores positivos de las entradas, por ejemplo, aplicando voltajes solo en una dirección para el giro del motor con respuesta positiva o en respuesta a este escalón solo en la respuesta de subida. 

Se han preparado dos tipos de experimento, una serie de escalos que van desde una ganancia pequeña hasta una ganancia máxima y una señal chirp. Conectar el arduino cargar el código, abrir el puerto serie y esperar a que se realice el experimento. Terminará cuando deje de salir texto por el puerto serie, copiar el texto a un archivo de texto y cambiar la extensión a .csv.

## Análisis con Matlab

Para la identificación del sistema, tenemos el siguiente script, Identitification_and_validation_model, que a partir de los datos obtenidos de los experimentos es capaz de identificar un modelo en espacio de estados.
Los parametros a identificar son los siguientes:
A = [-M1 M2; -M3 -M4]
B = [0; M5];
Esta estructura de las matrices A y B con estos parámetros se deduce de los modelos que puedas encontrar en la bibliografía o en la red. Esta estructura sirve tanto para motor con reductora como sin reductora, ya que abstrae los parámetros como 
la inercia, la fricción viscosa, la resistencia de armadura, la inductancia, y la constante eléctrica, propios de un motor de corriente continua, o además, de los parametros para motor con reductora como la relación de los engranajes, la eficiencia de la reductora, posibles fricciones, etcétera.

