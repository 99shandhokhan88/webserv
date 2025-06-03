# 🚀 WEBSERV - GUIDA COMPLETA ALLA CORREZIONE
## 📋 **Report di Preparazione per l'Evaluation**

---

## **📊 STATO ATTUALE DEL PROGETTO**

### ✅ **COMPLETAMENTE FUNZIONANTE**
- ✅ Server HTTP funzionante 
- ✅ Configurazione multi-port e multi-server
- ✅ Gestione GET, POST, DELETE
- ✅ CGI per Python, PHP, Perl, Shell Script
- ✅ Upload e download files
- ✅ Directory listing (autoindex)
- ✅ Pagine di errore personalizzate
- ✅ DELETE request **CORRETTO** (bug risolto)

---

## **🎯 MANDATORY PART - EVALUATION CHECKLIST**

### **1️⃣ Check the code and ask questions**

| **Domanda dell'Evaluator** | **La tua risposta** | **Dimostrazione/Prova** |
|----------------------------|-------------------|----------------------|
| **Launch siege installation** | `brew install siege` | Lo fai durante la correzione |
| **Explain HTTP server basics** | Un server HTTP riceve richieste HTTP, le processa e restituisce risposte. Il nostro gestisce GET, POST, DELETE con status codes appropriati | Mostra il codice in `srcs/Core/srcs/Server.cpp` |
| **I/O Multiplexing function** | Usiamo `poll()` per gestire multiple connessioni simultaneamente | Vedi `srcs/Core/srcs/Server.cpp:863` - funzione `start()` |
| **How does poll() work** | `poll()` monitora file descriptors per eventi di lettura/scrittura. Blocca fino a eventi disponibili, poi li processa | Mostra il main loop in `Server::start()` |
| **One poll() usage** | Sì, usiamo UN SOLO `poll()` nel main loop che gestisce sia server accept che client read/write | Codice: linea 873-890 in Server.cpp |
| **One read/write per client** | Sì, processiamoUN client per volta nel loop dopo poll() | Vedi `processEvents()` function |
| **Error handling on socket ops** | Tutti i `send/recv` controllano return value e rimuovono client se errore | Cerca `safeSend()` e gestione errori |
| **Return value checking** | Controlliamo sia -1 che 0, e valori parziali per write/read | `safeSend()` function controlla tutto |
| **No errno after socket ops** | ✅ CORRETTO - Non usiamo errno dopo socket operations | Verificabile nel codice |
| **No file I/O without poll** | ✅ CORRETTO - Tutti i file descriptor passano tramite poll() | Main loop verification |

**🔥 PUNTI CRITICI:**
- Se qualche punto non è chiaro, la valutazione si ferma
- Il progetto deve compilare senza errori di re-link

---

### **2️⃣ Configuration**

**Test da dimostrare:**

```bash
# 1. Multiple servers, different ports
./webserv configs/multi_port.conf
# Server 1: localhost:8080, Server 2: localhost:8081

# 2. Multiple hostnames 
curl --resolve example.com:8080:127.0.0.1 http://example.com:8080/

# 3. Custom error pages
curl http://localhost:8080/nonexistent
# Mostra pagina 404 personalizzata da /errors/404.html

# 4. Body size limit
curl -X POST -H "Content-Type: text/plain" --data "VERY LONG BODY..." http://localhost:8080/

# 5. Different directories per route
curl http://localhost:8080/cgi-bin/
curl http://localhost:8080/uploads/

# 6. Default index file
curl http://localhost:8080/
# Serve index.html automaticamente

# 7. Methods per route
curl -X DELETE http://localhost:8080/uploads/file.txt  # ✅ Allowed
curl -X DELETE http://localhost:8080/css/style.css    # ❌ Forbidden
```

---

### **3️⃣ Basic checks**

**Comandi di test:**

```bash
# GET request
curl -v http://localhost:8080/

# POST request (upload)
curl -X POST -F "file=@test.txt" http://localhost:8080/uploads/

# DELETE request 
curl -X DELETE http://localhost:8080/uploads/test.txt -v

# UNKNOWN method (non deve crashare)
curl -X UNKNOWN http://localhost:8080/

# Upload and retrieve file
curl -X POST -F "file=@myfile.txt" http://localhost:8080/uploads/
curl http://localhost:8080/uploads/myfile.txt
```

---

### **4️⃣ Check CGI**

**Test CGI disponibili:**

```bash
# Python CGI
curl http://localhost:8080/cgi-bin/test.py

# PHP CGI  
curl http://localhost:8080/cgi-bin/test.php

# Perl CGI
curl http://localhost:8080/cgi-bin/test.pl

# Shell Script CGI
curl http://localhost:8080/cgi-bin/test.sh

# CGI con POST
curl -X POST -d "name=test&value=123" http://localhost:8080/cgi-bin/form.py

# CGI error handling
curl http://localhost:8080/cgi-bin/error_script.py
```

**File CGI pronti per test:**
- `www/cgi-bin/test.py` - Python script di test
- `www/cgi-bin/test.php` - PHP script di test  
- `www/cgi-bin/test.pl` - Perl script di test
- `www/cgi-bin/test.sh` - Shell script di test

---

### **5️⃣ Check with a browser**

**Test da fare con browser:**

1. Apri `http://localhost:8080` nel browser
2. Controlla Network tab nel developer tools
3. Verifica headers di richiesta e risposta
4. Testa navigazione in directory: `http://localhost:8080/uploads/`
5. Testa URL errato: `http://localhost:8080/nonexistent`
6. Testa redirect se configurato

---

### **6️⃣ Port issues**

```bash
# Test multiple ports
./webserv configs/multi_port.conf
curl http://localhost:8080/  # Server 1
curl http://localhost:8081/  # Server 2

# Test same port multiple times (deve fallire)
# Nel config file, metti stesso port due volte - deve dare errore

# Test multiple servers, stesse porte (deve fallire)
./webserv configs/default.conf &
./webserv configs/default.conf &  # Deve fallire "Address already in use"
```

---

### **7️⃣ Siege & stress test**

```bash
# Install siege
brew install siege

# Basic stress test
siege -b -c 10 -r 10 http://localhost:8080/

# Availability test (>99.5% required)
siege -c 50 -t 30s http://localhost:8080/

# Memory leak test  
# Monitora memoria del processo durante stress test
ps aux | grep webserv  # Memoria non deve crescere indefinitamente

# Long running test
siege -c 20 -d 1 -r 100 http://localhost:8080/
```

---

## **🎁 BONUS PART**

### **1️⃣ Cookies and session**
- ❌ **NON IMPLEMENTATO** - Non abbiamo sistema di cookies/session

### **2️⃣ Multiple CGI systems**
- ✅ **IMPLEMENTATO** - Abbiamo Python, PHP, Perl, Shell Script CGI

---

## **🛠️ COME AVVIARE IL SERVER**

```bash
# Compilazione
make clean && make

# Avvio server default
./webserv configs/default.conf

# Avvio server multi-port
./webserv configs/multi_port.conf

# Controllo processi
ps aux | grep webserv

# Stop server
pkill webserv
```

---

## **📁 STRUTTURA FILE IMPORTANTI**

```
webserv/
├── configs/
│   ├── default.conf        # Configurazione principale
│   └── multi_port.conf     # Multi-server config
├── www/
│   ├── index.html          # Homepage
│   ├── delete_tests.html   # Test DELETE interface  
│   ├── uploads/            # Directory upload files
│   ├── cgi-bin/           # CGI scripts
│   │   ├── index.html     # CGI test page (TRADOTTO IN INGLESE)
│   │   ├── test.py        # Python CGI
│   │   ├── test.php       # PHP CGI  
│   │   ├── test.pl        # Perl CGI
│   │   └── test.sh        # Shell CGI
│   ├── css/               # Stylesheets
│   └── errors/            # Custom error pages
└── srcs/                  # Source code
    ├── Core/              # Server core
    └── Utils/             # Utilities
```

---

## **🚨 PROBLEMI RISOLTI**

### **❌ Problema DELETE (RISOLTO)**
- **Era:** Server crashava su richieste DELETE
- **Causa:** Doppia chiamata a `removeClient()` e gestione asincrona problematica
- **Fix:** Gestione sincrona con `unlink()` diretto e corretta gestione errori
- **Test:** `curl -X DELETE http://localhost:8080/uploads/test.txt` funziona perfettamente

### **✅ Traduzioni HTML**
- Tutti i file HTML in `cgi-bin/` tradotti da italiano a inglese
- Aggiunto supporto Perl CGI nell'interfaccia

---

## **📝 SCRIPT DI TEST RAPIDO**

```bash
#!/bin/bash
echo "=== WEBSERV QUICK TEST ==="

# Start server
./webserv configs/default.conf &
sleep 2

# Test GET
echo "Testing GET..."
curl -s http://localhost:8080/ > /dev/null && echo "✅ GET OK" || echo "❌ GET FAIL"

# Test POST  
echo "Testing POST..."
echo "test content" | curl -s -X POST --data-binary @- http://localhost:8080/uploads/test.txt && echo "✅ POST OK" || echo "❌ POST FAIL"

# Test DELETE
echo "Testing DELETE..."
curl -s -X DELETE http://localhost:8080/uploads/test.txt && echo "✅ DELETE OK" || echo "❌ DELETE FAIL"

# Test CGI
echo "Testing CGI..."
curl -s http://localhost:8080/cgi-bin/test.py > /dev/null && echo "✅ CGI OK" || echo "❌ CGI FAIL"

# Cleanup
pkill webserv
echo "=== TEST COMPLETED ==="
```

---

## **🎯 DOMANDE FREQUENTI DURANTE CORREZIONE**

**Q: Come gestite le connessioni multiple?**
A: Usiamo `poll()` per monitorare tutti i file descriptors in un unico loop.

**Q: Come prevenite i memory leaks?**
A: RAII pattern, smart pointers dove possibile, cleanup appropriato in destructors.

**Q: Cosa succede se un CGI va in loop infinito?**
A: Abbiamo timeout configurabili e gestione errori che previene hang del server.

**Q: Come gestite i file grandi?**
A: Lettura/scrittura a chunk per evitare memory overflow.

**Q: Il server scala bene?**
A: Sì, poll() gestisce efficientemente centinaia di connessioni simultanee.

---

## **🏆 PUNTEGGIO ATTESO**

- **Mandatory Part:** 100% ✅
- **Bonus CGI:** +5 punti ✅  
- **Outstanding Project:** Possibile per quality implementation

**TOTALE ATTESO: 100-125 punti**

---

## **📞 ULTIMO REMINDER**

1. ✅ Compila senza errori
2. ✅ No segfaults o crashes
3. ✅ No memory leaks
4. ✅ Tutti i test passano
5. ✅ Codice ben strutturato e commentato
6. ✅ Configuration flessibile e completa

**🔥 SEI PRONTO PER LA CORREZIONE! 🔥** 