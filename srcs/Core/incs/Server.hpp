/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/16 12:17:23 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/15 19:23:23 by vzashev          ###   ########.fr       */
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

# include <iostream>
# include <vector>
# include <map>
# include <set>
# include <string>
# include <cstring>
# include <sstream>
# include <fstream>
# include <algorithm>
# include <stdexcept>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/types.h>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <errno.h>
# include "../../Config/incs/ServerConfig.hpp"
# include "../../Config/incs/LocationConfig.hpp"

// Forward declarations per evitare dipendenze circolari
class Client;
class Request;
class Response;

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
    
    // ==================== METODI DI UTILITÀ ====================
    
    /**
     * @brief Modifica gli eventi di polling per un fd
     * @param index Indice nell'array poll_fds
     * @param events Nuovi eventi da monitorare
     */
    void setPollEvents(size_t index, short events);
    
    /**
     * @brief Gestisce una richiesta HTTP generica
     * @param request Richiesta HTTP parsata
     * @param response Oggetto risposta da popolare
     */
    void handleRequest(const Request& request, Response& response);
    
    // ==================== METODI STATICI DI GESTIONE ====================
    
    /**
     * @brief Elabora una richiesta client completa
     * @param client Client con richiesta da elaborare
     * 
     * Determina il tipo di richiesta e chiama il gestore appropriato
     */
    static void processRequest(Client* client);
    
    /**
     * @brief Rimuove un client e chiude la connessione
     * @param client_fd File descriptor del client da rimuovere
     */
    static void removeClient(int client_fd);
    
    /**
     * @brief Invia una risposta di errore HTTP
     * @param client Client destinatario
     * @param statusCode Codice di errore HTTP
     * @param message Messaggio di errore
     * @param config Configurazione per pagine di errore personalizzate
     */
    static void sendErrorResponse(Client* client, int statusCode, const std::string& message, const ServerConfig& config);
    
    /**
     * @brief Invia risposta "405 Method Not Allowed"
     * @param client Client destinatario
     * @param allowedMethods Lista dei metodi HTTP permessi
     */
    static void sendMethodNotAllowedResponse(Client* client, const std::vector<std::string>& allowedMethods);
    
    /**
     * @brief Invia risposta a richiesta OPTIONS
     * @param client Client destinatario
     * @param allowedMethods Lista dei metodi HTTP supportati
     */
    static void sendOptionsResponse(Client* client, const std::vector<std::string>& allowedMethods);
    
    /**
     * @brief Analizza dati form URL-encoded
     * @param body Contenuto del body HTTP
     * @param formData Mappa dove salvare i dati parsati
     */
    void parseFormUrlEncoded(const std::string& body, std::map<std::string, std::string>& formData);
};

// ==================== FUNZIONI HELPER GLOBALI ====================

/**
 * @brief Estrae la lunghezza del contenuto dagli header HTTP
 * @param headers Stringa contenente gli header HTTP
 * @return Lunghezza del contenuto in bytes
 */
size_t getContentLength(const std::string& headers);

#endif