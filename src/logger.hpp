#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>

namespace {
    template <typename T>
    std::string format_vec(const std::vector<T>& vec) {
        std::ostringstream oss;
        oss << "(";
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i + 1 < vec.size()) oss << ", ";
        }
        oss << ")";
        return oss.str();
    }
}

namespace logger {
    // --- DEBUG ---
    #ifndef NDEBUG
    inline void debug(const std::string& msg) {
        std::cout << "[DEBUG] " << msg << '\n';
    }

    template <typename T, typename... Args>
    inline void debug(const std::string& msg, const T& value, const Args&... args) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos) {
            std::ostringstream oss;
            if (std::is_floating_point<T>::value) {
                oss << std::scientific << value;
            } else {
                oss << value;
            }
            std::string formatted = msg;
            formatted.replace(pos, 2, oss.str());
            debug(formatted, args...);
        } else {
            std::cout << "[DEBUG] " << msg << '\n';
        }
    }

    template <typename T>
    inline void debug(const std::string& msg, const std::vector<T>& vec) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos && !vec.empty()) {
            std::string formatted = msg;
            formatted.replace(pos, 2, format_vec(vec));
            std::cout << "[DEBUG] " << formatted << '\n';
        } else {
            std::cout << "[DEBUG] " << msg << '\n';
        }
    }
    #else
    inline void debug(const std::string&) {}
    template <typename T, typename... Args>
    inline void debug(const std::string&, const T&, const Args&...) {}
    template <typename T>
    inline void debug(const std::string&, const std::vector<T>&) {}
    #endif

    //  --- INFO ---
    inline void info(const std::string& msg) {
        std::cout << "[INFO] " << msg << '\n';
    }

    template <typename T, typename... Args>
    inline void info(const std::string& msg, const T& value, const Args&... args) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos) {
            std::ostringstream oss;
            if (std::is_floating_point<T>::value) {
                oss << std::scientific << value;
            } else {
                oss << value;
            }
            std::string formatted = msg;
            formatted.replace(pos, 2, oss.str());
            info(formatted, args...);
        } else {
            std::cout << "[INFO] " << msg << '\n';
        }
    }

    template <typename T>
    inline void info(const std::string& msg, const std::vector<T>& vec) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos && !vec.empty()) {
            std::string formatted = msg;
            formatted.replace(pos, 2, format_vec(vec));
            std::cout << "[INFO] " << formatted << '\n';
        } else {
            std::cout << "[INFO] " << msg << '\n';
        }
    }

    // --- WARNING ---
    inline void warning(const std::string& msg) {
        std::cout << "[WARNING] " << msg << '\n';
    }

    template <typename T, typename... Args>
    inline void warning(const std::string& msg, const T& value, const Args&... args) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos) {
            std::ostringstream oss;
            if (std::is_floating_point<T>::value) {
                oss << std::scientific << value;
            } else {
                oss << value;
            }
            std::string formatted = msg;
            formatted.replace(pos, 2, oss.str());
            warning(formatted, args...);
        } else {
            std::cout << "[WARNING] " << msg << '\n';
        }
    }

    template <typename T>
    inline void warning(const std::string& msg, const std::vector<T>& vec) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos && !vec.empty()) {
            std::string formatted = msg;
            formatted.replace(pos, 2, format_vec(vec));
            std::cout << "[WARNING] " << formatted << '\n';
        } else {
            std::cout << "[WARNING] " << msg << '\n';
        }
    }

    // --- ERROR ---
    inline void error(const std::string& msg) {
        std::cout << "[ERROR] " << msg << '\n';
    }

    template <typename T, typename... Args>
    inline void error(const std::string& msg, const T& value, const Args&... args) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos) {
            std::ostringstream oss;
            if (std::is_floating_point<T>::value) {
                oss << std::scientific << value;
            } else {
                oss << value;
            }
            std::string formatted = msg;
            formatted.replace(pos, 2, oss.str());
            error(formatted, args...);
        } else {
            std::cout << "[ERROR] " << msg << '\n';
        }
    }

    template <typename T>
    inline void error(const std::string& msg, const std::vector<T>& vec) {
        size_t pos = msg.find("{}");
        if (pos != std::string::npos && !vec.empty()) {
            std::string formatted = msg;
            formatted.replace(pos, 2, format_vec(vec));
            std::cout << "[ERROR] " << formatted << '\n';
        } else {
            std::cout << "[ERROR] " << msg << '\n';
        }
    }
}