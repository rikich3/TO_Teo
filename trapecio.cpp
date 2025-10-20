#include <iostream>
#include <cmath>
#include <functional>
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

// Struct que representa una función con una variable
struct Funcion {
    std::function<double(double)> f;  // Función matemática f(x)
    std::string expresion;            // Representación en string de la función
    
    // Constructor
    Funcion(std::function<double(double)> func, std::string expr) 
        : f(func), expresion(expr) {}
    
    // Evaluar la función en un punto x
    double evaluar(double x) const {
        return f(x);
    }
    
    // Mostrar información de la función
    void mostrar() const {
        std::cout << "Función: " << expresion << std::endl;
    }
};

// Función para calcular el área de un trapecio
double areaTrapecio(double lado1, double lado2, double altura) {
  return ((lado1 + lado2) * altura) / 2.0;
}

// Función para generar puntos equidistantes entre x1 y x2
std::vector<double> generarPuntos(double x1, double x2, int c) {
  std::vector<double> puntos;
  puntos.reserve(c + 1);
  
  double paso = (x2 - x1) / c;
  
  for (int i = 0; i <= c; i++) {
    puntos.push_back(x1 + i * paso);
  }
  
  return puntos;
}

const int CANTIDAD_MAXIMA_DE_HILOS = 20;

// Función que será ejecutada por cada hilo
void calcularTrapecios(const Funcion& f, double x_inicio, double x_fin, double dx, double* resultado, std::mutex& mtx) {
    double area_parcial = 0.0;
    
    // Calcular el área del trapecio en este segmento
    double lado1 = f.evaluar(x_inicio);
    double lado2 = f.evaluar(x_fin);
    area_parcial = areaTrapecio(lado1, lado2, dx);
    
    // Usar mutex para evitar race conditions al sumar el resultado
    std::lock_guard<std::mutex> lock(mtx);
    *resultado += area_parcial;
}

int main() {
    Funcion f([](double x) { return 2*x*x + 3*x + 0.5; }, "f(x) = x²");  // Función cuadrática para mejor visualización
    
    std::cout << "=== Aproximación de Integral usando Regla del Trapecio con Hilos ===" << std::endl;
    
    double x1 = 2.0;  // Límite inferior
    double x2 = 20.0;  // Límite superior
    f.mostrar();
    std::cout << "Límites de integración: [" << x1 << ", " << x2 << "]" << std::endl;
    std::cout << "Número de hilos: " << CANTIDAD_MAXIMA_DE_HILOS << std::endl << std::endl;
    
    double dx = (x2 - x1) / (double)CANTIDAD_MAXIMA_DE_HILOS;
    
    double* resultado_integral = new double(0.0);  // Memoria dinámica para el resultado
    std::mutex mtx;  // Mutex para evitar race conditions
    std::vector<std::thread> hilos;  // Vector para almacenar los hilos
    hilos.reserve(CANTIDAD_MAXIMA_DE_HILOS);  // Reservar memoria para eficiencia
    
    auto puntos = generarPuntos(x1, x2, CANTIDAD_MAXIMA_DE_HILOS);
    
    // Tiempo de inicio
    auto inicio = std::chrono::high_resolution_clock::now();
    
    // CREACIÓN DE HILOS - vamos a dividir el área en trapecios
    std::cout << "Creando y ejecutando " << CANTIDAD_MAXIMA_DE_HILOS << " hilos..." << std::endl;
    for(int i = 0; i < CANTIDAD_MAXIMA_DE_HILOS; i++){
        double x_inicio = puntos[i];
        double x_fin = puntos[i+1];
        
        // Crear hilo que calculará el trapecio en este segmento
        hilos.emplace_back(calcularTrapecios, std::ref(f), x_inicio, x_fin, dx, resultado_integral, std::ref(mtx));
        
        std::cout << "Hilo " << i+1 << ": calculando trapecio en [" << x_inicio << ", " << x_fin << "]" << std::endl;
    }
    
    // EJECUCIÓN PARALELA - esperar a que todos los hilos terminen
    std::cout << "\nEsperando que todos los hilos terminen su ejecución..." << std::endl;
    for(auto& hilo : hilos) {
        hilo.join();  // Esperar a que cada hilo termine
    }
    
    auto fin = std::chrono::high_resolution_clock::now();
    auto duracion = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio);
    
    // Mostrar resultados
    std::cout << "\n=== RESULTADOS ===" << std::endl;
    std::cout << "Aproximación de la integral: " << *resultado_integral << std::endl;
    std::cout << "Tiempo de ejecución: " << duracion.count() << " microsegundos" << std::endl;
    
    // Para f(x) = x², la integral de 0 a 2 es: ∫x²dx = x³/3 = 8/3 ≈ 2.667
    double valor_exacto = (pow(x2, 3) - pow(x1, 3)) / 3.0;
    double error = abs(*resultado_integral - valor_exacto);
    std::cout << "Valor exacto: " << valor_exacto << std::endl;
    std::cout << "Error absoluto: " << error << std::endl;
    std::cout << "Error relativo: " << (error/valor_exacto)*100 << "%" << std::endl;
    
    // Liberar memoria dinámica
    delete resultado_integral;

    return 0;
}
