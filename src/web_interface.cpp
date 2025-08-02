#include <WebServer.h>
#include "web_interface.h"
#include "definitions.h"
#include "deauth.h"

WebServer server(80);
int num_networks;

String getEncryptionType(wifi_auth_mode_t encryptionType);

void redirect_root()
{
    server.sendHeader("Location", "/");
    server.send(301);
}

void handle_root()
{
    String html = R"(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Deauther Pro</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            color: #333;
            line-height: 1.6;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        
        .header {
            text-align: center;
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 30px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
            backdrop-filter: blur(10px);
        }
        
        .header h1 {
            font-size: 2.5rem;
            background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            margin-bottom: 10px;
            font-weight: bold;
        }
        
        .subtitle {
            color: #666;
            font-size: 1.1rem;
            font-weight: 300;
        }
        
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .stat-card {
            background: rgba(255, 255, 255, 0.9);
            border-radius: 15px;
            padding: 20px;
            text-align: center;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        
        .stat-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 25px rgba(0, 0, 0, 0.2);
        }
        
        .stat-number {
            font-size: 2rem;
            font-weight: bold;
            color: #4ecdc4;
            display: block;
        }
        
        .stat-label {
            color: #666;
            font-size: 0.9rem;
            margin-top: 5px;
        }
        
        .networks-section {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
        }
        
        .section-title {
            font-size: 1.5rem;
            color: #333;
            margin-bottom: 20px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .section-icon {
            width: 30px;
            height: 30px;
            background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
        }
        
        .networks-table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
            background: white;
            border-radius: 10px;
            overflow: hidden;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.1);
        }
        
        .networks-table th {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            padding: 15px 12px;
            text-align: left;
            font-weight: 600;
            font-size: 0.9rem;
        }
        
        .networks-table td {
            padding: 12px;
            border-bottom: 1px solid #f0f0f0;
            font-size: 0.9rem;
        }
        
        .networks-table tr:nth-child(even) {
            background-color: #f8f9ff;
        }
        
        .networks-table tr:hover {
            background-color: #e8f0fe;
            transform: scale(1.01);
            transition: all 0.2s ease;
        }
        
        .signal-strength {
            display: inline-block;
            padding: 4px 8px;
            border-radius: 12px;
            font-size: 0.8rem;
            font-weight: bold;
        }
        
        .signal-excellent { background: #4caf50; color: white; }
        .signal-good { background: #8bc34a; color: white; }
        .signal-fair { background: #ff9800; color: white; }
        .signal-poor { background: #f44336; color: white; }
        
        .encryption-badge {
            display: inline-block;
            padding: 4px 8px;
            border-radius: 12px;
            font-size: 0.8rem;
            font-weight: bold;
            background: #e3f2fd;
            color: #1976d2;
        }
        
        .control-panel {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .control-card {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
        }
        
        .control-card h3 {
            color: #333;
            margin-bottom: 15px;
            font-size: 1.2rem;
        }
        
        .input-group {
            margin-bottom: 15px;
        }
        
        .input-group label {
            display: block;
            margin-bottom: 5px;
            color: #666;
            font-weight: 500;
        }
        
        .form-input {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            font-size: 1rem;
            transition: border-color 0.3s ease;
            background: white;
        }
        
        .form-input:focus {
            outline: none;
            border-color: #4ecdc4;
            box-shadow: 0 0 0 3px rgba(78, 205, 196, 0.1);
        }
        
        .btn {
            width: 100%;
            padding: 12px 20px;
            border: none;
            border-radius: 10px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-top: 10px;
        }
        
        .btn-primary {
            background: linear-gradient(45deg, #4ecdc4, #44a08d);
            color: white;
        }
        
        .btn-primary:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(78, 205, 196, 0.4);
        }
        
        .btn-warning {
            background: linear-gradient(45deg, #ff9800, #f57c00);
            color: white;
        }
        
        .btn-warning:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(255, 152, 0, 0.4);
        }
        
        .btn-danger {
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            color: white;
        }
        
        .btn-danger:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(255, 107, 107, 0.4);
        }
        
        .btn-scan {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
        }
        
        .btn-scan:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        
        .codes-section {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 25px;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.1);
        }
        
        .codes-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        
        .code-card {
            background: #f8f9ff;
            border-left: 4px solid #4ecdc4;
            border-radius: 8px;
            padding: 15px;
            transition: transform 0.2s ease;
        }
        
        .code-card:hover {
            transform: translateX(5px);
            background: #e8f0fe;
        }
        
        .code-number {
            font-size: 1.5rem;
            font-weight: bold;
            color: #4ecdc4;
            margin-bottom: 5px;
        }
        
        .code-title {
            font-weight: 600;
            color: #333;
            margin-bottom: 8px;
        }
        
        .code-description {
            font-size: 0.9rem;
            color: #666;
            line-height: 1.4;
        }
        
        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }
            
            .header h1 {
                font-size: 1.8rem;
            }
            
            .control-panel {
                grid-template-columns: 1fr;
            }
            
            .codes-grid {
                grid-template-columns: 1fr;
            }
        }
        
        .pulse {
            animation: pulse 2s infinite;
        }
        
        @keyframes pulse {
            0% { transform: scale(1); }
            50% { transform: scale(1.05); }
            100% { transform: scale(1); }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>ESP32 WiFi Deauther Pro</h1>
            <p class="subtitle">Herramienta avanzada de pentesting para redes inalambricas</p>
        </div>
        
        <div class="stats-grid">
            <div class="stat-card">
                <span class="stat-number">)" + String(num_networks) + R"(</span>
                <div class="stat-label">Redes Detectadas</div>
            </div>
            <div class="stat-card">
                <span class="stat-number">)" + String(eliminated_stations) + R"(</span>
                <div class="stat-label">Estaciones Eliminadas</div>
            </div>
        </div>
        
        <div class="networks-section">
            <h2 class="section-title">
                <div class="section-icon">W</div>
                Redes WiFi Disponibles
            </h2>
            <table class="networks-table">
                <tr>
                    <th>ID</th>
                    <th>SSID</th>
                    <th>BSSID</th>
                    <th>Canal</th>
                    <th>Senal</th>
                    <th>Seguridad</th>
                </tr>
)";

    for (int i = 0; i < num_networks; i++)
    {
        String encryption = getEncryptionType(WiFi.encryptionType(i));
        int rssi = WiFi.RSSI(i);
        String signalClass = "";
        
        if (rssi > -50) signalClass = "signal-excellent";
        else if (rssi > -60) signalClass = "signal-good";
        else if (rssi > -70) signalClass = "signal-fair";
        else signalClass = "signal-poor";
        
        html += "<tr><td><strong>" + String(i) + "</strong></td>";
        html += "<td><strong>" + WiFi.SSID(i) + "</strong></td>";
        html += "<td><code>" + WiFi.BSSIDstr(i) + "</code></td>";
        html += "<td>" + String(WiFi.channel(i)) + "</td>";
        html += "<td><span class=\"signal-strength " + signalClass + "\">" + String(rssi) + " dBm</span></td>";
        html += "<td><span class=\"encryption-badge\">" + encryption + "</span></td></tr>";
    }

    html += R"(
            </table>
            
            <form method="post" action="/rescan">
                <button type="submit" class="btn btn-scan">Escanear Redes WiFi</button>
            </form>
        </div>

        <div class="control-panel">
            <div class="control-card">
                <h3>Ataque Dirigido</h3>
                <form method="post" action="/deauth">
                    <div class="input-group">
                        <label for="net_num">Numero de Red Objetivo</label>
                        <input type="text" name="net_num" id="net_num" class="form-input" placeholder="Ej: 0, 1, 2..." required>
                    </div>
                    <div class="input-group">
                        <label for="reason">Codigo de Razon</label>
                        <input type="text" name="reason" id="reason" class="form-input" placeholder="Ej: 1, 2, 3..." required>
                    </div>
                    <button type="submit" class="btn btn-warning">Iniciar Ataque Dirigido</button>
                </form>
            </div>

            <div class="control-card">
                <h3>Ataque Masivo</h3>
                <form method="post" action="/deauth_all">
                    <div class="input-group">
                        <label for="reason_all">Codigo de Razon</label>
                        <input type="text" name="reason" id="reason_all" class="form-input" placeholder="Ej: 1, 2, 3..." required>
                    </div>
                    <button type="submit" class="btn btn-danger pulse">Atacar Todas las Redes</button>
                    <p style="font-size: 0.8rem; color: #666; margin-top: 10px;">
                        ⚠️ Advertencia: Este modo desconectara el WiFi del dispositivo
                    </p>
                </form>
            </div>
        </div>

        <div class="control-card" style="text-align: center; margin-bottom: 30px;">
            <form method="post" action="/stop">
                <button type="submit" class="btn btn-primary">Detener Todos los Ataques</button>
            </form>
        </div>

        <div class="codes-section">
            <h2 class="section-title">
                <div class="section-icon">#</div>
                Codigos de Desautenticacion
            </h2>
            <div class="codes-grid">
                <div class="code-card">
                    <div class="code-number">1</div>
                    <div class="code-title">Razon No Especificada</div>
                    <div class="code-description">Desconexion por razon desconocida o no especificada</div>
                </div>
                <div class="code-card">
                    <div class="code-number">2</div>
                    <div class="code-title">Autenticacion Invalida</div>
                    <div class="code-description">La autenticacion previa ya no es valida</div>
                </div>
                <div class="code-card">
                    <div class="code-number">3</div>
                    <div class="code-title">Estacion Saliendo</div>
                    <div class="code-description">La estacion se desconecta porque esta saliendo</div>
                </div>
                <div class="code-card">
                    <div class="code-number">4</div>
                    <div class="code-title">Inactividad</div>
                    <div class="code-description">Desconexion por periodo prolongado de inactividad</div>
                </div>
                <div class="code-card">
                    <div class="code-number">5</div>
                    <div class="code-title">Sobrecarga AP</div>
                    <div class="code-description">El punto de acceso no puede manejar mas conexiones</div>
                </div>
                <div class="code-card">
                    <div class="code-number">6</div>
                    <div class="code-title">Trama Clase 2</div>
                    <div class="code-description">Trama de clase 2 de estacion no autenticada</div>
                </div>
                <div class="code-card">
                    <div class="code-number">7</div>
                    <div class="code-title">Trama Clase 3</div>
                    <div class="code-description">Trama de clase 3 de estacion no asociada</div>
                </div>
                <div class="code-card">
                    <div class="code-number">8</div>
                    <div class="code-title">STA Saliendo</div>
                    <div class="code-description">Estacion disociada porque esta saliendo del BSS</div>
                </div>
                <div class="code-card">
                    <div class="code-number">14</div>
                    <div class="code-title">Error MIC</div>
                    <div class="code-description">Fallo en el codigo de integridad del mensaje</div>
                </div>
                <div class="code-card">
                    <div class="code-number">15</div>
                    <div class="code-title">Timeout 4-Way</div>
                    <div class="code-description">Tiempo agotado en handshake de 4 vias</div>
                </div>
                <div class="code-card">
                    <div class="code-number">23</div>
                    <div class="code-title">802.1X Fallo</div>
                    <div class="code-description">Fallo en autenticacion IEEE 802.1X</div>
                </div>
                <div class="code-card">
                    <div class="code-number">24</div>
                    <div class="code-title">Cifrado Rechazado</div>
                    <div class="code-description">Conjunto de cifrado rechazado por politica</div>
                </div>
            </div>
        </div>
    </div>
</body>
</html>
)";

    server.send(200, "text/html", html);
}

void handle_deauth()
{
    int wifi_number = server.arg("net_num").toInt();
    uint16_t reason = server.arg("reason").toInt();

    String html = R"(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Estado del Ataque - ESP32 Deauther</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: #333;
        }
        
        .alert-container {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 40px;
            text-align: center;
            max-width: 500px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.2);
            backdrop-filter: blur(10px);
            animation: slideIn 0.5s ease-out;
        }
        
        @keyframes slideIn {
            from {
                transform: translateY(-50px);
                opacity: 0;
            }
            to {
                transform: translateY(0);
                opacity: 1;
            }
        }
        
        .success-icon {
            width: 80px;
            height: 80px;
            background: linear-gradient(45deg, #4caf50, #45a049);
            border-radius: 50%;
            margin: 0 auto 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 40px;
            color: white;
            animation: pulse 2s infinite;
        }
        
        .error-icon {
            width: 80px;
            height: 80px;
            background: linear-gradient(45deg, #f44336, #d32f2f);
            border-radius: 50%;
            margin: 0 auto 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 40px;
            color: white;
            animation: shake 0.5s ease-in-out;
        }
        
        @keyframes pulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }
        
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            75% { transform: translateX(10px); }
        }
        
        .alert-title {
            font-size: 1.8rem;
            font-weight: bold;
            margin-bottom: 15px;
            color: #333;
        }
        
        .alert-message {
            font-size: 1.1rem;
            margin-bottom: 10px;
            color: #666;
            line-height: 1.6;
        }
        
        .detail-info {
            background: #f8f9ff;
            border-radius: 10px;
            padding: 15px;
            margin: 20px 0;
            border-left: 4px solid #4ecdc4;
        }
        
        .detail-label {
            font-weight: 600;
            color: #4ecdc4;
            margin-bottom: 5px;
        }
        
        .detail-value {
            font-size: 1.2rem;
            font-weight: bold;
            color: #333;
        }
        
        .btn-home {
            display: inline-block;
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            padding: 15px 30px;
            text-decoration: none;
            border-radius: 25px;
            font-weight: 600;
            margin-top: 20px;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .btn-home:hover {
            transform: translateY(-3px);
            box-shadow: 0 10px 20px rgba(102, 126, 234, 0.3);
        }
        
        .warning-text {
            font-size: 0.9rem;
            color: #ff9800;
            margin-top: 15px;
            font-style: italic;
        }
    </style>
</head>
<body>
    <div class="alert-container)";

    if (wifi_number < num_networks)
    {
        html += R"(">
        <div class="success-icon">⚡</div>
        <h2 class="alert-title">Ataque Iniciado Exitosamente</h2>
        <p class="alert-message">El ataque de desautenticacion ha comenzado correctamente</p>
        
        <div class="detail-info">
            <div class="detail-label">Red Objetivo:</div>
            <div class="detail-value">Red #)" + String(wifi_number) + R"(</div>
        </div>
        
        <div class="detail-info">
            <div class="detail-label">Codigo de Razon:</div>
            <div class="detail-value">)" + String(reason) + R"(</div>
        </div>
        
        <p class="warning-text">El ataque esta en progreso. Monitorea las estadisticas en la pagina principal.</p>
        )";
        start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
    }
    else
    {
        html += R"( error">
        <div class="error-icon">⚠</div>
        <h2 class="alert-title">Error en el Ataque</h2>
        <p class="alert-message">Numero de red WiFi invalido</p>
        
        <div class="detail-info">
            <div class="detail-label">Red Solicitada:</div>
            <div class="detail-value">#)" + String(wifi_number) + R"(</div>
        </div>
        
        <p class="alert-message">Por favor selecciona una red valida del listado (0 a )" + String(num_networks - 1) + R"()</p>
        )";
    }

    html += R"(
        <a href="/" class="btn-home">Regresar al Panel Principal</a>
    </div>
</body>
</html>
)";

    server.send(200, "text/html", html);
}

void handle_deauth_all()
{
    uint16_t reason = server.arg("reason").toInt();

    String html = R"(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ataque Masivo - ESP32 Deauther</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #ff6b6b 0%, #ee5a52 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: #333;
        }
        
        .warning-container {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 40px;
            text-align: center;
            max-width: 500px;
            box-shadow: 0 20px 40px rgba(0, 0, 0, 0.3);
            backdrop-filter: blur(10px);
            animation: dangerPulse 2s infinite;
        }
        
        @keyframes dangerPulse {
            0%, 100% { 
                transform: scale(1);
                box-shadow: 0 20px 40px rgba(255, 107, 107, 0.3);
            }
            50% { 
                transform: scale(1.02);
                box-shadow: 0 25px 50px rgba(255, 107, 107, 0.5);
            }
        }
        
        .danger-icon {
            width: 100px;
            height: 100px;
            background: linear-gradient(45deg, #ff6b6b, #ee5a52);
            border-radius: 50%;
            margin: 0 auto 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 50px;
            color: white;
            animation: spin 3s linear infinite;
        }
        
        @keyframes spin {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }
        
        .warning-title {
            font-size: 2rem;
            font-weight: bold;
            margin-bottom: 15px;
            color: #d32f2f;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .warning-message {
            font-size: 1.2rem;
            margin-bottom: 15px;
            color: #333;
            line-height: 1.6;
            font-weight: 500;
        }
        
        .critical-warning {
            background: linear-gradient(45deg, #ffeb3b, #ffc107);
            color: #333;
            padding: 20px;
            border-radius: 15px;
            margin: 20px 0;
            font-weight: bold;
            border: 3px solid #ff9800;
            animation: blink 1.5s infinite;
        }
        
        @keyframes blink {
            0%, 50% { opacity: 1; }
            51%, 100% { opacity: 0.7; }
        }
        
        .reason-info {
            background: #f8f9ff;
            border-radius: 10px;
            padding: 15px;
            margin: 20px 0;
            border-left: 4px solid #ff6b6b;
        }
        
        .reason-label {
            font-weight: 600;
            color: #ff6b6b;
            margin-bottom: 5px;
        }
        
        .reason-value {
            font-size: 1.5rem;
            font-weight: bold;
            color: #333;
        }
        
        .countdown {
            font-size: 3rem;
            font-weight: bold;
            color: #ff6b6b;
            margin: 20px 0;
            animation: countdown 1s infinite;
        }
        
        @keyframes countdown {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.1); }
        }
        
        .status-text {
            font-size: 1.1rem;
            color: #666;
            margin-top: 15px;
            font-style: italic;
        }
    </style>
</head>
<body>
    <div class="warning-container">
        <div class="danger-icon">💥</div>
        <h1 class="warning-title">Ataque Masivo Iniciado</h1>
        <p class="warning-message">Se ha iniciado el ataque de desautenticacion masiva contra todas las redes detectadas</p>
        
        <div class="critical-warning">
            ⚠️ ATENCION: El WiFi del dispositivo se desconectara automaticamente
        </div>
        
        <div class="reason-info">
            <div class="reason-label">Codigo de Razon Utilizado:</div>
            <div class="reason-value">)" + String(reason) + R"(</div>
        </div>
        
        <div class="countdown">🔥</div>
        
        <p class="status-text">Para detener el ataque, reinicia fisicamente el dispositivo ESP32</p>
        
        <div style="margin-top: 30px; padding: 15px; background: #ffebee; border-radius: 10px; color: #d32f2f;">
            <strong>Nota de Seguridad:</strong><br>
            Este ataque afectara a todas las redes en el area. Usar solo con fines educativos y de testing autorizado.
        </div>
    </div>
</body>
</html>
)";

    server.send(200, "text/html", html);
    server.stop();
    start_deauth(0, DEAUTH_TYPE_ALL, reason);
}

void handle_rescan()
{
    num_networks = WiFi.scanNetworks();
    redirect_root();
}

void handle_stop()
{
    stop_deauth();
    redirect_root();
}

void start_web_interface()
{
    server.on("/", handle_root);
    server.on("/deauth", handle_deauth);
    server.on("/deauth_all", handle_deauth_all);
    server.on("/rescan", handle_rescan);
    server.on("/stop", handle_stop);

    server.begin();
}

void web_interface_handle_client()
{
    server.handleClient();
}

String getEncryptionType(wifi_auth_mode_t encryptionType)
{
    switch (encryptionType)
    {
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
