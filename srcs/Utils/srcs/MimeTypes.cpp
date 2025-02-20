/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypes.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 21:50:09 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 19:57:41 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#include "MimeTypes.hpp"


// Map of file extensions to MIME types
std::map<std::string, std::string> MimeTypes::mimeTypes;


// Get the MIME type for a file extension
std::string MimeTypes::getMimeType(const std::string& extension)
{
    // Initialize the MIME types map
    if (mimeTypes.empty())
    {
        initializeMimeTypes();  // Initialize the MIME types
    }
    
    // Find the MIME type for the extension
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    
    // Return the MIME type if found
    if (it != mimeTypes.end())
    {
        return  (it->second);  // Return the MIME type
    }

    // Return the default MIME type if not found
    return  ("application/octet-stream"); // Default MIME type
    
}


// Initialize the MIME types map
void MimeTypes::initializeMimeTypes()
{
    mimeTypes[".html"] = "text/html";
    mimeTypes[".htm"] = "text/html";
    mimeTypes[".css"] = "text/css";
    mimeTypes[".js"] = "application/javascript";
    mimeTypes[".json"] = "application/json";
    mimeTypes[".jpg"] = "image/jpeg";
    mimeTypes[".jpeg"] = "image/jpeg";
    mimeTypes[".png"] = "image/png";
    mimeTypes[".gif"] = "image/gif";
    mimeTypes[".txt"] = "text/plain";
    mimeTypes[".pdf"] = "application/pdf";
    mimeTypes[".zip"] = "application/zip";
    mimeTypes[".xml"] = "application/xml";
    mimeTypes[".mp3"] = "audio/mpeg";
    mimeTypes[".mp4"] = "video/mp4";
}
