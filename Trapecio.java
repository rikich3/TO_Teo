interface Funcion {
  double evaluar(double x);
}

class FuncionEjemplo implements Funcion {

  public double evaluar(double x) {
    // Catch errors derivate from arithmetic operations
    try {
      return 2 * x * x + 3 * x + 0.5;
    } catch (ArithmeticException e) {
      System.err.println("Error aritmético: " + e.getMessage());
    } catch (IllegalArgumentException e) {
      System.err.println("Argumento inválido: " + e.getMessage());
    } catch (Exception e) {
      System.err.println("Error inesperado en evaluar(): " + e.getMessage());
    }
    return Double.NaN;
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

  @Override
  public void run() {
    try {
      // Catch unallowed threads amount
      if (n <= 0) throw new IllegalArgumentException(
        "El número de subdivisiones debe ser positivo"
      );
      // Catch null function
      if (f == null) throw new NullPointerException("La función no puede ser nula");

      double h = (b - a) / n;
      double suma = f.evaluar(a) + f.evaluar(b);
      for (int i = 1; i < n; i++) {
        double xi = a + i * h;
        suma += 2 * f.evaluar(xi);
      }
      resultado = (h / 2) * suma;
    } catch (NullPointerException | IllegalArgumentException e) {
      System.err.println("Error en el hilo: " + e.getMessage());
      resultado = Double.NaN;
    } catch (Exception e) {
      System.err.println("Error inesperado en el hilo: " + e.getMessage());
      resultado = Double.NaN;
    }
  }

  public double getResultado() {
    return resultado;
  }
}

public class Trapecio {

  public static void main(String[] args) {
    double a = 2,
      b = 20;
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
      // Catch interrupted exception from join threads
    } catch (InterruptedException e) {
      e.printStackTrace();
    } catch (Exception e) {
      System.err.println("Error inesperado durante la agregación de resultados: " + e.getMessage());
    }

    System.out.printf("Area aproximada con %d hilos: %.4f%n", numHilos, areaTotal);
  }
}
