import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.*;

interface Funcion {
  double evaluar(double x);
}


class TareaTrapecio implements Callable<Double> {

  private double x1, x2, base;
  private Funcion funcion;

  public TareaTrapecio(double x1, double x2, double base, Funcion funcion) {
    this.x1 = x1;
    this.x2 = x2;
    this.base = base;
    this.funcion = funcion;
  }

  @Override
  public Double call() {
    try {
      if (funcion == null) throw new NullPointerException("La función no puede ser nula");
      if (base <= 0) throw new IllegalArgumentException("La base del trapecio debe ser positiva");

      double y1 = funcion.evaluar(x1);
      double y2 = funcion.evaluar(x2);

      if (Double.isNaN(y1) || Double.isNaN(y2)) throw new ArithmeticException(
        "Evaluación de función inválida (NaN)"
      );

      return (base / 2.0) * (y1 + y2);
    } catch (NullPointerException | IllegalArgumentException | ArithmeticException e) {
      System.err.printf("Error en tarea [x1=%.4f, x2=%.4f]: %s%n", x1, x2, e.getMessage());
      return Double.NaN;
    } catch (Exception e) {
      System.err.printf(
        "Error inesperado en tarea [x1=%.4f, x2=%.4f]: %s%n",
        x1,
        x2,
        e.getMessage()
      );
      return Double.NaN;
    }
  }
}


public class TrapecioPool {

  public static void main(String[] args) {
    double limiteInferior = 2,
      limiteSuperior = 20;
    double areaAnterior = -1;
    boolean detener = false;
    int numTrapecios = 1;

    int numNucleos = Runtime.getRuntime().availableProcessors();
        // Se reemplaza new FuncionEjemplo() por la expresión lambda.
    Funcion funcion = x -> 2 * x * x + 3 * x + 0.5;

    System.out.printf("Usando %d núcleos disponibles.%n", numNucleos);

    while (!detener) {
      double base = (limiteSuperior - limiteInferior) / numTrapecios;

      ExecutorService ejecutor = Executors.newFixedThreadPool(numNucleos);
      List<Future<Double>> resultados = new ArrayList<>();

      try {
        for (int i = 0; i < numTrapecios; i++) {
          double xi = limiteInferior + i * base;
          double xf = xi + base;
          resultados.add(ejecutor.submit(new TareaTrapecio(xi, xf, base, funcion)));
        }

        double area = 0.0;
        for (Future<Double> futuro : resultados) {
          try {
            double valor = futuro.get();
            if (!Double.isNaN(valor)) {
              area += valor;
            }
          } catch (ExecutionException e) {
            System.err.println("Error en ejecución de una tarea: " + e.getCause());
          } catch (InterruptedException e) {
            System.err.println("Ejecución interrumpida: " + e.getMessage());
            Thread.currentThread().interrupt();
          }
        }

        ejecutor.shutdown();

        System.out.printf("N = %d -> Área aproximada = %.6f%n", numTrapecios, area);

        double areaRedondeada = Math.round(area * 10000.0) / 10000.0;
        double areaAnteriorRedondeada = Math.round(areaAnterior * 10000.0) / 10000.0;

        if (areaRedondeada == areaAnteriorRedondeada) {
          detener = true;
          System.out.println("\nSe detuvo porque el área ya no cambia en 4 decimales.");
        }

        areaAnterior = area;
        numTrapecios++;
      } catch (IllegalArgumentException e) {
        System.err.println("Error de argumento en el cálculo principal: " + e.getMessage());
        detener = true;
      } catch (Exception e) {
        System.err.println("Error inesperado en el cálculo principal: " + e.getMessage());
        detener = true;
      } finally {
        if (!ejecutor.isShutdown()) {
          ejecutor.shutdownNow();
        }
      }
    }
  }
}