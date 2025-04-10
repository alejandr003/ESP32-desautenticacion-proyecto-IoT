#include <WebServer.h>
#include "web_interface.h"
#include "definitions.h"
#include "deauth.h"

WebServer server(80);
int num_networks;

// Move the function declaration to the top
String getEncryptionType(wifi_auth_mode_t encryptionType);

void redirect_root() {
  server.sendHeader("Location", "/");
  server.send(301);
}

void handle_root() {
  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-WIFI</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 20px;
            background-color: #f4f4f4;
        }
        h1, h2 {
            color: #2c3e50;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
        }
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        th {
            background-color: #3498db;
            color: white;
        }
        tr:nth-child(even) {
            background-color: #f2f2f2;
        }
        form {
            background-color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        input[type="text"], input[type="submit"] {
            width: 100%;
            padding: 10px;
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 4px;
        }
        input[type="submit"] {
            background-color: #3498db;
            color: white;
            border: none;
            cursor: pointer;
            transition: background-color 0.3s;
        }
        input[type="submit"]:hover {
            background-color: #2980b9;
        }
    </style>
</head>
<body>
    <h1>ESP32 Desautenticación de Wifi Killer</h1>
    
    <h2>Redes WIFI</h2>
    <table>
        <tr>
            <th>Numero</th>
            <th>SSID</th>
            <th>BSSID</th>
            <th>Canal</th>
            <th>RSSI</th>
            <th>Encriptación</th>
        </tr>
)";

  for (int i = 0; i < num_networks; i++) {
    String encryption = getEncryptionType(WiFi.encryptionType(i));
    html += "<tr><td>" + String(i) + "</td><td>" + WiFi.SSID(i) + "</td><td>" + WiFi.BSSIDstr(i) + "</td><td>" + 
            String(WiFi.channel(i)) + "</td><td>" + String(WiFi.RSSI(i)) + "</td><td>" + encryption + "</td></tr>";
  }

  html += R"(
    </table>

    <form method="post" action="/rescan">
        <input type="submit" value="Escanear Redes WIFI">
    </form>

    <form method="post" action="/deauth">
        <h2>Desautenticador de una red especifica</h2>
        <input type="text" name="net_num" placeholder="Numero de red de ataque">
        <input type="text" name="reason" placeholder="Codigo a ejecutar">
        <input type="submit" value="Lanzar ataque">
    </form>

    <p>Estaciones eliminadas: )" + String(eliminated_stations) + R"(</p>

        <form method="post" action="/stop">
        <input type="submit" value="Detener el ataque">
    </form>

    <form method="post" action="/deauth_all">
        <h2>Desautenticador masivo de redes</h2>
        <input type="text" name="reason" placeholder="Codigo a ejecutar">
        <input type="submit" value="Atacar a todas las redes">
    </form>

    <form method="post" action="/stop">
        <input type="submit" value="Detener todos los ataques">
    </form>

    <h2>Codigos de ejecución</h2>
<table>
    <tr>
        <th>Código</th>
        <th>Significado</th>
    </tr>
    <tr>
        <td>0</td>
        <td>
            <details>
                <summary>Reservado</summary>
                Este código está reservado y no tiene un uso específico asignado.
            </details>
        </td>
    </tr>
    <tr>
        <td>1</td>
        <td>
            <details>
                <summary>Razón no especificada</summary>
                La desconexión ocurrió por una razón no especificada.
            </details>
        </td>
    </tr>
    <tr>
        <td>2</td>
        <td>
            <details>
                <summary>Autenticación previa ya no válida</summary>
                La autenticación previa ya no es válida, lo que provoca la desconexión.
            </details>
        </td>
    </tr>
    <tr>
        <td>3</td>
        <td>
            <details>
                <summary>Desautenticado porque la estación está saliendo</summary>
                La estación (STA) se desautenticó porque está saliendo o ha salido del conjunto de servicios básicos (IBSS o ESS).
            </details>
        </td>
    </tr>
    <tr>
        <td>4</td>
        <td>
            <details>
                <summary>Disociado por inactividad</summary>
                La estación fue disociada debido a un período prolongado de inactividad.
            </details>
        </td>
    </tr>
    <tr>
        <td>5</td>
        <td>
            <details>
                <summary>Disociado por sobrecarga del dispositivo WAP</summary>
                El dispositivo WAP no puede manejar todas las estaciones asociadas actualmente.
            </details>
        </td>
    </tr>
    <tr>
        <td>6</td>
        <td>
            <details>
                <summary>Trama de clase 2 recibida de una STA no autenticada</summary>
                Se recibió una trama de clase 2 de una estación que no está autenticada.
            </details>
        </td>
    </tr>
    <tr>
        <td>7</td>
        <td>
            <details>
                <summary>Trama de clase 3 recibida de una STA no asociada</summary>
                Se recibió una trama de clase 3 de una estación que no está asociada.
            </details>
        </td>
    </tr>
    <tr>
        <td>8</td>
        <td>
            <details>
                <summary>Disociado porque la STA está saliendo</summary>
                La estación fue disociada porque está saliendo o ha salido del conjunto de servicios básicos (BSS).
            </details>
        </td>
    </tr>
    <tr>
        <td>9</td>
        <td>
            <details>
                <summary>Solicitud de (re)asociación no autenticada</summary>
                La estación solicitante no está autenticada con la estación que responde.
            </details>
        </td>
    </tr>
    <tr>
        <td>10</td>
        <td>
            <details>
                <summary>Capacidad de potencia inaceptable</summary>
                La información en el elemento de capacidad de potencia no es aceptable.
            </details>
        </td>
    </tr>
    <tr>
        <td>11</td>
        <td>
            <details>
                <summary>Canales soportados inaceptables</summary>
                La información en el elemento de canales soportados no es aceptable.
            </details>
        </td>
    </tr>
    <tr>
        <td>12</td>
        <td>
            <details>
                <summary>Disociado por gestión de transición BSS</summary>
                La estación fue disociada debido a la gestión de transición del conjunto de servicios básicos.
            </details>
        </td>
    </tr>
    <tr>
        <td>13</td>
        <td>
            <details>
                <summary>Elemento inválido</summary>
                Un elemento definido en el estándar no cumple con las especificaciones.
            </details>
        </td>
    </tr>
    <tr>
        <td>14</td>
        <td>
            <details>
                <summary>Error en el código de integridad del mensaje (MIC)</summary>
                Se detectó un fallo en el código de integridad del mensaje.
            </details>
        </td>
    </tr>
    <tr>
        <td>15</td>
        <td>
            <details>
                <summary>Tiempo de espera en el handshake de 4 vías</summary>
                El proceso de handshake de 4 vías excedió el tiempo de espera.
            </details>
        </td>
    </tr>
    <tr>
        <td>16</td>
        <td>
            <details>
                <summary>Tiempo de espera en el handshake de clave grupal</summary>
                El proceso de handshake de clave grupal excedió el tiempo de espera.
            </details>
        </td>
    </tr>
    <tr>
        <td>17</td>
        <td>
            <details>
                <summary>Elemento inválido en el handshake de 4 vías</summary>
                Un elemento en el handshake de 4 vías no coincide con el marco de solicitud de asociación.
            </details>
        </td>
    </tr>
    <tr>
        <td>18</td>
        <td>
            <details>
                <summary>Cifrado grupal inválido</summary>
                El cifrado grupal especificado no es válido.
            </details>
        </td>
    </tr>
    <tr>
        <td>19</td>
        <td>
            <details>
                <summary>Cifrado por pares inválido</summary>
                El cifrado por pares especificado no es válido.
            </details>
        </td>
    </tr>
    <tr>
        <td>20</td>
        <td>
            <details>
                <summary>AKMP inválido</summary>
                El Protocolo de Gestión de Claves de Autenticación (AKMP) especificado no es válido.
            </details>
        </td>
    </tr>
    <tr>
        <td>21</td>
        <td>
            <details>
                <summary>Versión RSNE no soportada</summary>
                La versión del Elemento de Negociación de Seguridad de Red (RSNE) no es soportada.
            </details>
        </td>
    </tr>
    <tr>
        <td>22</td>
        <td>
            <details>
                <summary>Capacidades RSNE inválidas</summary>
                Las capacidades especificadas en el RSNE no son válidas.
            </details>
        </td>
    </tr>
    <tr>
        <td>23</td>
        <td>
            <details>
                <summary>Fallo en la autenticación IEEE 802.1X</summary>
                La autenticación IEEE 802.1X falló.
            </details>
        </td>
    </tr>
    <tr>
        <td>24</td>
        <td>
            <details>
                <summary>Rechazo del conjunto de cifrado</summary>
                El conjunto de cifrado fue rechazado debido a la política de seguridad.
            </details>
        </td>
    </tr>
</table>
</body>
</html>
)";

  server.send(200, "text/html", html);
}


void handle_deauth() {
  int wifi_number = server.arg("net_num").toInt();
  uint16_t reason = server.arg("reason").toInt();

  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ataque de una red</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f0f0f0;
        }
        .alert {
            background-color: #4CAF50;
            color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
            text-align: center;
        }
        .alert.error {
            background-color: #f44336;
        }
        .button {
            display: inline-block;
            padding: 10px 20px;
            margin-top: 20px;
            background-color: #008CBA;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .button:hover {
            background-color: #005f73;
        }
    </style>
</head>
<body>
    <div class="alert)";

  if (wifi_number < num_networks) {
    html += R"(">
        <h2>Comenzando el Ataque de Desautenticación.</h2>
        <p>Desautenticando la red wifi numero: )" + String(wifi_number) + R"(</p>
        <p>Codigo de ataque: )" + String(reason) + R"(</p>
    </div>)";
    start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
  } else {
    html += R"( error">
        <h2>Error:Número de red wifi inválida </h2>
        <p>Por favor selecciona una red wifi válida.</p>
    </div>)";
  }

  html += R"(
    <a href="/" class="button">Regresar al inicio</a>
</body>
</html>
  )";

  server.send(200, "text/html", html);
}

void handle_deauth_all() {
  uint16_t reason = server.arg("reason").toInt();

  String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ataque de todas las redes</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background-color: #f0f0f0;
        }
        .alert {
            background-color: #ff9800;
            color: white;
            padding: 20px;
            border-radius: 5px;
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
            text-align: center;
        }
        .button {
            display: inline-block;
            padding: 10px 20px;
            margin-top: 20px;
            background-color: #008CBA;
            color: white;
            text-decoration: none;
            border-radius: 5px;
            transition: background-color 0.3s;
        }
        .button:hover {
            background-color: #005f73;
        }
    </style>
</head>
<body>
    <div class="alert">
        <h2>Comienza el ataque de desautenticación en todas las redes.</h2>
        <p>El WiFi se apagará ahora. Para detener el ataque, por favor reinicie el ESP32.</p>
        <p>Reason code: )" + String(reason) + R"(</p>
    </div>
</body>
</html>
  )";

  server.send(200, "text/html", html);
  server.stop();
  start_deauth(0, DEAUTH_TYPE_ALL, reason);
}

void handle_rescan() {
  num_networks = WiFi.scanNetworks();
  redirect_root();
}

void handle_stop() {
  stop_deauth();
  redirect_root();
}

void start_web_interface() {
  server.on("/", handle_root);
  server.on("/deauth", handle_deauth);
  server.on("/deauth_all", handle_deauth_all);
  server.on("/rescan", handle_rescan);
  server.on("/stop", handle_stop);

  server.begin();
}

void web_interface_handle_client() {
  server.handleClient();
}

// The function implementation can stay where it is
String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2_ENTERPRISE";
    default:
      return "UNKNOWN";
  }
}
