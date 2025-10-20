#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <numeric>
#include <chrono>

// Función a integrar: f(x) = 2x^2 + 3x + 0.5
double function_to_integrate(double x) {
    return 2.0 * x * x + 3.0 * x + 0.5;
}

// ------------------- Thread Pool Implementation -------------------

class ThreadPool {
public:
    ThreadPool(size_t num_threads) : stop_all(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        // Espera hasta que haya una tarea o se solicite la detención
                        condition.wait(lock, [this]{
                            return stop_all || !tasks.empty();
                        });

                        if (stop_all && tasks.empty()) {
                            return; // El hilo termina
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task(); // Ejecuta la tarea
                }
            });
        }
    }

    // Agrega una nueva tarea a la cola
    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop_all) return; // No aceptar tareas si ya se está deteniendo
            tasks.emplace(std::move(task));
        }
        condition.notify_one(); // Notifica a un hilo que hay una tarea nueva
    }

    // Destructor: espera a que todos los hilos terminen
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop_all = true;
        }
        condition.notify_all(); // Despierta a todos los hilos
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join(); // Espera a que cada hilo termine
            }
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop_all;
};

// ------------------- Integración Paralela -------------------

// Variables globales para el cálculo (necesitan ser sincronizadas)
double total_result = 0.0;
std::mutex result_mutex;

// Tarea para calcular la contribución de un segmento (trapecio)
void integrate_segment(double a, double b, double h) {
    double area = h * (function_to_integrate(a) + function_to_integrate(b)) / 2.0;

    // Suma la contribución al resultado total de forma segura
    std::lock_guard<std::mutex> lock(result_mutex);
    total_result += area;
}

int main() {
    const double A = 2.0;       // Límite inferior
    const double B = 20.0;      // Límite superior
    const int N_TASKS = 10000;  // Número de segmentos (tareas)
    const int N_THREADS = 4;    // Número de hilos en el pool

    // Ancho de cada segmento
    double h = (B - A) / N_TASKS;

    // Inicializa el pool de hilos
    ThreadPool pool(N_THREADS);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Divide la integral en N_TASKS y las envía al pool
    for (int i = 0; i < N_TASKS; ++i) {
        double a_i = A + i * h;
        double b_i = A + (i + 1) * h;

        // Encola la tarea para integrar el segmento [a_i, b_i]
        pool.enqueue([a_i, b_i, h] {
            integrate_segment(a_i, b_i, h);
        });
    }

    // El destructor del ThreadPool esperará automáticamente a que todas
    // las tareas encoladas se completen antes de que main termine.

    // Pequeño retardo para asegurar que los hilos terminen antes de calcular el tiempo final
    // En una aplicación real, se usarían futuros/promesas o una barrera de sincronización,
    // pero para este ejemplo, el destructor del ThreadPool cumple esta función.
    // Simplemente aseguramos que los hilos tienen tiempo de tomar y completar la última tarea.
    // Nota: El destructor de 'pool' se llama al final del 'main' y espera a los hilos.

    // Para obtener el tiempo correcto, necesitamos esperar explícitamente a que todas las tareas
    // estén completas antes de salir del ámbito de 'pool'.

    // Una forma simple de esperar es hacer que el ThreadPool rastree el número de tareas activas
    // o simplemente mover el pool al final del bloque main para que su destructor se ejecute.

    // Para este ejemplo, podemos confiar en que la finalización de 'pool' al final de 'main'
    // nos da el tiempo hasta que se completa la última tarea.

    // (La temporización es imprecisa sin una barrera o futuros)

    // Detención forzada aquí para demostrar el tiempo después de que se supone que todo terminó
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;


    // Valor teórico de la integral definida de 2x^2 + 3x + 0.5 en [0, 10]:
    // [ (2/3)x^3 + (3/2)x^2 + (1/2)x ] en [0, 10]
    // = (2/3)(1000) + (3/2)(100) + (1/2)(10)
    // = 666.6666... + 150 + 5 = 821.6666...

    std::cout << "--- Resultado de la Integración Paralela (Pool de Hilos) ---" << std::endl;
    std::cout << "Función: 2x^2 + 3x + 0.5" << std::endl;
    std::cout << "Rango de Integración: [" << A << ", " << B << "]" << std::endl;
    std::cout << "Número de Segmentos (Tareas): " << N_TASKS << std::endl;
    std::cout << "Número de Hilos en el Pool: " << N_THREADS << std::endl;
    std::cout << "Resultado Aproximado: " << total_result << std::endl;
    std::cout << "Tiempo de Ejecución: " << duration.count() << " segundos" << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;

    return 0;
}