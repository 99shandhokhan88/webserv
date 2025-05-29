# 📁 Directory `www/` - Contenuto Web

## 🎯 **Panoramica**

Questa directory contiene tutto il contenuto web servito dal server HTTP, inclusi file statici, script CGI, pagine di test e risorse multimediali.

## 📂 **Struttura delle Directory**

```
www/
├── index.html            # 🏠 Pagina principale del sito
├── cgi-bin/             # 🔧 Script CGI eseguibili
│   ├── index.html       # ✅ Suite di test CGI
│   ├── info.py          # Script Python di test
│   ├── info.sh          # Script Bash di test
│   └── info.php         # Script PHP di test
├── uploads/             # 📤 Directory per file caricati
├── css/                 # 🎨 Fogli di stile CSS
│   └── test.css         # Stili di test
├── errors/              # ❌ Pagine di errore personalizzate
│   ├── 404.html         # Pagina "Not Found"
│   ├── 500.html         # Pagina "Internal Server Error"
│   └── 403.html         # Pagina "Forbidden"
├── test_dir/            # 📁 Directory per test autoindex
└── test_files/          # 📄 File di test vari
    ├── get_tests.html   # Test per richieste GET
    ├── post_tests.html  # Test per richieste POST
    ├── delete_tests.html # Test per richieste DELETE
    ├── test.html        # File di test generico
    ├── test.txt         # File di testo di test
    └── test.js          # Script JavaScript di test
```

## 🔍 **Descrizione dei Contenuti**

### **🏠 Pagine Principali**

#### **index.html**
- **Scopo**: Pagina di benvenuto e dashboard principale
- **Contenuto**: 
  - Link ai test del server
  - Interfaccia per testare funzionalità
  - Documentazione rapida
- **Stato**: ✅ Funzionale

### **🔧 CGI Scripts (`cgi-bin/`)**

Questa directory contiene script eseguibili per testare il supporto CGI multi-linguaggio:

#### **index.html**
- **Scopo**: Suite di test per script CGI
- **Funzionalità**:
  - Test Python CGI
  - Test Bash CGI  
  - Test PHP CGI
- **Stato**: ✅ Completamente funzionale

#### **info.py**
```python
# Script Python che mostra:
# - Informazioni ambiente
# - Variabili CGI
# - Parametri richiesta
```

#### **info.sh**
```bash
# Script Bash che mostra:
# - Environment variables
# - Informazioni sistema
# - Parametri GET/POST
```

#### **info.php**
```php
# Script PHP che mostra:
# - phpinfo() completo
# - Variabili superglobali
# - Configurazione PHP
```

### **📤 Upload Directory (`uploads/`)**

- **Scopo**: Directory di destinazione per file caricati
- **Permessi**: Scrittura abilitata per il server
- **Sicurezza**: 
  - Controllo tipi file
  - Limiti dimensione
  - Validazione nomi file

### **🎨 Risorse Statiche**

#### **CSS (`css/`)**
- **test.css**: Stili per le pagine di test
- **Responsive design**: Compatibile mobile/desktop
- **Temi**: Supporto per temi chiari/scuri

### **❌ Pagine di Errore (`errors/`)**

Pagine personalizzate per errori HTTP comuni:

#### **404.html - Not Found**
```html
<!-- Pagina elegante per risorse non trovate -->
<!-- Include suggerimenti e link utili -->
```

#### **500.html - Internal Server Error**
```html
<!-- Pagina per errori del server -->
<!-- Informazioni per il debug (se abilitato) -->
```

#### **403.html - Forbidden**
```html
<!-- Pagina per accessi negati -->
<!-- Spiegazione dei permessi -->
```

### **📁 Directory di Test**

#### **test_dir/**
- **Scopo**: Test funzionalità autoindex
- **Contenuto**: File vari per testare listing directory
- **Configurazione**: Autoindex abilitato

#### **File di Test**
- **get_tests.html**: Interfaccia per testare richieste GET
- **post_tests.html**: Form per testare richieste POST
- **delete_tests.html**: Interfaccia per testare richieste DELETE
- **test.txt**: File di testo semplice
- **test.js**: Script JavaScript di esempio

## 🔄 **Flusso delle Richieste**

### **Richieste Statiche**
```
GET /index.html → www/index.html → Risposta HTTP
GET /css/test.css → www/css/test.css → Content-Type: text/css
```

### **Richieste CGI**
```
GET /cgi-bin/info.py → Esecuzione Python → Output dinamico
POST /cgi-bin/upload.sh → Elaborazione form → Salvataggio file
```

### **Upload File**
```
POST /upload → Parsing multipart → Salvataggio in uploads/
```

### **Errori**
```
GET /nonexistent → 404 → www/errors/404.html
Errore server → 500 → www/errors/500.html
```

## 🛠️ **Configurazione**

### **Tipi MIME Supportati**
```
.html → text/html
.css  → text/css
.js   → application/javascript
.txt  → text/plain
.png  → image/png
.jpg  → image/jpeg
.pdf  → application/pdf
```

### **Directory Index**
```
index.html (priorità 1)
index.htm  (priorità 2)
index.php  (priorità 3)
```

### **CGI Extensions**
```
.py  → Python interpreter
.sh  → Bash shell
.php → PHP interpreter
.pl  → Perl interpreter
```

## 🔒 **Sicurezza**

### **Controlli Implementati**
- **Path traversal protection**: Blocca `../` negli URL
- **File extension validation**: Solo estensioni sicure
- **Upload size limits**: Limite dimensione file
- **CGI sandboxing**: Esecuzione in ambiente controllato

### **Permessi File**
```
Directory: 755 (rwxr-xr-x)
File HTML: 644 (rw-r--r--)
Script CGI: 755 (rwxr-xr-x)
Upload dir: 755 (rwxr-xr-x)
```

## 📊 **Statistiche**

- **File statici**: ~15 file
- **Script CGI**: 3 linguaggi supportati
- **Pagine errore**: 3 pagine personalizzate
- **Directory**: 6 directory organizzate
- **Dimensione totale**: ~50KB

## 🚀 **Come Aggiungere Contenuto**

### **File Statici**
1. Aggiungi file nella directory appropriata
2. Verifica permessi (644 per file, 755 per directory)
3. Testa l'accesso via browser

### **Script CGI**
1. Crea script in `cgi-bin/`
2. Imposta permessi eseguibili (755)
3. Aggiungi shebang appropriato
4. Testa funzionalità

### **Pagine di Errore**
1. Crea file HTML in `errors/`
2. Nomina con codice errore (es. `404.html`)
3. Configura nel file di configurazione server

## 🧪 **Test Disponibili**

### **Test Manuali**
- Navigazione pagine statiche
- Upload file tramite form
- Esecuzione script CGI
- Verifica pagine errore

### **Test Automatici**
- Script di test in directory root
- Verifica compliance HTTP
- Test performance
- Test sicurezza

## 📚 **Risorse Utili**

- **Configurazione server**: `../configs/`
- **Log server**: Controlla output console
- **Test automatici**: `../run_all_tests.sh`
- **Documentazione**: `../README.md`

---

**💡 Suggerimento**: Per testare rapidamente il server, visita `/cgi-bin/index.html` che contiene una suite completa di test! 