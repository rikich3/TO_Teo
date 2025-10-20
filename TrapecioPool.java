import java.util.concurrent.*;
import java.util.*;

interface Funcion {
    double evaluar(double x);
}

class FuncionEjemplo implements Funcion {
    public double evaluar(double x) {
        return 2 * x * x + 3 * x + 0.5;
    }
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
        return (base / 2.0) * (funcion.evaluar(x1) + funcion.evaluar(x2));
    }
}

public class TrapecioPool {
    public static void main(String[] args) throws Exception {
        double limiteInferior = 2, limiteSuperior = 20;
        double areaAnterior = -1;
        boolean detener = false;
        int numTrapecios = 1;

        int numNucleos = Runtime.getRuntime().availableProcessors();
        Funcion funcion = new FuncionEjemplo();

        while (!detener) {
            double base = (limiteSuperior - limiteInferior) / numTrapecios;

            //Creamos un pool con los nucleos que tiene el host
            ExecutorService ejecutor = Executors.newFixedThreadPool(numNucleos);
            List<Future<Double>> resultados = new ArrayList<>();

            for (int i = 0; i < numTrapecios; i++) {
                double xi = limiteInferior + i * base;
                double xf = xi + base;
                resultados.add(ejecutor.submit(new TareaTrapecio(xi, xf, base, funcion)));
            }

            double area = 0;
            for (Future<Double> futuro : resultados) {
                area += futuro.get();
            }

            ejecutor.shutdown();

            System.out.printf("N = %d -> rea aproximada = %.6f%n", numTrapecios, area);

            double areaRedondeada = Math.round(area * 10000.0) / 10000.0;
            double areaAnteriorRedondeada = Math.round(areaAnterior * 10000.0) / 10000.0;

            if (areaRedondeada == areaAnteriorRedondeada) {
                detener = true;
                System.out.println("\nSe detuvo porque el rea ya no cambia en 4 decimales.");
            }

            areaAnterior = area;
            numTrapecios++;
        }
    }
}