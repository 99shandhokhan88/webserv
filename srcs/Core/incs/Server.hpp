/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/29 22:54:15 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file Server.hpp
 * @brief Classe principale del webserver HTTP
 * 
 * Questa classe gestisce:
 * - Creazione e configurazione del socket del server
 * - Accettazione di nuove connessioni client
 * - Gestione del polling per I/O non-bloccante
 * - Routing delle richieste HTTP
 * - Generazione delle risposte HTTP
 * - Esecuzione di script CGI
 * - Gestione di file upload e download
 * 
 * Architettura:
 * - Pattern Singleton per la gestione globale dei server
 * - Polling con epoll/poll per gestione asincrona
 * - Gestione multi-server per più porte/configurazioni
 */

#ifndef SERVER_HPP
# define SERVER_HPP

#include "../../../incs/webserv.hpp"
#include "../../Config/incs/ServerConfig.hpp"
#include "../../Config/incs/LocationConfig.hpp"
#include "Client.hpp"
#include "../../HTTP/incs/Request.hpp"
#include "../../HTTP/incs/Response.hpp"

// ==================== CLASSE SERVER ====================

/**
 * @brief Classe principale del server HTTP
 * 
 * Gestisce l'intero ciclo di vita del server:
 * 1. Inizializzazione socket e binding
 * 2. Loop principale con polling
 * 3. Accettazione connessioni
 * 4. Gestione richieste HTTP
 * 5. Generazione risposte
 * 6. Cleanup risorse
 */
class Server {
private:
    // ==================== MEMBRI STATICI ====================
    // Gestione globale di tutti i server attivi
    
    /** @brief Lista di tutti i server attivi nel sistema */
    static std::vector<Server*>      servers;
    
    /** @brief File descriptors per il polling I/O */
    static std::vector<struct pollfd> poll_fds;
    
    /** @brief Mappa dei client connessi (fd -> Client) */
    static std::map<int, Client>      clients;

    // ==================== MEMBRI DI ISTANZA ====================
    
    /** @brief Configurazione specifica di questo server */
    ServerConfig     config;
    
    /** @brief File descriptor del socket del server */
    int              server_fd;
    
    /** @brief Indirizzo e porta del server */
    struct sockaddr_in address;

    // ==================== METODI PRIVATI ====================
    
    /**
     * @brief Configura e inizializza il socket del server
     * @throws std::runtime_error se fallisce la creazione/configurazione
     * 
     * Operazioni eseguite:
     * 1. Creazione socket TCP
     * 2. Configurazione opzioni (SO_REUSEADDR)
     * 3. Binding all'indirizzo/porta
     * 4. Attivazione modalità listen
     * 5. Configurazione non-blocking
     */
    void setupSocket();
    
    // ==================== GESTORI RICHIESTE HTTP ====================
    
    /** @brief Gestisce richieste GET (lettura file, directory listing) */
    void handleGetRequest(Client* client);
    
    
    void handleRootRequest(Client* client, const LocationConfig& location, const std::string& path);
    
    
    void handleDirectoryRequest(Client* client, const LocationConfig& location, const std::string& path);
    
    
    void handleCgiRequest(Client* client, const LocationConfig& location);
    
    /** @brief Gestisce richieste POST (upload file, form data) */
    void handlePostRequest(Client* client);
    
    /** @brief Gestisce richieste DELETE (cancellazione file) */
    void handleDeleteRequest(Client* client);
    
    /** @brief Gestisce richieste OPTIONS (CORS, metodi supportati) */
    void handleOptionsRequest(Client* client);
    
    // ==================== OPERAZIONI FILE E DIRECTORY ====================
    
    /**
     * @brief Invia un file come risposta HTTP
     * @param client Client che ha fatto la richiesta
     * @param path Percorso del file da inviare
     * @param isHeadRequest true per richieste HEAD (solo headers)
     */
    void sendFileResponse(Client* client, const std::string& path, bool isHeadRequest = false);
    
    /**
     * @brief Genera e invia listing di una directory
     * @param client Client che ha fatto la richiesta
     * @param path Percorso della directory
     */
    void handleDirectoryListing(Client* client, const std::string& path);
    
    /**
     * @brief Verifica se una richiesta deve essere gestita da CGI
     * @param location Configurazione della location
     * @param path Percorso richiesto
     * @return true se è una richiesta CGI
     */
    bool isCgiRequest(const LocationConfig& location, const std::string& path) const;
    
    /**
     * @brief Analizza body multipart per upload file
     * @param body Contenuto del body HTTP
     * @param boundary Boundary per separare le parti
     * @param uploadDir Directory dove salvare i file
     */
    void parseMultipartBody(const std::string& body, const std::string& boundary, const std::string& uploadDir);
    
    // ==================== GESTIONE POLLING ====================
    
    /** @brief Aggiunge un file descriptor al polling */
    static void addPollFD(int fd, short events);
    
    /** @brief Rimuove un file descriptor dal polling */
    static void removePollFD(int fd);
    
    /** @brief Verifica se un fd appartiene a un server */
    bool isServerFD(int fd) const;
    
    // ==================== GESTIONE ERRORI ====================
    
    /**
     * @brief Ottiene la pagina di errore personalizzata
     * @param errorCode Codice di errore HTTP
     * @return Contenuto HTML della pagina di errore
     */
    std::string getErrorPage(int errorCode) const;
    
    /**
     * @brief Invia dati in modo sicuro gestendo errori senza errno
     * @param client Client destinatario
     * @param data Dati da inviare
     * @return true se invio riuscito, false se client deve essere rimosso
     */
    static bool safeSend(Client* client, const std::string& data);

public:
    // ==================== COSTRUTTORE E DISTRUTTORE ====================
    
    /**
     * @brief Costruisce un nuovo server con la configurazione data
     * @param config Configurazione del server
     * @throws std::runtime_error se fallisce l'inizializzazione
     */
    Server(const ServerConfig& config);
    
    /**
     * @brief Distruttore - chiude socket e pulisce risorse
     */
    ~Server();
    
    // ==================== GETTERS ====================
    
    /** @brief Restituisce il file descriptor del server */
    int getServerFd() const;
    
    /** @brief Restituisce la lista dei file descriptors per polling */
    const std::vector<struct pollfd>& getPollFds() const;
    
    // ==================== OPERAZIONI PRINCIPALI ====================
    
    /**
     * @brief Loop principale del server
     * 
     * Esegue il ciclo infinito di:
     * 1. Polling per eventi I/O
     * 2. Accettazione nuove connessioni
     * 3. Lettura richieste client
     * 4. Elaborazione e risposta
     * 5. Cleanup connessioni chiuse
     */
    static void run();
    
    /**
     * @brief Pulisce tutte le risorse e chiude i server
     */
    static void cleanup();
    
    // ==================== GESTIONE CLIENT ====================
    
    /**
     * @brief Accetta una nuova connessione client
     * @throws std::runtime_error se fallisce l'accettazione
     */
    void acceptNewConnection();
    
    /**
     * @brief Gestisce la comunicazione con un client
     * @param client_fd File descriptor del client
     * 
     * Operazioni:
     * 1. Lettura dati dal socket
     * 2. Parsing richiesta HTTP
     * 3. Elaborazione richiesta
     * 4. Invio risposta
     * 5. Gestione keep-alive o chiusura
     */
    static void handleClient(int client_fd);
    
    /**
     * @brief Invia una risposta HTTP al client
     * @param client Client destinatario
     * @param status Codice di stato HTTP
     * @param content Contenuto della risposta
     */
    void sendResponse(Client* client, int status, const std::string& content);
    
    /**
     * @brief Modifica gli eventi di polling per un client
     * @param index Indice nel vettore poll_fds
     * @param events Nuovi eventi da monitorare
     */
    void setPollEvents(size_t index, short events);
    
    /**
     * @brief Gestisce una richiesta HTTP
     * @param request Richiesta da elaborare
     * @param response Risposta da generare
     */
    void handleRequest(const Request& request, Response& response);
    
    /**
     * @brief Elabora una richiesta di un client
     * @param client Client che ha fatto la richiesta
     */
    static void processRequest(Client* client);
    
    /**
     * @brief Rimuove un client dalle strutture dati
     * @param client_fd File descriptor del client da rimuovere
     */
    static void removeClient(int client_fd);
    
    /**
     * @brief Invia una risposta di errore al client
     * @param client Client destinatario
     * @param statusCode Codice di errore HTTP
     * @param message Messaggio di errore
     * @param config Configurazione del server per pagine di errore personalizzate
     */
    static void sendErrorResponse(Client* client, int statusCode, const std::string& message, const ServerConfig& config);
    
    /**
     * @brief Invia risposta "Method Not Allowed"
     * @param client Client destinatario
     * @param allowedMethods Lista dei metodi permessi
     */
    static void sendMethodNotAllowedResponse(Client* client, const std::vector<std::string>& allowedMethods);
    
    /**
     * @brief Invia risposta OPTIONS con metodi supportati
     * @param client Client destinatario
     * @param allowedMethods Lista dei metodi supportati
     */
    static void sendOptionsResponse(Client* client, const std::vector<std::string>& allowedMethods);
    
    /**
     * @brief Analizza dati form URL-encoded
     * @param body Corpo della richiesta
     * @param formData Mappa dove salvare i dati parsati
     */
    void parseFormUrlEncoded(const std::string& body, std::map<std::string, std::string>& formData);
};

#endif // SERVER_HPP