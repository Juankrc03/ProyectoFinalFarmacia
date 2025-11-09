#include <iostream>
#include <fstream>
#include <limits>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "json.hpp"   // Asegúrate: Propiedades C/C++ → General → $(ProjectDir)include

using nlohmann::json;

constexpr int REABASTECER_A = 40; // ← meta fija: si queda en 0, vuelve a 40

// ================= Utilidades =================
int readInt(const std::string& msg) {
    int v;
    while (true) {
        std::cout << msg;
        if (std::cin >> v) return v;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Valor inválido.\n";
    }
}
std::string readStr(const std::string& msg) {
    std::cout << msg;
    std::string s; std::getline(std::cin >> std::ws, s);
    return s;
}
