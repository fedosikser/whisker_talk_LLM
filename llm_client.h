#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <vector>

struct ChatMessage {
    std::string role;
    std::string content;
};

class LLMClient {
private:
    int sock;
    std::string host;
    int port;
    std::string endpoint;
    
    std::string formatMessagesJSON(const std::vector<ChatMessage>& messages) {
        std::ostringstream json;
        for (size_t i = 0; i < messages.size(); ++i) {
            if (i > 0) {
                json << ",";
            }
            json << "{\"role\": \"" << escapeJSON(messages[i].role) << "\", \"content\": \""
                 << escapeJSON(messages[i].content) << "\"}";
        }
        return json.str();
    }

    // Формируем JSON-запрос для Gemma
    std::string formatGemmaRequest(const std::vector<ChatMessage>& messages) {
        std::ostringstream json;
        json << "{";
        json << "\"model\": \"google/gemma-3-4b\",";
        json << "\"messages\": [" << formatMessagesJSON(messages) << "],";
        json << "\"max_tokens\": 500,";
        json << "\"temperature\": 0.7";
        json << "}";
        return json.str();
    }
    
    // Экранируем специальные символы для JSON
    std::string escapeJSON(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c; break;
            }
        }
        return result;
    }
    
    // Формируем HTTP запрос
    std::string formatHTTPRequest(const std::string& body) {
        std::ostringstream request;
        request << "POST " << endpoint << " HTTP/1.1\r\n";
        request << "Host: " << host << ":" << port << "\r\n";
        request << "Content-Type: application/json\r\n";
        request << "Accept: application/json\r\n";
        request << "Content-Length: " << body.length() << "\r\n";
        request << "Connection: close\r\n";
        request << "\r\n";
        request << body;
        return request.str();
    }
    
    // Извлекаем текст ответа из JSON
    std::string extractResponseFromJSON(const std::string& json_response) {
        // Ищем в choices[0].message.content
        size_t pos = json_response.find("\"choices\"");
        if (pos != std::string::npos) {
            pos = json_response.find("\"message\"", pos);
            if (pos != std::string::npos) {
                pos = json_response.find("\"content\"", pos);
                if (pos != std::string::npos) {
                    pos = json_response.find(":", pos);
                    if (pos != std::string::npos) {
                        pos = json_response.find("\"", pos);
                        if (pos != std::string::npos) {
                            size_t start = pos + 1;
                            size_t end = json_response.find("\"", start);
                            if (end != std::string::npos) {
                                std::string content = json_response.substr(start, end - start);
                                return unescapeJSON(content);
                            }
                        }
                    }
                }
            }
        }
        
        // Ищем в response (альтернативный формат)
        pos = json_response.find("\"response\"");
        if (pos != std::string::npos) {
            pos = json_response.find(":", pos);
            if (pos != std::string::npos) {
                pos = json_response.find("\"", pos);
                if (pos != std::string::npos) {
                    size_t start = pos + 1;
                    size_t end = json_response.find("\"", start);
                    if (end != std::string::npos) {
                        return unescapeJSON(json_response.substr(start, end - start));
                    }
                }
            }
        }
        
        // Если ничего не нашли, возвращаем ошибку
        return "Ошибка: не удалось распарсить ответ";
    }
    
    // Декодируем escape-последовательности JSON
    std::string unescapeJSON(const std::string& s) {
        std::string result;
        for (size_t i = 0; i < s.length(); i++) {
            if (s[i] == '\\' && i + 1 < s.length()) {
                switch (s[i + 1]) {
                    case '"': result += '"'; i++; break;
                    case '\\': result += '\\'; i++; break;
                    case 'n': result += '\n'; i++; break;
                    case 'r': result += '\r'; i++; break;
                    case 't': result += '\t'; i++; break;
                    default: result += s[i]; break;
                }
            } else {
                result += s[i];
            }
        }
        return result;
    }
    
    // Извлекаем тело ответа из HTTP
    std::string extractHTTPBody(const std::string& response) {
        size_t pos = response.find("\r\n\r\n");
        if (pos != std::string::npos) {
            return response.substr(pos + 4);
        }
        return response;
    }

    bool hasHTTPSuccessStatus(const std::string& response) {
        size_t line_end = response.find("\r\n");
        std::string status_line = line_end == std::string::npos
            ? response
            : response.substr(0, line_end);
        return status_line.find("200") != std::string::npos;
    }
    
public:
    LLMClient(std::string host = "127.0.0.1", int port = 1234, std::string endpoint = "/v1/chat/completions") 
        : sock(-1), host(host), port(port), endpoint(endpoint) {}
    
    ~LLMClient() {
        disconnect();
    }
    
    bool connect() {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cerr << "[LLMClient] Ошибка создания сокета" << std::endl;
            return false;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        
        struct hostent *server = gethostbyname(host.c_str());
        if (server == NULL) {
            std::cerr << "[LLMClient] Ошибка: хост " << host << " не найден" << std::endl;
            return false;
        }
        
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
        
        if (::connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "[LLMClient] Ошибка подключения к " << host << ":" << port << std::endl;
            return false;
        }
        
        return true;
    }
    
    std::string sendChat(const std::vector<ChatMessage>& messages) {
        if (sock >= 0) {
            disconnect();
        }

        if (!connect()) {
            return "Ошибка: не удалось подключиться к LLM серверу";
        }

        std::string jsonBody = formatGemmaRequest(messages);
        std::string httpRequest = formatHTTPRequest(jsonBody);
        
        // Отправляем запрос
        ssize_t sent = send(sock, httpRequest.c_str(), httpRequest.length(), 0);
        if (sent < 0) {
            disconnect();
            return "Ошибка отправки запроса";
        }
        
        // Получаем ответ
        char buffer[65536] = {0};
        std::string full_response;
        int bytes_received;
        
        while ((bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
            full_response.append(buffer, bytes_received);
            memset(buffer, 0, sizeof(buffer));
        }
        
        if (bytes_received < 0) {
            disconnect();
            return "Ошибка получения ответа";
        }
        
        // Извлекаем тело HTTP и парсим JSON
        std::string httpBody = extractHTTPBody(full_response);

        disconnect();

        if (!hasHTTPSuccessStatus(full_response)) {
            return "Ошибка HTTP от сервера: " + httpBody;
        }
        
        // Для отладки: выводим сырой ответ (закомментировано)
        // std::cerr << "Raw response: " << httpBody << std::endl;
        
        return extractResponseFromJSON(httpBody);
    }

    std::string sendRequest(const std::string& text) {
        return sendChat({{"user", text}});
    }
    
    void disconnect() {
        if (sock != -1) {
            close(sock);
            sock = -1;
        }
    }
    
    bool isConnected() const {
        return sock != -1;
    }
};
