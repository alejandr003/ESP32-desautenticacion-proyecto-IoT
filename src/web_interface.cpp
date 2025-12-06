#include <WiFi.h>
#include <WebServer.h>
// #include "web_interface.h" // Incluidos en este archivo
// #include "definitions.h"   // Incluidos en este archivo
// #include "deauth.h"       // Incluidos en este archivo

// Definiciones de variables globales (simuladas si no tienes los .h)
// **NOTA:** Asumo que estas variables están definidas en 'definitions.h' y 'deauth.h'
// Si estos archivos no existen, deberás definirlas aquí para que compile.
// Ejemplo:
// extern int eliminated_stations; // Si viene de deauth.h
int eliminated_stations = 0; // Simulando la variable si no existe
#define DEAUTH_TYPE_SINGLE 1
#define DEAUTH_TYPE_ALL 2
extern void start_deauth(int net_num, int type, uint16_t reason);
extern void stop_deauth();


WebServer server(80);
int num_networks;

// Declaraciones de funciones
String getEncryptionType(wifi_auth_mode_t encryptionType);
void redirect_root();
void handle_root();
void handle_deauth();
void handle_deauth_all();
void handle_rescan();
void handle_stop();
void start_web_interface();
void web_interface_handle_client();


void redirect_root()
{
    server.sendHeader("Location", "/");
    server.send(301);
}

// Función principal con la interfaz rediseñada
void handle_root()
{
    String html = R"(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 WiFi Auditor - Panel de Control</title>
    <style>
        /* Variables y Reset Básico */
        :root {
            --primary-color: #007bff;      /* Azul Primario */
            --secondary-color: #6c757d;    /* Gris Secundario */
            --success-color: #28a745;      /* Verde Éxito */
            --warning-color: #ffc107;      /* Amarillo Advertencia */
            --danger-color: #dc3545;       /* Rojo Peligro */
            --background-light: #f8f9fa;   /* Fondo Claro */
            --background-dark: #ffffff;    /* Fondo de Componente */
            --text-color: #343a40;         /* Texto Principal */
            --border-color: #dee2e6;       /* Color de Borde */
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Arial', sans-serif;
            background: var(--background-light);
            min-height: 100vh;
            color: var(--text-color);
            line-height: 1.6;
            padding: 20px;
        }
        
        .container {
            max-width: 1000px;
            margin: 0 auto;
        }
        
        /* Encabezado */
        .header {
            text-align: center;
            background: var(--background-dark);
            border-radius: 12px;
            padding: 30px;
            margin-bottom: 30px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
        
        .header h1 {
            font-size: 2rem;
            color: var(--primary-color);
            margin-bottom: 5px;
            font-weight: bold;
        }
        
        .subtitle {
            color: var(--secondary-color);
            font-size: 1rem;
            font-weight: 400;
        }
        
        /* Tarjetas de Estadísticas */
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .stat-card {
            background: var(--background-dark);
            border-radius: 12px;
            padding: 20px;
            text-align: center;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.05);
            border-left: 5px solid var(--primary-color);
        }
        
        .stat-number {
            font-size: 2.2rem;
            font-weight: bold;
            color: var(--primary-color);
            display: block;
            margin-bottom: 5px;
        }
        
        .stat-label {
            color: var(--secondary-color);
            font-size: 0.9rem;
            font-weight: 500;
        }
        
        /* Sección Principal */
        .section {
            background: var(--background-dark);
            border-radius: 12px;
            padding: 25px;
            margin-bottom: 30px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
        
        .section-title {
            font-size: 1.5rem;
            color: var(--text-color);
            margin-bottom: 20px;
            font-weight: 600;
            border-bottom: 2px solid var(--border-color);
            padding-bottom: 10px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        .section-icon {
            color: var(--primary-color);
            font-size: 1.5rem;
        }
        
        /* Tabla de Redes */
        .networks-table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 20px;
            border-radius: 8px;
            overflow: hidden;
        }
        
        .networks-table th {
            background-color: var(--primary-color);
            color: white;
            padding: 12px 15px;
            text-align: left;
            font-weight: 600;
            font-size: 0.9rem;
        }
        
        .networks-table td {
            padding: 12px 15px;
            border-bottom: 1px solid var(--border-color);
            font-size: 0.9rem;
        }
        
        .networks-table tr:nth-child(even) {
            background-color: #f8f9fa;
        }
        
        .networks-table tr:hover {
            background-color: #e9ecef;
        }
        
        /* Estilos de Señal y Seguridad */
        .signal-strength {
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 0.8rem;
            font-weight: 600;
        }
        
        .signal-excellent { background: var(--success-color); color: white; }
        .signal-good { background: #ffc107; color: #343a40; } /* Amarillo para bueno */
        .signal-fair { background: #fd7e14; color: white; } /* Naranja para regular */
        .signal-poor { background: var(--danger-color); color: white; }
        
        .encryption-badge {
            display: inline-block;
            padding: 4px 8px;
            border-radius: 4px;
            font-size: 0.8rem;
            font-weight: 600;
            background: #e9f0ff;
            color: var(--primary-color);
        }
        
        /* Paneles de Control */
        .control-panel {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        
        .control-card {
            background: var(--background-dark);
            border-radius: 12px;
            padding: 25px;
            box-shadow: 0 4px 12px rgba(0, 0, 0, 0.1);
        }
        
        .control-card h3 {
            color: var(--text-color);
            margin-bottom: 15px;
            font-size: 1.2rem;
            font-weight: 600;
            border-bottom: 1px dashed var(--border-color);
            padding-bottom: 8px;
        }
        
        .input-group {
            margin-bottom: 15px;
        }
        
        .input-group label {
            display: block;
            margin-bottom: 5px;
            color: var(--text-color);
            font-weight: 500;
            font-size: 0.9rem;
        }
        
        .form-input {
            width: 100%;
            padding: 10px 12px;
            border: 1px solid var(--border-color);
            border-radius: 6px;
            font-size: 1rem;
            transition: border-color 0.3s ease;
            background: #ffffff;
        }
        
        .form-input:focus {
            outline: none;
            border-color: var(--primary-color);
            box-shadow: 0 0 0 0.2rem rgba(0, 123, 255, 0.25);
        }
        
        /* Botones */
        .btn {
            width: 100%;
            padding: 10px 15px;
            border: none;
            border-radius: 6px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            margin-top: 10px;
        }
        
        .btn-primary {
            background-color: var(--success-color);
            color: white;
        }
        
        .btn-primary:hover {
            background-color: #218838;
            box-shadow: 0 2px 4px rgba(40, 167, 69, 0.5);
        }

        .btn-warning {
            background-color: var(--warning-color);
            color: #343a40;
        }
        
        .btn-warning:hover {
            background-color: #e0a800;
            box-shadow: 0 2px 4px rgba(255, 193, 7, 0.5);
        }
        
        .btn-danger {
            background-color: var(--danger-color);
            color: white;
        }
        
        .btn-danger:hover {
            background-color: #c82333;
            box-shadow: 0 2px 4px rgba(220, 53, 69, 0.5);
        }
        
        .btn-scan {
            background-color: var(--primary-color);
            color: white;
        }
        
        .btn-scan:hover {
            background-color: #0056b3;
            box-shadow: 0 2px 4px rgba(0, 123, 255, 0.5);
        }

        /* Códigos de Desautenticación */
        .codes-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        
        .code-card {
            background: #f8f9fa;
            border-left: 4px solid var(--primary-color);
            border-radius: 6px;
            padding: 15px;
            transition: background 0.2s ease;
        }
        
        .code-card:hover {
            background: #e9ecef;
        }
        
        .code-number {
            font-size: 1.2rem;
            font-weight: bold;
            color: var(--primary-color);
            margin-bottom: 5px;
        }
        
        .code-title {
            font-weight: 600;
            color: var(--text-color);
            margin-bottom: 5px;
            font-size: 0.95rem;
        }
        
        .code-description {
            font-size: 0.85rem;
            color: var(--secondary-color);
        }
        
        /* Animación */
        .pulse {
            animation: pulse 1.5s infinite;
        }
        
        @keyframes pulse {
            0% { opacity: 1; }
            50% { opacity: 0.7; }
            100% { opacity: 1; }
        }
        
        @media (max-width: 768px) {
            .header h1 { font-size: 1.5rem; }
            .stat-number { font-size: 1.8rem; }
            .control-panel, .codes-grid { grid-template-columns: 1fr; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>ESP32 WiFi Auditor</h1>
            <p class="subtitle">Herramienta de Auditoría y Testing de Redes Inalámbricas</p>
        </div>
        
        <div class="stats-grid">
            <div class="stat-card">
                <span class="stat-number">)" + String(num_networks) + R"(</span>
                <div class="stat-label">Redes Detectadas</div>
            </div>
            <div class="stat-card">
                <span class="stat-number">)" + String(eliminated_stations) + R"(</span>
                <div class="stat-label">Estaciones Afectadas</div>
            </div>
        </div>
        
        <div class="section networks-section">
            <h2 class="section-title">
                <span class="section-icon">📡</span>
                Redes WiFi Disponibles
            </h2>
            <table class="networks-table">
                <thead>
                    <tr>
                        <th>ID</th>
                        <th>SSID</th>
                        <th>BSSID</th>
                        <th>Canal</th>
                        <th>Señal</th>
                        <th>Seguridad</th>
                    </tr>
                </thead>
                <tbody>
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
                </tbody>
            </table>
            
            <form method="post" action="/rescan">
                <button type="submit" class="btn btn-scan">🔄 Escanear Redes WiFi</button>
            </form>
        </div>

        <div class="control-panel">
            <div class="control-card">
                <h3>🎯 Ataque Dirigido (Deauth)</h3>
                <form method="post" action="/deauth">
                    <div class="input-group">
                        <label for="net_num">ID de Red Objetivo</label>
                        <input type="number" name="net_num" id="net_num" class="form-input" placeholder="Ej: 0, 1, 2..." required min="0" max=")" + String(num_networks - 1) + R"(">
                    </div>
                    <div class="input-group">
                        <label for="reason">Código de Razón (Ver lista inferior)</label>
                        <input type="number" name="reason" id="reason" class="form-input" placeholder="Ej: 1, 2, 3..." required min="1">
                    </div>
                    <button type="submit" class="btn btn-warning">Iniciar Ataque Dirigido</button>
                </form>
            </div>

            <div class="control-card">
                <h3>🔥 Ataque Masivo (Broadcast)</h3>
                <form method="post" action="/deauth_all">
                    <div class="input-group">
                        <label for="reason_all">Código de Razón (Ver lista inferior)</label>
                        <input type="number" name="reason" id="reason_all" class="form-input" placeholder="Ej: 1, 2, 3..." required min="1">
                    </div>
                    <button type="submit" class="btn btn-danger pulse">Atacar Todas las Redes</button>
                    <p style="font-size: 0.8rem; color: var(--danger-color); margin-top: 10px; font-weight: 500;">
                        ⚠️ Advertencia: Este modo detendrá el WiFi del dispositivo y afectará a TODAS las redes cercanas.
                    </p>
                </form>
            </div>
        </div>
        
        <div class="section" style="text-align: center; padding: 20px;">
            <form method="post" action="/stop">
                <button type="submit" class="btn btn-primary">🛑 Detener Todos los Ataques Activos</button>
            </form>
        </div>

        <div class="section codes-section">
            <h2 class="section-title">
                <span class="section-icon">📜</span>
                Códigos de Razón de Desautenticación 802.11
            </h2>
            <div class="codes-grid">
                <div class="code-card">
                    <div class="code-number">1</div>
                    <div class="code-title">Razón No Especificada</div>
                    <div class="code-description">Desconexión por razón desconocida o no especificada.</div>
                </div>
                <div class="code-card">
                    <div class="code-number">2</div>
                    <div class="code-title">Autenticación Inválida</div>
                    <div class="code-description">La autenticación previa ya no es válida.</div>
                </div>
                <div class="code-card">
                    <div class="code-number">3</div>
                    <div class="code-title">Estación Saliendo (Deauth)</div>
                    <div class="code-description">La estación se desconecta porque está saliendo del BSS.</div>
                </div>
                <div class="code-card">
                    <div class="code-number">4</div>
                    <div class="code-title">Inactividad</div>
                    <div class="code-description">Desconexión por periodo prolongado de inactividad.</div>
                </div>
                <div class="code-card">
                    <div class="code-number">8</div>
                    <div class="code-title">Estación Saliendo (Disass)</div>
                    <div class="code-description">Estación disociada porque está saliendo del BSS.</div>
                </div>
                <div class="code-card">
                    <div class="code-number">14</div>
                    <div class="code-title">Error MIC (WPA)</div>
                    <div class="code-description">Fallo en el código de integridad del mensaje (WPA/WPA2).</div>
                </div>
                <div class="code-card">
                    <div class="code-number">15</div>
                    <div class="code-title">Timeout 4-Way (WPA)</div>
                    <div class="code-description">Tiempo agotado durante el handshake de 4 vías (WPA/WPA2).</div>
                </div>
                <div class="code-card">
                    <div class="code-number">23</div>
                    <div class="code-title">802.1X Fallo</div>
                    <div class="code-description">Fallo en la autenticación IEEE 802.1X.</div>
                </div>
            </div>
        </div>
    </div>
</body>
</html>
)";

    server.send(200, "text/html", html);
}

// Función de manejo de ataque dirigido (Rediseñada)
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
    <title>Estado del Ataque - ESP32 Auditor</title>
    <style>
        :root {
            --primary-color: #007bff;
            --success-color: #28a745;
            --danger-color: #dc3545;
            --warning-color: #ffc107;
            --background-light: #f8f9fa;
            --background-dark: #ffffff;
            --text-color: #343a40;
        }
        
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, var(--primary-color) 0%, #0056b3 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: var(--text-color);
        }
        
        .alert-container {
            background: var(--background-dark);
            border-radius: 12px;
            padding: 40px;
            text-align: center;
            max-width: 500px;
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.2);
            animation: slideIn 0.5s ease-out;
        }
        
        @keyframes slideIn {
            from { transform: translateY(-30px); opacity: 0; }
            to { transform: translateY(0); opacity: 1; }
        }
        
        .icon {
            width: 80px;
            height: 80px;
            border-radius: 50%;
            margin: 0 auto 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 40px;
            color: white;
            font-weight: bold;
        }

        .success-icon { 
            background: var(--success-color);
            box-shadow: 0 0 0 0 rgba(40, 167, 69, 0.4);
            animation: pulse 2s infinite;
        }
        
        .error-icon { 
            background: var(--danger-color);
            animation: shake 0.5s ease-in-out;
        }
        
        @keyframes pulse {
            0%, 100% { box-shadow: 0 0 0 0 rgba(40, 167, 69, 0.4); }
            50% { box-shadow: 0 0 0 15px rgba(40, 167, 69, 0); }
        }
        
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-5px); }
            75% { transform: translateX(5px); }
        }
        
        .alert-title {
            font-size: 1.8rem;
            font-weight: bold;
            margin-bottom: 10px;
            color: var(--text-color);
        }
        
        .alert-message {
            font-size: 1rem;
            margin-bottom: 10px;
            color: #6c757d;
        }
        
        .detail-info {
            background: var(--background-light);
            border-radius: 8px;
            padding: 15px;
            margin: 15px 0;
            border-left: 4px solid var(--primary-color);
            text-align: left;
        }
        
        .detail-label {
            font-weight: 500;
            color: var(--primary-color);
            margin-bottom: 3px;
            font-size: 0.9rem;
        }
        
        .detail-value {
            font-size: 1.1rem;
            font-weight: bold;
            color: var(--text-color);
        }
        
        .btn-home {
            display: inline-block;
            background-color: var(--primary-color);
            color: white;
            padding: 10px 20px;
            text-decoration: none;
            border-radius: 6px;
            font-weight: 600;
            margin-top: 20px;
            transition: background-color 0.3s ease;
        }
        
        .btn-home:hover {
            background-color: #0056b3;
        }
        
        .warning-text {
            font-size: 0.9rem;
            color: #856404;
            margin-top: 15px;
            font-style: italic;
            background-color: #fff3cd;
            padding: 10px;
            border-radius: 6px;
            border: 1px solid var(--warning-color);
        }
    </style>
</head>
<body>
    <div class="alert-container)";

    if (wifi_number < num_networks)
    {
        html += R"(">
        <div class="icon success-icon">✅</div>
        <h2 class="alert-title">Ataque Dirigido Iniciado</h2>
        <p class="alert-message">La solicitud de desautenticación ha sido enviada.</p>
        
        <div class="detail-info">
            <div class="detail-label">Red Objetivo:</div>
            <div class="detail-value">ID #)" + String(wifi_number) + " (" + WiFi.SSID(wifi_number) + R"()</div>
        </div>
        
        <div class="detail-info">
            <div class="detail-label">Código de Razón:</div>
            <div class="detail-value">)" + String(reason) + R"(</div>
        </div>
        
        <p class="warning-text">El proceso está activo. Usa el botón 'Detener' en el panel principal para finalizar.</p>
        )";
        start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
    }
    else
    {
        html += R"(">
        <div class="icon error-icon">❌</div>
        <h2 class="alert-title">Error de Operación</h2>
        <p class="alert-message">El ID de red WiFi proporcionado es inválido.</p>
        
        <div class="detail-info" style="border-left: 4px solid var(--danger-color);">
            <div class="detail-label">ID Solicitado:</div>
            <div class="detail-value">#)" + String(wifi_number) + R"(</div>
        </div>
        
        <p class="alert-message">Por favor, regresa y selecciona un ID de red válido del listado (0 a )" + String(num_networks - 1) + R"().</p>
        )";
    }

    html += R"(
        <a href="/" class="btn-home">⬅️ Regresar al Panel Principal</a>
    </div>
</body>
</html>
)";

    server.send(200, "text/html", html);
}

// Función de manejo de ataque masivo (Rediseñada)
void handle_deauth_all()
{
    uint16_t reason = server.arg("reason").toInt();

    String html = R"(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ataque Masivo - ESP32 Auditor</title>
    <style>
        :root {
            --primary-color: #007bff;
            --danger-color: #dc3545;
            --warning-color: #ffc107;
            --background-dark: #ffffff;
            --text-color: #343a40;
        }
        
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, var(--danger-color) 0%, #a71d2a 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: var(--text-color);
        }
        
        .warning-container {
            background: var(--background-dark);
            border-radius: 12px;
            padding: 40px;
            text-align: center;
            max-width: 500px;
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.3);
            border: 5px solid var(--danger-color);
            animation: dangerPulse 2s infinite;
        }
        
        @keyframes dangerPulse {
            0%, 100% { transform: scale(1); }
            50% { transform: scale(1.01); }
        }
        
        .danger-icon {
            width: 90px;
            height: 90px;
            background: var(--danger-color);
            border-radius: 50%;
            margin: 0 auto 20px;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 50px;
            color: white;
            animation: shake 0.5s ease-in-out;
        }
        
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            25% { transform: translateX(-10px); }
            75% { transform: translateX(10px); }
        }
        
        .warning-title {
            font-size: 2rem;
            font-weight: bold;
            margin-bottom: 10px;
            color: var(--danger-color);
        }
        
        .warning-message {
            font-size: 1.1rem;
            margin-bottom: 20px;
            color: var(--text-color);
            font-weight: 500;
        }
        
        .critical-warning {
            background: #fff3cd;
            color: #856404;
            padding: 15px;
            border-radius: 8px;
            margin: 15px 0;
            font-weight: 600;
            border: 2px solid var(--warning-color);
        }
        
        .reason-info {
            background: #f8f9fa;
            border-radius: 8px;
            padding: 15px;
            margin: 15px 0;
            border-left: 4px solid var(--danger-color);
        }
        
        .reason-label {
            font-weight: 500;
            color: var(--danger-color);
            margin-bottom: 3px;
        }
        
        .reason-value {
            font-size: 1.5rem;
            font-weight: bold;
            color: var(--text-color);
        }
        
        .status-text {
            font-size: 1rem;
            color: #6c757d;
            margin-top: 15px;
            font-style: italic;
        }
        
        .security-note {
            margin-top: 30px; 
            padding: 15px; 
            background: #f8d7da; 
            border-radius: 10px; 
            color: #721c24;
            border: 1px solid #f5c6cb;
            font-size: 0.9rem;
        }
    </style>
</head>
<body>
    <div class="warning-container">
        <div class="danger-icon">🚨</div>
        <h1 class="warning-title">Ataque Masivo: En Progreso</h1>
        <p class="warning-message">Se ha iniciado un ataque de desautenticación en modo broadcast.</p>
        
        <div class="critical-warning">
            ⚠️ **Conexión Pérdida:** La interfaz web ya no está disponible.
        </div>
        
        <div class="reason-info">
            <div class="reason-label">Código de Razón Utilizado:</div>
            <div class="reason-value">)" + String(reason) + R"(</div>
        </div>
        
        <p class="status-text">Para detener el ataque, debes reiniciar físicamente el dispositivo ESP32.</p>
        
        <div class="security-note">
            **Nota de Auditoría:** Este ataque está diseñado para propósitos de prueba de seguridad y puede causar una interrupción significativa en la conectividad del área. Úsalo solo en entornos de prueba autorizados.
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