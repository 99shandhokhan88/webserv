/**
 * @file Client.hpp
 * @brief Classe per la gestione dello stato e delle operazioni dei client HTTP
 * 
 * Questa classe rappresenta un singolo client connesso al server e gestisce:
 * - Stato della connessione (file descriptor, keep-alive)
 * - Buffer dei dati ricevuti e da inviare
 * - Parsing delle richieste HTTP
 * - Gestione del ciclo di vita della connessione
 * - Interfaccia per l'invio delle risposte
 * 
 * Ciclo di vita tipico:
 * 1. Creazione con fd della connessione
 * 2. Accumulo dati ricevuti (appendRequestData)
 * 3. Verifica completezza richiesta (isRequestComplete)
 * 4. Parsing richiesta HTTP (parseRequest)
 * 5. Elaborazione tramite server (handleRequest)
 * 6. Invio risposta (send_pending_data)
 * 7. Reset per nuova richiesta o chiusura
 */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../../../incs/webserv.hpp"
#include "../../HTTP/incs/Request.hpp"

/**
 * @brief Classe che rappresenta un client connesso al server
 * 
 * Gestisce lo stato di una singola connessione client:
 * - Mantiene il file descriptor della connessione
 * - Accumula i dati ricevuti fino a formare una richiesta completa
 * - Parsa le richieste HTTP
 * - Gestisce il keep-alive delle connessioni
 * - Fornisce interfaccia per l'invio delle risposte
 */
class Client {
private:
    // ==================== MEMBRI PRIVATI ====================
    
    /** @brief Buffer per dati in attesa di essere inviati al client */
    std::string pending_data;
    
    /** @brief Flag per gestione keep-alive della connessione */
    bool keep_alive;

    // ==================== METODI HELPER PRIVATI ====================
    
    /**
     * @brief Estrae il Content-Length dagli header HTTP
     * @param headers Stringa contenente gli header HTTP
     * @return Dimensione del body in bytes, 0 se non specificata
     * 
     * REFACTORING: Estratta da isRequestComplete() per migliorare leggibilità
     */
    size_t extractContentLength(const std::string& headers) const;
    
    /**
     * @brief Verifica se la richiesta è di tipo GET o HEAD (senza body)
     * @return true se la richiesta non dovrebbe avere un body
     * 
     * REFACTORING: Estratta da isRequestComplete() per semplificare la logica
     */
    bool isMethodWithoutBody() const;
    
    /**
     * @brief Valida la dimensione del body contro i limiti configurati
     * @param content_length Dimensione dichiarata del body
     * @param body_received Dimensione effettiva ricevuta
     * @throws std::runtime_error se il body è troppo grande
     * 
     * REFACTORING: Estratta da isRequestComplete() per centralizzare i controlli
     */
    void validateBodySize(size_t content_length, size_t body_received) const;

public:
    // ==================== MEMBRI PUBBLICI ====================
    
    /** @brief File descriptor della connessione socket */
    int fd;
    
    /** @brief Oggetto richiesta HTTP parsata */
    Request request;
    
    /** @brief Buffer grezzo dei dati ricevuti dal client */
    std::string request_data;
    
    // ==================== GESTIONE RICHIESTE ====================
    
    /**
     * @brief Parsa i dati grezzi in un oggetto Request HTTP
     * @throws std::exception se la richiesta è malformata
     * 
     * Operazioni eseguite:
     * 1. Estrazione metodo, URL e versione HTTP
     * 2. Parsing degli header HTTP
     * 3. Gestione del body (se presente)
     * 4. Validazione della richiesta
     */
    void parseRequest();

    /**
     * @brief Resetta lo stato del client per una nuova richiesta
     * 
     * Pulisce:
     * - Buffer dei dati di richiesta
     * - Stato della richiesta precedente
     * - Mantiene la connessione se keep-alive è attivo
     */
    void reset() {
        request_data.clear();
        // Reset other request-related state
    }

    /**
     * @brief Verifica se la richiesta HTTP è completa
     * @return true se la richiesta è completa e pronta per il parsing
     * 
     * Controlla:
     * - Presenza di linea vuota che separa header da body
     * - Completezza del body (se Content-Length specificato)
     * - Terminazione corretta per richieste chunked
     * 
     * REFACTORING: Semplificata utilizzando metodi helper privati
     */
    bool isRequestComplete();

    // ==================== COSTRUTTORE E DISTRUTTORE ====================
    
    /**
     * @brief Costruisce un nuovo client
     * @param client_fd File descriptor della connessione (default: -1)
     * 
     * Inizializza:
     * - File descriptor della connessione
     * - Stato keep-alive (default: false)
     * - Buffer vuoti per dati
     */
    explicit Client(int client_fd = -1);
    
    /**
     * @brief Distruttore - cleanup automatico delle risorse
     */
    ~Client() {};

    // ==================== GESTIONE RICHIESTE E RISPOSTE ====================
    
    /**
     * @brief Gestisce la richiesta del client tramite il server
     * @param server Riferimento al server che elabora la richiesta
     * 
     * Delega al server:
     * - Routing della richiesta
     * - Elaborazione business logic
     * - Generazione della risposta
     * - Gestione errori
     */
    void handleRequest(class Server& server);

    /**
     * @brief Legge dati dal socket del client
     * @return Stringa contenente i dati letti
     * @throws std::exception se errore di lettura
     * 
     * Operazioni:
     * - Lettura non-bloccante dal socket
     * - Gestione errori di rete
     * - Rilevamento disconnessioni
     */
    std::string readData();
    
    /**
     * @brief Prepara una risposta per l'invio al client
     * @param content Contenuto della risposta da inviare
     * 
     * Operazioni:
     * - Formattazione risposta HTTP
     * - Aggiunta header appropriati
     * - Buffering per invio asincrono
     */
    void prepare_response(const std::string& content);
    
    /**
     * @brief Invia i dati in attesa al client
     * @return true se tutti i dati sono stati inviati
     * 
     * Gestisce:
     * - Invio non-bloccante
     * - Invio parziale (EAGAIN/EWOULDBLOCK)
     * - Gestione errori di rete
     * - Aggiornamento buffer pending_data
     */
    bool send_pending_data();

    // ==================== GESTIONE DATI ====================
    
    /**
     * @brief Aggiunge dati ricevuti al buffer di richiesta
     * @param data Puntatore ai dati ricevuti
     * @param length Lunghezza dei dati in bytes
     * 
     * Operazioni:
     * - Append sicuro al buffer request_data
     * - Gestione di ricezioni parziali
     * - Controllo limiti di dimensione
     */
    void appendRequestData(const char* data, size_t length);
    
    /**
     * @brief Verifica se la connessione deve rimanere aperta
     * @return true se keep-alive è attivo
     * 
     * Considera:
     * - Header "Connection: keep-alive"
     * - Versione HTTP (1.1 default keep-alive)
     * - Configurazione del server
     */
    bool shouldKeepAlive() const;
    
    /**
     * @brief Imposta lo stato keep-alive della connessione
     * @param value true per attivare keep-alive
     * 
     * Influenza:
     * - Durata della connessione
     * - Header di risposta
     * - Gestione del socket dopo la risposta
     */
    void setKeepAlive(bool value);
};

#endif