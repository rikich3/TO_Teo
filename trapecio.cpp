#include <iostream>
#include <thread>
#include <vector>
#include <cmath>

using namespace std;

inline double calcularAreaTrapecio(const double h, const double y1, const double y2) {
    return (h / 2) * (y1 + y2);
}

class TrapecioHilo {
private:
    const double a, b;
    const int n;
    double resultado;
    double (*funcion)(double);

public:
    TrapecioHilo(const double a, const double b, const int n, double (*f)(double))
        : a(a), b(b), n(n), funcion(f), resultado(0.0) {}

    void calcular() {
        const double h = (b - a) / n;
        double suma = funcion(a) + funcion(b);
        for (int i = 1; i < n; i++) {
            const double xi = a + i * h;
            suma += 2 * funcion(xi);
        }
        resultado = (h / 2) * suma;
    }

    double getResultado() const {
        return resultado;
    }
};

double funcionEjemplo(double x) {
    return 2 * x * x + 3 * x + 0.5;
}

inline double evaluarFuncion(double (*f)(double), const double x) {
    return f(x);
}

int main() {
    const double a = 2.0, b = 20.0;
    const int subdivisiones = 10000;
    const int numHilos = 4;

    vector<TrapecioHilo*> hilos;
    vector<thread> threads;

    const double intervalo = (b - a) / numHilos;

    for (int i = 0; i < numHilos; i++) {
        const double ai = a + i * intervalo;
        const double bi = (i == numHilos - 1) ? b : ai + intervalo;
        const int ni = subdivisiones / numHilos;
        
        TrapecioHilo* th = new TrapecioHilo(ai, bi, ni, funcionEjemplo);
        hilos.push_back(th);
        threads.push_back(thread(&TrapecioHilo::calcular, th));
    }

    for (auto& t : threads) {
        t.join();
    }

    double areaTotal = 0.0;
    for (const auto& hilo : hilos) {
        areaTotal += hilo->getResultado();
    }

    cout << "Area aproximada con " << numHilos << " hilos: " 
         << fixed << areaTotal << endl;

    for (auto& hilo : hilos) {
        delete hilo;
    }

    return 0;
}
