# 🏗️ Guida Rapida all'Architettura del Webserver

## 🎯 **Panoramica Generale**

Il webserver è un server HTTP multi-client che gestisce richieste simultanee usando I/O non-bloccante e polling.

### **Flusso Principale di Esecuzione**

```
1. main() → Carica configurazione
2. Server() → Inizializza socket
3. run() → Loop infinito di polling
4. accept() → Nuove connessioni
5. handleClient() → Gestisce richieste
6. processRequest() → Elabora e risponde
```

## 🔄 **Ciclo di Vita di una Richiesta**

### **1. Avvio del Server**
```cpp
// main.cpp
ServerConfig config("config.conf");  // Carica configurazione
Server server(config);               // Crea server
server.run();                        // Avvia loop principale
```

### **2. Inizializzazione Socket**
```cpp
// Server::setupSocket()
socket() → setsockopt() → bind() → listen() → fcntl(O_NONBLOCK)
```

### **3. Loop Principale**
```cpp
// Server::run()
while (true) {
    poll(fds, timeout);              // Aspetta eventi I/O
    
    for (ogni fd con eventi) {
        if (è server_fd) {
            acceptNewConnection();    // Nuova connessione
        } else {
            handleClient(fd);        // Gestisci client esistente
        }
    }
}
```

### **4. Gestione Client**
```cpp
// Server::handleClient()
recv() → appendRequestData() → isRequestComplete() → parseRequest() → processRequest()
```

### **5. Elaborazione Richiesta**
```cpp
// Server::processRequest()
switch (method) {
    case GET:    handleGetRequest();
    case POST:   handlePostRequest();
    case DELETE: handleDeleteRequest();
    case OPTIONS: handleOptionsRequest();
}
```

## 🏛️ **Architettura dei Componenti**

### **Componenti Principali**

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     main.cpp    │───▶│  ServerConfig   │───▶│     Server      │
│  (Entry Point)  │    │ (Configuration) │    │ (Main Logic)    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
                                                        │
                                                        ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│     Client      │◀───│   EventLoop     │◀───│   Poll System   │
│ (Client State)  │    │ (I/O Management)│    │ (File Descriptors)│
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │
         ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│    Request      │───▶│ RequestParser   │───▶│    Response     │
│ (HTTP Request)  │    │ (HTTP Parsing)  │    │ (HTTP Response) │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### **Gestione Multi-Client**

```
Server (Singleton Pattern)
├── Static Members:
│   ├── servers[]     → Lista di tutti i server
│   ├── poll_fds[]    → File descriptors per polling
│   └── clients{}     → Mappa fd → Client
│
└── Instance Members:
    ├── config        → Configurazione specifica
    ├── server_fd     → Socket del server
    └── address       → Indirizzo/porta
```

## 📊 **Flusso dei Dati**

### **Richiesta HTTP**
```
Client Browser → Socket → recv() → Buffer → RequestParser → Request Object
```

### **Elaborazione**
```
Request Object → Router → Handler (GET/POST/DELETE) → Business Logic
```

### **Risposta HTTP**
```
Content → Response Object → ResponseGenerator → send() → Socket → Client Browser
```

## 🔧 **Gestione degli Errori**

### **Livelli di Gestione**
1. **Socket Level**: Errori di rete, connessioni chiuse
2. **HTTP Level**: Richieste malformate, metodi non supportati
3. **Application Level**: File non trovati, permessi negati
4. **System Level**: Memoria insufficiente, file descriptor esauriti

### **Strategia di Recovery**
```cpp
try {
    // Operazione normale
} catch (const std::exception& e) {
    sendErrorResponse(client, errorCode, message);
    removeClient(client_fd);  // Cleanup
}
```

## 🚀 **Caratteristiche Avanzate**

### **CGI Support**
```
Request → isCgiRequest() → CGIExecutor → fork() → exec() → pipe() → Response
```

### **File Upload**
```
POST → parseMultipartBody() → extractBoundary() → saveFile() → Response
```

### **Directory Listing**
```
GET /dir/ → handleDirectoryListing() → generateHTML() → Response
```

### **Keep-Alive**
```
Connection: keep-alive → client.shouldKeepAlive() → mantieni connessione
```

## 📝 **Pattern di Design Utilizzati**

### **1. Singleton Pattern**
- `Server::servers` - Gestione globale dei server

### **2. Factory Pattern**
- Creazione di oggetti `Request` e `Response`

### **3. Strategy Pattern**
- Diversi handler per metodi HTTP

### **4. Observer Pattern**
- Sistema di polling per eventi I/O

## 🔍 **Debug e Monitoring**

### **Log Points Importanti**
```cpp
std::cout << "Server started on port " << port << std::endl;
std::cout << "New client connected: " << client_fd << std::endl;
std::cout << "Request: " << method << " " << path << std::endl;
std::cerr << "Error: " << e.what() << std::endl;
```

### **Metriche da Monitorare**
- Numero di client connessi
- Richieste per secondo
- Tempo di risposta medio
- Errori per tipo

## 🎯 **Punti di Estensione**

### **Facili da Aggiungere**
- Nuovi metodi HTTP
- Nuovi tipi MIME
- Pagine di errore personalizzate
- Middleware di logging

### **Modifiche Più Complesse**
- HTTPS/SSL support
- Autenticazione
- Load balancing
- Caching

---

## 📚 **Per Approfondire**

1. **Leggi i commenti** in `Server.hpp` per l'interfaccia completa
2. **Studia** `main.cpp` per il punto di ingresso
3. **Analizza** `Server::run()` per il loop principale
4. **Esamina** i metodi `handle*Request()` per la logica specifica

**Questa guida ti dà una visione d'insieme. Per i dettagli implementativi, consulta i file sorgente documentati!** 