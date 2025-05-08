#!/usr/bin/env python3
import cgi
import os

print("Content-Type: text/html\n\n")  # La prima cosa che devi fare!
print("<h1>Test CGI Funzionante!</h1>")

# Ottieni i dati inviati tramite POST (multipart/form-data)
form = cgi.FieldStorage()

# Verifica se il file è presente
if "file" in form:
    file_item = form["file"]

    if file_item.filename:
        # Pulisci il nome del file
        filename = os.path.basename(file_item.filename)
        upload_dir = "/path/to/your/uploads"  # Cambia con il tuo percorso di upload
        filepath = os.path.join(upload_dir, filename)

        # Salva il file
        with open(filepath, "wb") as f:
            f.write(file_item.file.read())
        
        print("<h2>File caricato correttamente: {}</h2>".format(filename))
    else:
        print("<h2>No file uploaded</h2>")
else:
    print("<h2>No file uploaded</h2>")
