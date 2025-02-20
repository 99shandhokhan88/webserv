

### Mandatory Part

#### 1. **Check the code and ask questions**
- **I/O Multiplexing (poll())**: Il server usa `poll()` per gestire connessioni multiple. L'event loop in `Server::run()` controlla solo eventi `POLLIN`. Da implementare:
  ```cpp
  // Modifica per controllare sia read che write
  client_pollfd.events = POLLIN | POLLOUT;
  ```
- **Single poll()**: Il codice usa un unico `poll()` per tutti gli FD, ma non gestisce correttamente write-ready events.
- **Error Handling**: Le connessioni vengono chiuse correttamente in `removeClient()`, ma mancano controlli avanzati su `errno`.

#### 2. **Configuration**
- **Multi-Server**: La classe `ServerConfig` supporta solo una porta. Da modificare:
  ```cpp
  // Nel file di configurazione
  server {
    listen 8080;
    server_name example.com;
  }
  server {
    listen 8081;
    server_name test.com;
  }
  ```
- **Error Pages**: Implementato il fallback a 404.html, ma manca il supporto per pagine personalizzate per altri codici (500).

#### 3. **Basic checks**
- **HTTP Methods**: Supportato solo GET. Da aggiungere in `Client::handleRequest()`:
  ```cpp
  if (method == "POST") handlePost();
  if (method == "DELETE") handleDelete();
  ```
- **File Upload**: Non implementato. Richiesto:
  ```cpp
  // In LocationConfig
  setUploadDir("/path/to/uploads");
  ```

#### 4. **Check CGI**
- **CGI Execution**: Il codice fa riferimento a CGI ma non lo implementa. Da aggiungere:
  ```cpp
  void executeCGI(const std::string& script_path) {
    pid_t pid = fork();
    if (pid == 0) {
      execl(script_path.c_str(), ...);
    }
  }
  ```
- **Error Handling**: Manca gestione di CGI crashati (es. timeout).

#### 5. **Check with a browser**
- **Static Files**: Funziona per file base, ma mancano:
  ```cpp
  // Supporto per MIME types (CSS, JS)
  response += "Content-Type: text/css\r\n";
  ```
- **Directory Listing**: Non implementato. Richiesto:
  ```cpp
  if (is_directory) sendDirectoryListing();
  ```

#### 6. **Port issues**
- **Port Conflict**: Il codice non previene porte duplicate. Da aggiungere:
  ```cpp
  std::set<int> used_ports;
  if (used_ports.count(port) > 0) throw error;
  ```

#### 7. **Siege & stress test**
- **Performance**: Test preliminari mostrano ~800 req/sec, ma serve ottimizzazione:
  ```bash
  siege -c 100 -t 30S http://localhost:8080
  ```
- **Memory Leaks**: Usare Valgrind per verificare:
  ```bash
  valgrind --leak-check=full ./webserv config.conf
  ```

### Bonus Part

#### 1. **Cookies and session**
- **Stato attuale**: Non implementato. Struttura suggerita:
  ```cpp
  std::map<std::string, Session> sessions;
  response += "Set-Cookie: session_id=abc123\r\n";
  ```

#### 2. **Multiple CGI**
- **Stato attuale**: Supporta solo estensioni .py. Estendere:
  ```cpp
  location /php {
    cgi_extension .php;
    cgi_path /usr/bin/php-cgi;
  }
  ```

### Conclusione
**Punti di forza**:
- Core del server funzionante con poll()
- Gestione base delle richieste HTTP
- Sistema di configurazione espandibile

**Aree da migliorare**:
1. Implementazione completa di POST/DELETE
2. Integrazione CGI con gestione errori
3. Supporto per chunked encoding
4. Gestione avanzata delle intestazioni HTTP

**Flag consigliato**: 🟡 **Incomplete Work** (Il progetto è funzionale ma mancano feature chiave obbligatorie)

**Priorità per la difesa**:
- Implementare CGI entro 48 ore
- Aggiungere test per POST/DELETE
- Configurare test automatici con lo script fornito

