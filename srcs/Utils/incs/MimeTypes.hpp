/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MimeTypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzashev <vzashev@student.42roma.it>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 19:55:53 by vzashev           #+#    #+#             */
/*   Updated: 2025/02/18 19:56:25 by vzashev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <string>
#include <map>


class MimeTypes
{

public:

    // Get the MIME type for a file extension
    static std::string getMimeType(const std::string& extension);

private:

    // Map of file extensions to MIME types
    static std::map<std::string, std::string> mimeTypes;

    // Initialize the MIME types map
    static void initializeMimeTypes();
    
};

#endif // MIMETYPES_HPP
