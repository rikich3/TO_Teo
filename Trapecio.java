interface Funcion {
    double evaluar(double x);
}

class FuncionEjemplo implements Funcion {
    public double evaluar(double x) {
        return 2 * x * x + 3 * x + 0.5;
    }
}

class TrapecioHilo extends Thread {
    private double a, b;
    private int n;
    private Funcion f;
    private double resultado;

    public TrapecioHilo(double a, double b, int n, Funcion f) {
        this.a = a;
        this.b = b;
        this.n = n;
        this.f = f;
    }

    public void run() {
        double h = (b - a) / n;
        double suma = f.evaluar(a) + f.evaluar(b);
        for (int i = 1; i < n; i++) {
            double xi = a + i * h;
            suma += 2 * f.evaluar(xi);
        }
        resultado = (h / 2) * suma;
    }

    public double getResultado() {
        return resultado;
    }
}

public class Trapecio {
    public static void main(String[] args) {
        double a = 2, b = 20;
        int subdivisiones = 10000;
        int numHilos = 4;

        Funcion f = new FuncionEjemplo();
        TrapecioHilo[] hilos = new TrapecioHilo[numHilos];

        double intervalo = (b - a) / numHilos;

        for (int i = 0; i < numHilos; i++) {
            double ai = a + i * intervalo;
            double bi = (i == numHilos - 1) ? b : ai + intervalo;
            int ni = subdivisiones / numHilos;
            hilos[i] = new TrapecioHilo(ai, bi, ni, f);
            hilos[i].start();
        }

        double areaTotal = 0.0;
        try {
            for (TrapecioHilo hilo : hilos) {
                hilo.join();
                areaTotal += hilo.getResultado();
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        System.out.printf("Area aproximada con %d hilos: %.4f%n", numHilos, areaTotal);

    }
}
