#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/utsname.h>

int main() {
    // Invia gli header HTTP
    printf("Content-type: text/html\r\n\r\n");
    
    // Ottieni informazioni sul sistema
    struct utsname sysinfo;
    uname(&sysinfo);
    
    // Ottieni la data e l'ora correnti
    time_t now = time(NULL);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Output HTML
    printf("<!DOCTYPE html>\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("    <title>C CGI Info Test</title>\n");
    printf("    <style>\n");
    printf("        body { font-family: Arial, sans-serif; margin: 20px; }\n");
    printf("        h1 { color: #00BCD4; }\n");
    printf("        .info { background-color: #e0f7fa; padding: 15px; border-radius: 5px; margin: 10px 0; }\n");
    printf("        .success { background-color: #e8f5e9; border-left: 5px solid #4CAF50; padding: 10px; }\n");
    printf("        table { border-collapse: collapse; width: 100%%; }\n");
    printf("        th, td { text-align: left; padding: 8px; border-bottom: 1px solid #ddd; }\n");
    printf("        th { background-color: #f2f2f2; }\n");
    printf("    </style>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("    <h1>C CGI Test - Multi-Language CGI Implementation</h1>\n");
    
    printf("    <div class=\"success\">\n");
    printf("        <p>✅ La CGI multi-linguaggio è configurata correttamente! Il programma C è stato eseguito con successo.</p>\n");
    printf("    </div>\n");

    printf("    <h2>Informazioni di sistema</h2>\n");
    printf("    <div class=\"info\">\n");
    printf("        <p>Sistema: %s %s</p>\n", sysinfo.sysname, sysinfo.release);
    printf("        <p>Versione: %s</p>\n", sysinfo.version);
    printf("        <p>Macchina: %s</p>\n", sysinfo.machine);
    printf("        <p>Data e ora: %s</p>\n", time_str);
    printf("    </div>\n");

    printf("    <h2>Variabili d'ambiente CGI</h2>\n");
    printf("    <table>\n");
    printf("        <tr>\n");
    printf("            <th>Nome</th>\n");
    printf("            <th>Valore</th>\n");
    printf("        </tr>\n");

    // Stampa le variabili d'ambiente CGI comuni
    const char* env_vars[] = {
        "REQUEST_METHOD", "QUERY_STRING", "CONTENT_TYPE", "CONTENT_LENGTH",
        "SERVER_SOFTWARE", "SERVER_NAME", "SERVER_PROTOCOL", "GATEWAY_INTERFACE",
        "REMOTE_ADDR", "REMOTE_HOST", "PATH_INFO", "PATH_TRANSLATED",
        "SCRIPT_NAME", "SCRIPT_FILENAME", "HTTP_USER_AGENT", "HTTP_REFERER",
        NULL
    };

    for (int i = 0; env_vars[i] != NULL; i++) {
        const char* value = getenv(env_vars[i]);
        if (value) {
            printf("        <tr><td>%s</td><td>%s</td></tr>\n", env_vars[i], value);
        }
    }

    printf("    </table>\n");
    
    printf("    <h2>Verifica interprete</h2>\n");
    printf("    <div class=\"info\">\n");
    printf("        <p>Questo programma C è stato compilato come CGI ed eseguito direttamente dal server web.</p>\n");
    printf("        <p>PID del processo: %d</p>\n", getpid());
    
    // Ottieni il percorso dello script
    const char* script_path = getenv("SCRIPT_FILENAME");
    if (script_path) {
        printf("        <p>Script path: <code>%s</code></p>\n", script_path);
    }
    
    printf("    </div>\n");

    printf("    <p><a href=\"/cgi.html\">Torna alla pagina CGI</a></p>\n");
    printf("</body>\n");
    printf("</html>\n");

    return 0;
} 