# 📁 Directory `configs/` - Configurazione Server

## 🎯 **Panoramica**

Questa directory contiene i file di configurazione per il webserver. Il sistema di configurazione è flessibile e permette di definire server multipli, location specifiche, e comportamenti personalizzati.

## 📂 **File di Configurazione**

```
configs/
├── default.conf         # 🔧 Configurazione di default
├── multi_server.conf    # 🌐 Configurazione multi-server
├── cgi_test.conf       # 🔧 Configurazione per test CGI
├── upload_test.conf    # 📤 Configurazione per test upload
└── examples/           # 📚 Esempi di configurazione
    ├── minimal.conf    # Configurazione minimale
    ├── advanced.conf   # Configurazione avanzata
    └── ssl.conf        # Configurazione HTTPS (futuro)
```

## 🔧 **Sintassi di Configurazione**

### **Struttura Base**
```nginx
# Commenti iniziano con #
server {
    # Configurazione server
    listen 8080;
    server_name localhost;
    root www;
    
    # Configurazione location
    location / {
        # Direttive specifiche per questa location
    }
}
```

### **Direttive Supportate**

#### **🌐 Direttive Server**

| Direttiva | Descrizione | Esempio | Default |
|-----------|-------------|---------|---------|
| `listen` | Porta di ascolto | `listen 8080;` | `80` |
| `server_name` | Nome del server | `server_name example.com;` | `localhost` |
| `root` | Directory root | `root /var/www;` | `www` |
| `index` | File index | `index index.html index.php;` | `index.html` |
| `client_max_body_size` | Dimensione max body | `client_max_body_size 10M;` | `1M` |
| `error_page` | Pagine di errore | `error_page 404 /errors/404.html;` | - |

#### **📍 Direttive Location**

| Direttiva | Descrizione | Esempio | Default |
|-----------|-------------|---------|---------|
| `allow_methods` | Metodi HTTP permessi | `allow_methods GET POST;` | `GET` |
| `autoindex` | Listing directory | `autoindex on;` | `off` |
| `cgi_pass` | Esecuzione CGI | `cgi_pass /usr/bin/python3;` | - |
| `upload_path` | Directory upload | `upload_path /uploads;` | - |
| `redirect` | Redirezione | `redirect 301 /new-path;` | - |

## 📋 **Esempi di Configurazione**

### **🔧 Configurazione Minimale**
```nginx
# configs/minimal.conf
server {
    listen 8080;
    server_name localhost;
    root www;
    
    location / {
        allow_methods GET;
        index index.html;
    }
}
```

### **🌐 Server Multipli**
```nginx
# configs/multi_server.conf
server {
    listen 8080;
    server_name site1.local;
    root www/site1;
    
    location / {
        allow_methods GET POST;
        autoindex on;
    }
}

server {
    listen 8081;
    server_name site2.local;
    root www/site2;
    
    location / {
        allow_methods GET POST DELETE;
    }
    
    location /api {
        allow_methods POST;
        cgi_pass /usr/bin/python3;
    }
}
```

### **🔧 Configurazione CGI**
```nginx
# configs/cgi_test.conf
server {
    listen 8080;
    server_name localhost;
    root www;
    client_max_body_size 5M;
    
    # Pagine di errore personalizzate
    error_page 404 /errors/404.html;
    error_page 500 /errors/500.html;
    
    # Location per file statici
    location / {
        allow_methods GET POST;
        index index.html;
        autoindex on;
    }
    
    # Location per script CGI
    location /cgi-bin {
        allow_methods GET POST;
        cgi_pass /usr/bin/python3;
    }
    
    # Location per script PHP
    location ~ \.php$ {
        allow_methods GET POST;
        cgi_pass /usr/bin/php;
    }
    
    # Location per upload
    location /upload {
        allow_methods POST;
        upload_path www/uploads;
        client_max_body_size 10M;
    }
}
```

### **📤 Configurazione Upload**
```nginx
# configs/upload_test.conf
server {
    listen 8080;
    server_name localhost;
    root www;
    client_max_body_size 50M;
    
    location / {
        allow_methods GET;
        index index.html;
    }
    
    location /upload {
        allow_methods POST;
        upload_path www/uploads;
        client_max_body_size 100M;
    }
    
    location /files {
        allow_methods GET DELETE;
        autoindex on;
        root www/uploads;
    }
}
```

## 🔄 **Flusso di Configurazione**

### **1. Caricamento**
```
main() → ServerConfig(file) → ConfigParser → Validazione → Server
```

### **2. Parsing**
```
File → Tokenizer → Parser → AST → Configurazione → Validazione
```

### **3. Applicazione**
```
Richiesta → Matching Server → Matching Location → Applicazione Direttive
```

## 🛠️ **Validazione Configurazione**

### **Controlli Automatici**
- **Sintassi**: Verifica sintassi corretta
- **Semantica**: Controllo coerenza direttive
- **Percorsi**: Validazione esistenza directory
- **Porte**: Controllo disponibilità porte
- **Permessi**: Verifica permessi file/directory

### **Errori Comuni**
```bash
# Errore: Porta già in uso
Error: bind() failed: Address already in use

# Errore: Directory non esistente
Error: Root directory 'www2' does not exist

# Errore: Sintassi non valida
Error: Expected ';' after directive at line 15

# Errore: Direttiva sconosciuta
Error: Unknown directive 'unknown_option' at line 8
```

## 🔍 **Debug Configurazione**

### **Modalità Verbose**
```bash
# Avvia server con debug configurazione
./webserv -v configs/debug.conf

# Output dettagliato del parsing
Server config loaded:
  - Listen: 8080
  - Root: www
  - Locations: 3
    - /: GET, POST
    - /cgi-bin: GET, POST (CGI enabled)
    - /upload: POST (Upload enabled)
```

### **Test Configurazione**
```bash
# Test sintassi senza avviare server
./webserv -t configs/test.conf

# Output:
Configuration test successful
```

## 📊 **Configurazioni Predefinite**

### **🔧 default.conf**
- **Scopo**: Configurazione standard per sviluppo
- **Porta**: 8080
- **Funzionalità**: GET, POST, CGI, Upload
- **Sicurezza**: Livello base

### **🌐 multi_server.conf**
- **Scopo**: Test server multipli
- **Porte**: 8080, 8081, 8082
- **Funzionalità**: Virtual hosting
- **Use case**: Test load balancing

### **🔧 cgi_test.conf**
- **Scopo**: Test completo funzionalità CGI
- **Linguaggi**: Python, PHP, Bash
- **Sicurezza**: Sandbox CGI
- **Performance**: Ottimizzato per CGI

### **📤 upload_test.conf**
- **Scopo**: Test upload file
- **Limiti**: 100MB max
- **Sicurezza**: Validazione tipi file
- **Storage**: Directory dedicata

## 🔒 **Sicurezza**

### **Best Practices**
```nginx
# Limita dimensione body
client_max_body_size 10M;

# Disabilita metodi non necessari
location /static {
    allow_methods GET;
}

# Proteggi directory sensibili
location /admin {
    allow_methods GET POST;
    # auth_basic "Admin Area";  # Futuro
}

# Sandbox CGI
location /cgi-bin {
    cgi_pass /usr/bin/python3;
    # cgi_timeout 30;  # Futuro
}
```

### **Controlli di Sicurezza**
- **Path traversal**: Blocco automatico `../`
- **File extension**: Whitelist estensioni sicure
- **Upload validation**: Controllo tipi MIME
- **Resource limits**: Limiti CPU/memoria per CGI

## 🚀 **Come Creare una Configurazione**

### **1. Copia Template**
```bash
cp configs/default.conf configs/mysite.conf
```

### **2. Modifica Parametri**
```nginx
server {
    listen 9000;                    # Cambia porta
    server_name mysite.local;       # Cambia nome
    root www/mysite;               # Cambia root
    
    location / {
        allow_methods GET POST;
        index index.html;
    }
}
```

### **3. Testa Configurazione**
```bash
./webserv -t configs/mysite.conf
```

### **4. Avvia Server**
```bash
./webserv configs/mysite.conf
```

## 📚 **Risorse Utili**

- **Documentazione completa**: `../README.md`
- **Esempi avanzati**: `examples/`
- **Test configurazione**: `../test_config.sh`
- **Log errori**: Output console del server

---

**💡 Suggerimento**: Inizia sempre con `default.conf` e modifica gradualmente per le tue esigenze specifiche! 