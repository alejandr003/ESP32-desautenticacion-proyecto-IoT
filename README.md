# ESP32-Killer

![logo](https://github.com/user-attachments/assets/4e2ac65f-1b25-4a97-822a-6a91ca71b5be)

Un proyecto para el ESP32 que permite desautenticar estaciones conectadas a una red WiFi.

# AVISO
Esta herramienta es para pruebas y de tipo educativo. El uso de esta herramienta es bajo tu responsabilidad. **No** soy responsable de las consecuencias derivadas, es bajo tu propio riesgo.

## Construcción
Clona este repositorio:

`git clone https://github.com/alejandr003/ESP32-desautenticacion-proyecto-IoT`

1) Tener instalado VSC.
2) Instala la extensión de PlatformIO en vscode.
3) Abrir la carpeta del proyecto de dónde lo hayas guardado.
4) Conecta la ESP32 a tu equipo.
5) Verifica el COM donde se haya alojado y que tu computadora lo reconozca. NOTA: Asegurate de que tengas el ESP32 tenga los drivers necesarios para su uso.

![Texto alternativo](https://ukmars.org/ukmars/wp-content/uploads/2020/08/platformio-logo.png)

## Usando ESP32
El ESP32 aloja una red WiFi con el nombre `RED-Oculta` y una contraseña `03esp32izzi`. Conéctate a esta red y escribe la siguinte IP: **192.168.4.1** en un navegador web de tu elección. Verás las siguientes opciones:
* Reescanear redes: Escanea y detecta todas las redes WiFi en tu área. Después de un escaneo exitoso, las redes se listarán en la tabla superior.
* Lanzar ataque de desautenticación: Desautentica a todos los clientes conectados a una red. Ingresa el número de red de la tabla superior y un código de razón de la tabla inferior de la página. Luego haz clic en el botón y el LED de tu ESP32 parpadeará mientras desautentica una estación.
* Desautenticar todas las redes: Lanza un ataque de desautenticación en todas las redes y estaciones con un código de razón específico. Para detener esto, debes reiniciar tu ESP32 (no hay otra forma de programarlo ya que el ESP32 cambia rápidamente entre todos los canales de red y debe deshabilitar su AP).
* Detener ataque de desautenticación: Detiene un ataque de desautenticación en curso.

