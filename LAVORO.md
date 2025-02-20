**Workflow e Divisione del Lavoro per il Progetto Webserv**

---

### **1. Venelin - Configuration Parser**
**Responsabilità:** Implementare un parser per il file di configurazione che gestisca tutte le direttive richieste dal subject.

#### **Task Specifici:**
- **Supporto per Multi-Server:** 
  - Leggere blocchi `server` multipli nel file di configurazione.
  - Ogni server deve avere: `listen` (porta), `server_name`, `root`, `index`, `error_page`, `client_max_body_size`.
- **Gestione delle Route (`location`):**
  - Parsing dei blocchi `location` con opzioni: `root`, `cgi_extension`, `cgi_path`, `allow_methods`, `autoindex`, `return` (redirect).
  - Validazione dei metodi HTTP consentiti (es. `GET`, `POST`, `DELETE`).
- **Error Handling:**
  - Caricare le pagine di errore personalizzate (es. `error_page 404 /errors/404.html`).
  - Fallback alle pagine di default se non specificate.
- **Struttura Dati:**
  - Creare classi `ServerConfig` e `LocationConfig` per memorizzare le impostazioni.
  - Esempio: `std::vector<ServerConfig> servers` per gestire più server.

#### **Integrazione con Altri Moduli:**
- Passare la configurazione parsata alla classe `Server` di Maryem per l'inizializzazione.
- Fornire a Fbiondo i dati sulle route e CGI per gestire le richieste.

---

### **2. Maryem - Socket & Server Core**
**Responsabilità:** Implementare il core del server, gestione delle connessioni e I/O multiplexing.

#### **Task Specifici:**
- **Multi-Server Setup:**
  - Creare un socket listener per ogni `server` nella configurazione (diverse porte/host).
  - Usare `poll()`/`epoll()` per monitorare tutti i socket (server + client).
- **Non-Blocking I/O:**
  - Impostare tutti i file descriptor in modalità non-blocking.
  - Gestire eventi `POLLIN` (nuove connessioni) e `POLLOUT` (risposte).
- **Accettazione Connessioni:**
  - Creare una classe `Client` per ogni connessione accettata.
  - Aggiungere il client a `poll_fds` per monitorarne le attività.
- **Integrazione con Configurazione:**
  - Usare `ServerConfig` per impostare porta, root, error pages, ecc.
  - Delegare la gestione delle richieste alla classe `Client` di Fbiondo.

#### **Esempio di Codice (Socket Setup):**
```cpp
// Per ogni server nella configurazione:
for (const auto& server_config : configs) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(server_fd, ...); // Usa server_config.getPort()
    listen(server_fd, SOMAXCONN);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    poll_fds.push_back({server_fd, POLLIN, 0});
}
```

---

### **3. Fbiondo - Request/Response Handling**
**Responsabilità:** Parsing delle richieste HTTP, generazione risposte, gestione CGI e upload.

#### **Task Specifici:**
- **HTTP Request Parsing:**
  - Estrazione di metodo, path, headers, e body (supporto per chunked encoding).
  - Validazione dei metodi consentiti per la route (da `LocationConfig`).
- **Generazione Risposte:**
  - Servire file statici (HTML, CSS, JS) dalla directory `root`.
  - Gestione errori (404, 500) con pagine custom o default.
  - Supporto per `Content-Type` dinamico (es. `text/html`, `image/png`).
- **CGI Execution:**
  - Eseguire script (es. Python, PHP) basati su `cgi_extension`.
  - Impostare variabili d'ambiente (`PATH_INFO`, `QUERY_STRING`).
  - Gestire input/output tramite pipe e `fork()`.
- **Upload File:**
  - Salvare file caricati via POST nella directory specificata nella configurazione.
- **Connessioni Persistenti:**
  - Gestire `Connection: keep-alive` e timeout.

#### **Esempio di Codice (CGI):**
```cpp
// Esegui CGI
pid_t pid = fork();
if (pid == 0) {
    dup2(cgi_input[0], STDIN_FILENO);
    dup2(cgi_output[1], STDOUT_FILENO);
    execve(cgi_path, cgi_args, environ); // cgi_path da LocationConfig
}
// Leggi output dal pipe e invia la risposta
```

---

### **Integrazione e Flusso di Lavoro**
1. **Inizializzazione:**
   - Venelin legge il file di configurazione e genera una lista di `ServerConfig`.
   - Maryem crea un socket per ogni `ServerConfig` e li aggiunge a `poll_fds`.
   - Fbiondo prepara gli handler per CGI e file statici.

2. **Event Loop:**
   - Maryem monitora eventi con `poll()`.
   - Se evento su un socket server: `accept()` nuova connessione e crea `Client`.
   - Se evento su client: delega a Fbiondo per leggere la richiesta e generare risposta.

3. **Gestione Richiesta:**
   - Fbiondo usa `ServerConfig` per trovare la route corrispondente.
   - Se è una route CGI: esegue lo script e invia l'output.
   - Se è una route static: serve il file o genera errore 404.

4. **Testing:**
   - **Configurazione:** Verifica multi-server, error pages, limiti di body.
   - **Browser:** Testare con Chrome/Firefox per verificare compatibilità.
   - **Stress Test:** Usare `siege` o `ab` per testare prestazioni.

---

### **Checklist per la Valutazione (Basata sul Evaluation Sheet)**
- ✅ **Configurazione Multi-Server:** Supporto per più porte/host.
- ✅ **CGI:** Esecuzione corretta di script con GET/POST.
- ✅ **HTTP Methods:** Supporto per GET, POST, DELETE.
- ✅ **Error Handling:** Pagine custom per 404, 500.
- ✅ **Non-Blocking I/O:** Nessun blocco durante read/write.
- ✅ **Upload File:** Salvataggio in directory specificata.
- ✅ **Stress Test:** Server rimane stabile sotto carico.

---

### **Note Importanti**
- **Coordinazione:** Usare Git per merge frequenti e risolvere conflitti.
- **Testing:** Automatizzare test con script (es. Python) per verificare ogni feature.
- **Documentazione:** Commentare il codice e mantenere un README con istruzioni.