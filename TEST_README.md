# WebServ Complete Test Suite

Questa è una suite di test completa per il progetto webserv di 42, progettata per verificare la conformità HTTP, i requisiti del subject e tutti i casi estremi.

## 📋 Panoramica

La suite include 4 script di test principali:

1. **`run_all_tests.sh`** - Script master che esegue tutti i test
2. **`subject_compliance_test.sh`** - Test per i requisiti specifici del subject 42
3. **`comprehensive_test.sh`** - Test completi per la conformità HTTP
4. **`extreme_edge_cases_test.sh`** - Test per casi estremi e parsing HTTP

## 🚀 Come Eseguire i Test

### Prerequisiti

1. **Server in esecuzione**: Assicurati che il tuo webserver sia in esecuzione:
   ```bash
   ./webserv configs/default.conf
   ```

2. **Dipendenze**: I test richiedono:
   - `curl` - per le richieste HTTP
   - `nc` (netcat) - per i test raw TCP
   - `python3` - per generare dati di test
   - `bc` - per calcoli matematici (opzionale)

### Esecuzione Rapida

Per eseguire tutti i test in una volta:
```bash
./run_all_tests.sh
```

### Esecuzione Singola

Per eseguire test specifici:

```bash
# Solo requisiti del subject
./subject_compliance_test.sh

# Solo conformità HTTP generale
./comprehensive_test.sh

# Solo casi estremi
./extreme_edge_cases_test.sh
```

## 📊 Cosa Viene Testato

### 1. Subject Compliance Test (`subject_compliance_test.sh`)

Verifica tutti i requisiti specifici del subject 42:

- ✅ **Metodi HTTP obbligatori**: GET, POST, DELETE
- ✅ **Status code corretti**: 200, 404, 405, 400, etc.
- ✅ **Header HTTP**: Content-Type, Content-Length, Server
- ✅ **File di configurazione**: Parsing e server blocks
- ✅ **Pagine di errore personalizzate**
- ✅ **Upload di file**
- ✅ **Directory listing (autoindex)**
- ✅ **Esecuzione CGI**
- ✅ **Servizio file statici**
- ✅ **Conformità HTTP/1.1**
- ✅ **Location blocks**
- ✅ **Restrizioni metodi per location**
- ✅ **File di default (index)**
- ✅ **I/O non bloccante**

### 2. Comprehensive Test (`comprehensive_test.sh`)

Test completi per funzionalità HTTP:

- 🔍 **Metodi HTTP di base**: GET, POST, DELETE
- 📋 **Conformità header HTTP**
- 🚫 **Gestione metodi non permessi (405)**
- 📁 **Funzionalità autoindex**
- ❌ **Pagine di errore personalizzate**
- 🔧 **Test CGI**
- 📤 **Test upload file**
- 📊 **Test richieste grandi**
- 🔧 **Test richieste malformate**
- 🔄 **Test richieste concorrenti**
- 🎯 **Test casi limite**
- 🌐 **Conformità HTTP/1.1**
- ⚡ **Test di stress**
- 🔒 **Test di sicurezza**

### 3. Extreme Edge Cases Test (`extreme_edge_cases_test.sh`)

Test per i casi più estremi:

- 💥 **Richieste HTTP malformate**
- 📝 **Casi limite degli header**
- 🔗 **Casi limite degli URL**
- 📋 **Versioni HTTP non valide**
- 📦 **Casi limite del body**
- 📄 **Terminatori di riga diversi**
- 🔄 **Chunked encoding**
- 🔌 **Casi limite delle connessioni**
- ⏱️ **Test di timeout**
- 💾 **Test di esaurimento risorse**
- 🔒 **Casi limite di sicurezza**

## 📈 Interpretazione dei Risultati

### Codici di Uscita

- **0**: Tutti i test passati ✅
- **1**: Buona conformità (≥80%) ⚠️
- **2**: Conformità discreta (≥60%) ⚠️
- **3**: Conformità scarsa (<60%) ❌

### Output dei Test

Ogni test mostra:
- ✅ **PASS** - Test superato
- ❌ **FAIL** - Test fallito
- 📊 **Statistiche finali** - Riepilogo dei risultati

### Esempio di Output

```
✓ PASS - GET_ROOT: HTTP GET to http://localhost:8080/ (Expected: 200, Got: 200)
✗ FAIL - POST_CSS_NOT_ALLOWED: HTTP POST to http://localhost:8080/css/ (Expected: 405, Got: 200)

=== TEST SUMMARY ===
Total tests: 45
Passed: 42
Failed: 3
Overall Score: 93%
```

## 🔧 Configurazione

### Modifica Porta/Host

Se il tuo server usa una porta diversa, modifica le variabili nei file:

```bash
SERVER_HOST="localhost"
SERVER_PORT="8080"  # Cambia qui
```

### Personalizzazione Path

I test assumono questa struttura:
```
webserv/
├── webserv                 # Eseguibile
├── configs/
│   └── default.conf       # Configurazione
├── www/                   # Document root
│   ├── index.html
│   ├── uploads/
│   ├── css/
│   └── cgi-bin/
└── test_scripts/          # Script di test
```

## 🐛 Risoluzione Problemi

### Server Non Risponde

```bash
# Verifica che il server sia in esecuzione
ps aux | grep webserv

# Verifica la porta
netstat -tlnp | grep 8080

# Testa manualmente
curl http://localhost:8080/
```

### Test Falliscono

1. **Controlla i log del server** per errori
2. **Verifica la configurazione** in `configs/default.conf`
3. **Testa manualmente** le funzionalità che falliscono
4. **Controlla i permessi** dei file e directory

### Dipendenze Mancanti

```bash
# Ubuntu/Debian
sudo apt-get install curl netcat-openbsd python3 bc

# CentOS/RHEL
sudo yum install curl nc python3 bc

# macOS
brew install curl netcat python3 bc
```

## 📝 Note Importanti

### Requisiti del Subject

I test verificano specificamente:
- Conformità HTTP/1.1 completa
- Gestione corretta degli status code
- Parsing robusto delle richieste
- Gestione degli errori appropriata
- Funzionalità CGI
- Upload e gestione file
- Configurazione flessibile

### Casi Estremi Testati

- Richieste malformate e incomplete
- Header con caratteri non validi
- URL con caratteri speciali
- Richieste molto grandi
- Connessioni concorrenti
- Tentativi di attacco comuni

### Performance

I test includono anche verifiche di performance:
- Gestione richieste concorrenti
- Tempo di risposta
- Stabilità sotto carico

## 🎯 Obiettivi di Conformità

Per superare la valutazione, il tuo server dovrebbe:

- ✅ **≥95%** sui test del subject
- ✅ **≥90%** sui test HTTP generali  
- ✅ **≥80%** sui casi estremi
- ✅ **100%** sui controlli di sanità

## 📞 Supporto

Se hai problemi con i test:

1. Controlla che il server rispetti il subject 42
2. Verifica la configurazione di esempio
3. Testa manualmente le funzionalità che falliscono
4. Assicurati che tutti i path e permessi siano corretti

---

**Buona fortuna con il tuo webserver! 🚀** 