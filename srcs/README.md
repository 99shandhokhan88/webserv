# 📁 Directory `srcs/` - Codice Sorgente

## 🎯 **Panoramica**

Questa directory contiene tutto il codice sorgente del webserver, organizzato in moduli logici per facilitare la manutenzione e la comprensione.

## 📂 **Struttura delle Directory**

```
srcs/
├── main.cpp              # ✅ Punto di ingresso del programma
├── Core/                 # 🏗️ Componenti principali del server
│   ├── incs/            # Header files del core
│   │   ├── Server.hpp   # ✅ Classe principale del server
│   │   └── Client.hpp   # ✅ Gestione stato client
│   └── srcs/            # Implementazioni del core
│       ├── Server.cpp   # ✅ Logica principale del server
│       ├── Client.cpp   # Gestione client
│       └── EventLoop.cpp # Loop degli eventi
├── HTTP/                # 🌐 Gestione protocollo HTTP
│   ├── incs/            # Header HTTP
│   └── srcs/            # Implementazioni HTTP
├── Config/              # ⚙️ Sistema di configurazione
│   ├── incs/            # Header configurazione
│   └── srcs/            # Parser configurazione
├── CGI/                 # 🔧 Esecuzione script CGI
│   ├── incs/            # Header CGI
│   └── srcs/            # Executor CGI
├── Utils/               # 🛠️ Utilità e helper
│   ├── incs/            # Header utilità
│   └── srcs/            # Implementazioni utilità
└── Errors/              # ❌ Gestione errori
    ├── incs/            # Header errori
    └── srcs/            # Handler errori
```

## 🔍 **Descrizione dei Moduli**

### **📄 main.cpp**
- **Scopo**: Punto di ingresso del programma
- **Responsabilità**: 
  - Validazione argomenti
  - Caricamento configurazione
  - Inizializzazione e avvio server
- **Stato**: ✅ Completamente documentato

### **🏗️ Core/**
**Componenti fondamentali del server**

- **Server.hpp/cpp**: Classe principale che gestisce:
  - Socket server e binding
  - Loop di polling per I/O asincrono
  - Accettazione connessioni
  - Routing richieste HTTP
  - Gestione multi-client

- **Client.hpp/cpp**: Gestione stato client:
  - Buffer dati ricevuti/da inviare
  - Parsing richieste HTTP
  - Keep-alive connections
  - Interfaccia con il server

- **EventLoop.cpp**: Sistema di eventi:
  - Polling con poll()/epoll
  - Gestione timeout
  - Multiplexing I/O

### **🌐 HTTP/**
**Implementazione protocollo HTTP/1.1**

- **Request**: Parsing e rappresentazione richieste HTTP
- **Response**: Generazione risposte HTTP
- **RequestParser**: Parser robusto per HTTP
- **ResponseGenerator**: Costruzione risposte conformi

### **⚙️ Config/**
**Sistema di configurazione flessibile**

- **ServerConfig**: Configurazione server principale
- **LocationConfig**: Configurazione per location specifiche
- **ConfigParser**: Parser file di configurazione

### **🔧 CGI/**
**Supporto Common Gateway Interface**

- **CGIExecutor**: Esecuzione script CGI
- Supporto per Python, PHP, Bash
- Gestione environment variables
- Pipe per comunicazione

### **🛠️ Utils/**
**Utilità e funzioni helper**

- **FileHandler**: Operazioni su file
- **StringUtils**: Manipolazione stringhe
- **MimeTypes**: Riconoscimento tipi MIME

### **❌ Errors/**
**Sistema di gestione errori**

- **ErrorHandler**: Gestione centralizzata errori
- Pagine di errore personalizzate
- Logging errori

## 🔄 **Flusso di Esecuzione**

### **1. Avvio**
```
main.cpp → ServerConfig → Server::constructor → setupSocket()
```

### **2. Loop Principale**
```
Server::run() → poll() → handleEvents() → processRequests()
```

### **3. Gestione Richiesta**
```
accept() → Client → RequestParser → Router → Handler → Response
```

## 🛠️ **Convenzioni di Codice**

### **Naming**
- **Classi**: PascalCase (es. `ServerConfig`)
- **Metodi**: camelCase (es. `handleRequest`)
- **Variabili**: snake_case (es. `client_fd`)
- **Costanti**: UPPER_CASE (es. `MAX_CLIENTS`)

### **File Organization**
- **Header**: `.hpp` in directory `incs/`
- **Implementation**: `.cpp` in directory `srcs/`
- **Include guards**: `#ifndef CLASS_HPP`

### **Documentazione**
- **Doxygen style**: `@brief`, `@param`, `@return`
- **Commenti in italiano** per spiegazioni dettagliate
- **Esempi pratici** nei commenti quando utile

## 📊 **Statistiche Attuali**

- **File documentati**: 3/15+ file principali
- **Righe di codice**: ~5000+ righe
- **Moduli**: 6 moduli principali
- **Stato documentazione**: 🔄 In corso

## 🚀 **Come Contribuire**

### **Per Aggiungere Funzionalità**
1. Identifica il modulo appropriato
2. Aggiungi header in `incs/`
3. Implementa in `srcs/`
4. Documenta con commenti dettagliati
5. Testa la funzionalità

### **Per Debug**
1. Controlla i log del server
2. Usa i commenti per capire il flusso
3. Verifica la documentazione del modulo
4. Consulta `GUIDA_RAPIDA_ARCHITETTURA.md`

## 📚 **Risorse Utili**

- **Architettura generale**: `../GUIDA_RAPIDA_ARCHITETTURA.md`
- **Piano miglioramenti**: `../ANALISI_E_MIGLIORAMENTI.md`
- **Test**: `../TEST_README.md`
- **Configurazione**: `../configs/`

---

**💡 Suggerimento**: Inizia sempre leggendo i commenti nei file `.hpp` per capire l'interfaccia, poi passa ai `.cpp` per i dettagli implementativi! 