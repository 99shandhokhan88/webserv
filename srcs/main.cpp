/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:58:41 by vzashev           #+#    #+#             */
/*   Updated: 2025/05/07 17:42:54 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/**
 * @file main.cpp
 * @brief Punto di ingresso principale del webserver
 * 
 * Questo file contiene la funzione main che:
 * 1. Valida gli argomenti della riga di comando
 * 2. Carica il file di configurazione
 * 3. Inizializza e avvia il server
 * 4. Gestisce gli errori fatali
 */

#include "../incs/webserv.hpp"
#include "Config/incs/ServerConfig.hpp"
#include "Core/incs/Server.hpp"

/**
 * @brief Funzione principale del programma
 * @param argc Numero di argomenti della riga di comando
 * @param argv Array degli argomenti della riga di comando
 * @return 0 se successo, 1 se errore
 * 
 * Flusso di esecuzione:
 * 1. Controlla che sia stato fornito esattamente un file di configurazione
 * 2. Carica la configurazione dal file specificato
 * 3. Crea un'istanza del server con la configurazione caricata
 * 4. Avvia il server ed entra nel loop degli eventi
 */
int main(int argc, char **argv) {
    // Controllo argomenti: deve essere fornito esattamente un file di configurazione
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        std::cerr << "Esempio: ./webserv configs/default.conf" << std::endl;
        return (1);
    }
    else
    {
        try
        {
            // Fase 1: Caricamento della configurazione
            // Legge e valida il file di configurazione specificato
            ServerConfig config(argv[1]);

            // Fase 2: Inizializzazione del server
            // Crea il server con la configurazione caricata
            Server server(config);
            
            // Fase 3: Avvio del server
            // Entra nel loop principale degli eventi (blocking call)
            server.run();
        }
        catch (const std::exception& e)
        {
            // Gestione errori fatali: configurazione non valida, 
            // impossibilità di bind delle porte, etc.
            std::cerr << "Fatal error: " << e.what() << std::endl;
            return 1;
        }
    }

    return 0;  // Successo (questo punto non dovrebbe mai essere raggiunto)
}