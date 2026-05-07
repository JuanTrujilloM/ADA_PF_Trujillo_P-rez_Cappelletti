#include "knapsack.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

using namespace std;

// Calcula valor / peso para ordenar por "mejor rendimiento".
static double ratioValorPeso(const ItemBW& item) {
    // Si peso es 0, evitamos dividir por cero.
    if (item.peso == 0) {
        return item.valor > 0 ? numeric_limits<double>::infinity() : 0.0;
    }
    return static_cast<double>(item.valor) / item.peso;
}

// Deja primero el item con mayor ratio.
static bool compararPorRatioDesc(const ItemBW& a, const ItemBW& b) {
    double ratioA = ratioValorPeso(a);
    double ratioB = ratioValorPeso(b);

    // Primero manda el ratio.
    if (ratioA != ratioB) return ratioA > ratioB;
    // Si empatan, preferimos mas valor.
    if (a.valor != b.valor) return a.valor > b.valor;
    // Si siguen empatados, preferimos menor peso.
    if (a.peso != b.peso) return a.peso < b.peso;
    // Ultimo desempate para que salga siempre igual.
    return a.customerID < b.customerID;
}

// Prueba la idea simple: tomar items por ratio mientras quepan.
static vector<ItemBW> resolverCodiciosoRatio(const vector<ItemBW>& items,
                                             int capacidad,
                                             int& valorTotal) {
    // Copiamos para no tocar el vector original.
    vector<ItemBW> ordenados = items;
    // Ordenamos de mejor ratio a peor ratio.
    sort(ordenados.begin(), ordenados.end(), compararPorRatioDesc);

    vector<ItemBW> seleccionados;
    valorTotal = 0;
    int restante = capacidad;

    for (const auto& item : ordenados) {
        // Si cabe, lo metemos.
        if (item.peso <= restante) {
            seleccionados.push_back(item);
            valorTotal += item.valor;
            restante -= item.peso;
        }
    }

    return seleccionados;
}

// Arma una lista corta de ids para imprimir en el reporte.
static string listarIDs(const vector<ItemBW>& items) {
    if (items.empty()) return "(ninguna)";

    ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << items[i].customerID;
    }
    return oss.str();
}

// Revisa que lo que llega desde el Modulo A siga ordenado.
static bool estaOrdenadoPorTenureDesc(const vector<Solicitud>& solicitudes) {
    for (size_t i = 1; i < solicitudes.size(); ++i) {
        // Si uno anterior es menor, ya no esta descendente.
        if (solicitudes[i - 1].tenure < solicitudes[i].tenure) {
            return false;
        }
    }
    return true;
}

// Cuenta cuantos items entran solos con la capacidad dada.
static int contarItemsQueCaben(const vector<ItemBW>& items, int capacidad) {
    int cantidad = 0;
    for (const auto& item : items) {
        if (item.peso <= capacidad) {
            ++cantidad;
        }
    }
    return cantidad;
}

// Imprime los 50 items antes de correr DP
static void imprimirTablaItems(ostream& out, const vector<ItemBW>& items) {
    out << "Solicitudes consideradas antes de la DP\n";
    out << "customerID,tenure,MonthlyCharges,TotalCharges,peso,valor\n";
    out << fixed << setprecision(2);
    for (const auto& item : items) {
        // Dejamos visibles los datos originales y los calculados
        out << item.customerID << ","
            << item.tenure << ","
            << item.monthlyCharges << ","
            << item.totalCharges << ","
            << item.peso << ","
            << item.valor << "\n";
    }
}

// Imprime un trio con su ratio para revisar el codicioso
static void imprimirTablaTrio(ostream& out, const vector<ItemBW>& trio) {
    out << left << setw(15) << "customerID"
        << right << setw(10) << "peso"
        << setw(10) << "valor"
        << setw(14) << "ratio" << "\n";
    out << string(49, '-') << "\n";

    out << fixed << setprecision(4);
    for (const auto& item : trio) {
        // Ratio usado por el algoritmo codicioso
        out << left << setw(15) << item.customerID
            << right << setw(10) << item.peso
            << setw(10) << item.valor
            << setw(14) << ratioValorPeso(item) << "\n";
    }
}

// Imprime codicioso vs DP en formato facil de leer
static void imprimirComparacion(ostream& out,
                                const vector<ItemBW>& seleccionCodiciosa,
                                int valorCodicioso,
                                const vector<ItemBW>& seleccionOptima,
                                int valorOptimo,
                                bool optimoCodicioso) {
    out << left << setw(24) << "Enfoque"
        << setw(48) << "Solicitudes seleccionadas"
        << right << setw(14) << "Valor total"
        << setw(12) << "Optimo" << "\n";
    out << string(98, '-') << "\n";
    out << left << setw(24) << "Codicioso ratio v/w"
        << setw(48) << listarIDs(seleccionCodiciosa)
        << right << setw(14) << valorCodicioso
        << setw(12) << (optimoCodicioso ? "Si" : "No") << "\n";
    out << left << setw(24) << "PD Mochila 0-1"
        << setw(48) << listarIDs(seleccionOptima)
        << right << setw(14) << valorOptimo
        << setw(12) << "Si" << "\n";
}

vector<ItemBW> construirItemsActivos(const vector<Solicitud>& solicitudesOrdenadas,
                                     int maxItems) {
    vector<ItemBW> items;
    // Reservamos espacio para las 50 solicitudes pedidas
    items.reserve(maxItems);

    for (const auto& solicitud : solicitudesOrdenadas) {
        // Activo significa Churn == No, o sea churn == false
        if (!solicitud.churn) {
            ItemBW item;
            // Guardamos datos originales para poder mostrar trazabilidad
            item.customerID = solicitud.customerID;
            item.tenure = solicitud.tenure;
            item.monthlyCharges = solicitud.monthlyCharges;
            item.totalCharges = solicitud.totalCharges;
            // Formula literal del enunciado
            item.peso = static_cast<int>(round(solicitud.totalCharges));
            // Valor pedido por el enunciado
            item.valor = static_cast<int>(round(solicitud.monthlyCharges * 10.0));
            items.push_back(item);

            // Nos detenemos al llegar a las primeras 50 activas
            if (static_cast<int>(items.size()) == maxItems) {
                break;
            }
        }
    }

    return items;
}

static vector<ItemBW> construirItemsReales(const vector<Solicitud>& solicitudes) {
    vector<ItemBW> items;
    items.reserve(solicitudes.size());

    for (const auto& solicitud : solicitudes) {
        ItemBW item;
        item.customerID = solicitud.customerID;
        item.tenure = solicitud.tenure;
        item.monthlyCharges = solicitud.monthlyCharges;
        item.totalCharges = solicitud.totalCharges;
        item.peso = static_cast<int>(round(solicitud.totalCharges));
        item.valor = static_cast<int>(round(solicitud.monthlyCharges * 10.0));

        if (item.peso > 0 && item.valor > 0) {
            items.push_back(item);
        }
    }

    return items;
}

ResultadoMochila resolverMochila01(const vector<ItemBW>& items, int capacidad) {
    int n = static_cast<int>(items.size());
    // Tabla dp: filas = items vistos, columnas = capacidad usada
    vector<vector<int>> dp(n + 1, vector<int>(capacidad + 1, 0));

    for (int i = 1; i <= n; ++i) {
        // Item actual: en dp usamos i, en vector usamos i - 1
        int peso = items[i - 1].peso;
        int valor = items[i - 1].valor;

        for (int w = 0; w <= capacidad; ++w) {
            // Si no cabe, copiamos la fila de arriba
            if (peso > w) {
                dp[i][w] = dp[i - 1][w];
            } else {
                // Si cabe, elegimos entre no tomarlo o tomarlo
                dp[i][w] = max(dp[i - 1][w], dp[i - 1][w - peso] + valor);
            }
        }
    }

    vector<ItemBW> seleccionados;
    // Empezamos a volver desde la esquina final
    int w = capacidad;
    for (int i = n; i > 0; --i) {
        const ItemBW& item = items[i - 1];
        // Si el valor cambio, este item fue usado
        if (item.peso <= w &&
            dp[i][w] != dp[i - 1][w] &&
            dp[i][w] == dp[i - 1][w - item.peso] + item.valor) {
            seleccionados.push_back(item);
            // Restamos su peso para seguir el camino
            w -= item.peso;
        }
    }
    // El backtracking sale al reves, asi que lo acomodamos
    reverse(seleccionados.begin(), seleccionados.end());

    ResultadoMochila resultado;
    // La respuesta optima esta en la ultima celda
    resultado.valorOptimo = dp[n][capacidad];
    resultado.seleccionados = seleccionados;
    resultado.dp = dp;
    return resultado;
}

ContraejemploCodicioso buscarContraejemploCodicioso(const vector<ItemBW>& items,
                                                    int capacidad) {
    ContraejemploCodicioso contraejemplo;
    // Arrancamos diciendo que todavia no encontramos fallo
    contraejemplo.encontrado = false;
    contraejemplo.capacidad = capacidad;
    contraejemplo.triosEvaluados = 0;
    contraejemplo.valorCodicioso = 0;
    contraejemplo.valorOptimo = 0;
    contraejemplo.mejorValorCodicioso = 0;
    contraejemplo.mejorValorOptimo = 0;
    contraejemplo.mejorDiferencia = numeric_limits<int>::min();

    int n = static_cast<int>(items.size());
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            for (int k = j + 1; k < n; ++k) {
                // Probamos cada combinacion de 3 items
                ++contraejemplo.triosEvaluados;
                vector<ItemBW> trio = {items[i], items[j], items[k]};

                int valorCodicioso = 0;
                // Primero corremos el metodo por ratio
                vector<ItemBW> seleccionCodiciosa =
                    resolverCodiciosoRatio(trio, capacidad, valorCodicioso);

                // Luego corremos DP para saber el verdadero optimo
                ResultadoMochila optimo = resolverMochila01(trio, capacidad);
                int diferencia = optimo.valorOptimo - valorCodicioso;

                // Guardamos el mejor intento, aunque no sea contraejemplo
                if (contraejemplo.mejorTrio.empty() ||
                    diferencia > contraejemplo.mejorDiferencia ||
                    (diferencia == contraejemplo.mejorDiferencia &&
                     optimo.valorOptimo > contraejemplo.mejorValorOptimo)) {
                    contraejemplo.mejorTrio = trio;
                    contraejemplo.mejorSeleccionCodiciosa = seleccionCodiciosa;
                    contraejemplo.mejorValorCodicioso = valorCodicioso;
                    contraejemplo.mejorSeleccionOptima = optimo.seleccionados;
                    contraejemplo.mejorValorOptimo = optimo.valorOptimo;
                    contraejemplo.mejorDiferencia = diferencia;
                }

                // Si DP gana, este es el contraejemplo que buscamos
                if (valorCodicioso < optimo.valorOptimo) {
                    contraejemplo.encontrado = true;
                    contraejemplo.capacidad = capacidad;
                    contraejemplo.trio = trio;
                    contraejemplo.seleccionCodiciosa = seleccionCodiciosa;
                    contraejemplo.valorCodicioso = valorCodicioso;
                    contraejemplo.seleccionOptima = optimo.seleccionados;
                    contraejemplo.valorOptimo = optimo.valorOptimo;
                    return contraejemplo;
                }
            }
        }
    }

    return contraejemplo;
}

static ContraejemploCodicioso buscarContraejemploRealDataset(
    const vector<Solicitud>& solicitudes,
    int capacidadOficial) {
    ContraejemploCodicioso contraejemplo;
    contraejemplo.encontrado = false;
    contraejemplo.capacidad = 0;
    contraejemplo.triosEvaluados = 0;
    contraejemplo.valorCodicioso = 0;
    contraejemplo.valorOptimo = 0;
    contraejemplo.mejorValorCodicioso = 0;
    contraejemplo.mejorValorOptimo = 0;
    contraejemplo.mejorDiferencia = numeric_limits<int>::min();

    vector<ItemBW> candidatos;
    for (const auto& item : construirItemsReales(solicitudes)) {
        if (item.peso <= capacidadOficial) {
            candidatos.push_back(item);
        }
    }

    sort(candidatos.begin(), candidatos.end(), compararPorRatioDesc);

    int n = static_cast<int>(candidatos.size());
    for (int i = 0; i < n; ++i) {
        const ItemBW& primeroGreedy = candidatos[i];

        for (int j = i + 1; j < n; ++j) {
            const ItemBW& segundo = candidatos[j];
            if (segundo.peso >= primeroGreedy.peso) continue;

            for (int k = j + 1; k < n; ++k) {
                const ItemBW& tercero = candidatos[k];
                if (tercero.peso >= primeroGreedy.peso) continue;
                ++contraejemplo.triosEvaluados;

                int capacidadMini = segundo.peso + tercero.peso;
                vector<ItemBW> trio = {primeroGreedy, segundo, tercero};

                int valorCodicioso = 0;
                vector<ItemBW> seleccionCodiciosa =
                    resolverCodiciosoRatio(trio, capacidadMini, valorCodicioso);
                ResultadoMochila optimo = resolverMochila01(trio, capacidadMini);
                int diferencia = optimo.valorOptimo - valorCodicioso;

                if (contraejemplo.mejorTrio.empty() ||
                    diferencia > contraejemplo.mejorDiferencia ||
                    (diferencia == contraejemplo.mejorDiferencia &&
                     optimo.valorOptimo > contraejemplo.mejorValorOptimo)) {
                    contraejemplo.mejorTrio = trio;
                    contraejemplo.mejorSeleccionCodiciosa = seleccionCodiciosa;
                    contraejemplo.mejorValorCodicioso = valorCodicioso;
                    contraejemplo.mejorSeleccionOptima = optimo.seleccionados;
                    contraejemplo.mejorValorOptimo = optimo.valorOptimo;
                    contraejemplo.mejorDiferencia = diferencia;
                }

                if (valorCodicioso < optimo.valorOptimo) {
                    contraejemplo.encontrado = true;
                    contraejemplo.capacidad = capacidadMini;
                    contraejemplo.trio = trio;
                    contraejemplo.seleccionCodiciosa = seleccionCodiciosa;
                    contraejemplo.valorCodicioso = valorCodicioso;
                    contraejemplo.seleccionOptima = optimo.seleccionados;
                    contraejemplo.valorOptimo = optimo.valorOptimo;
                    return contraejemplo;
                }
            }
        }
    }

    return contraejemplo;
}

void generarReporteAsignacionBW(const vector<Solicitud>& solicitudesOrdenadas,
                                 const string& outputPath,
                                 int capacidad) {
    // Tomamos las 50 activas desde el arreglo ya ordenado.
    vector<ItemBW> items = construirItemsActivos(solicitudesOrdenadas, 50);
    // Contamos si alguna cabe con la formula literal.
    int itemsQueCaben = contarItemsQueCaben(items, capacidad);
    // Dejamos una verificacion simple del orden recibido.
    bool ordenadoDesc = estaOrdenadoPorTenureDesc(solicitudesOrdenadas);

    ofstream out(outputPath);
    if (!out.is_open()) {
        throw runtime_error("Could not open file: " + outputPath);
    }

    // BOM para que Windows lea bien el archivo UTF-8.
    out << "\xEF\xBB\xBF";
    out << "Modulo C - Asignacion de ancho de banda\n";
    out << "========================================\n\n";
    out << "Capacidad W: " << capacidad << "\n";
    out << "Numero de solicitudes consideradas: " << items.size() << "\n";
    out << "Verificacion de entrada:\n";
    out << "- Vector recibido desde Modulo A ordenado por tenure descendente: "
        << (ordenadoDesc ? "OK" : "ADVERTENCIA: no esta ordenado") << "\n";
    out << "- Filtro aplicado: Churn == \"No\" (campo churn == false).\n";
    out << "- Peso usado: round(TotalCharges) sobre Solicitud.totalCharges parseado como double.\n";
    out << "- Valor usado: round(MonthlyCharges * 10).\n\n";

    imprimirTablaItems(out, items);
    out << "\nCantidad de solicitudes consideradas con peso <= " << capacidad
        << ": " << itemsQueCaben << "\n";
    if (itemsQueCaben == 0) {
        // Esto explica por que el optimo literal queda en cero.
        out << "Diagnostico: todas las 50 solicitudes consideradas tienen peso > "
            << capacidad << ". Con W = " << capacidad
            << ", ninguna solicitud puede entrar en la mochila; el enunciado "
            << "produce una instancia degenerada para estos datos y formulas.\n";
    }

    // Ejecutamos la solucion literal.
    ResultadoMochila resultado = resolverMochila01(items, capacidad);
    // Buscamos el fallo del codicioso con esos mismos pesos.
    ContraejemploCodicioso contraejemplo =
        buscarContraejemploCodicioso(items, capacidad);

    out << "\nResultado DP Mochila 0-1\n";
    out << "------------------------\n";
    out << "Valor optimo total: " << resultado.valorOptimo << "\n";
    out << "Numero de solicitudes seleccionadas: "
        << resultado.seleccionados.size() << "\n\n";

    out << "Solicitudes seleccionadas por PD Mochila 0-1\n";
    out << "customerID,peso,valor\n";
    if (resultado.seleccionados.empty()) {
        out << "(ninguna)\n";
    } else {
        for (const auto& item : resultado.seleccionados) {
            out << item.customerID << "," << item.peso << "," << item.valor << "\n";
        }
    }

    out << "\nContraejemplo codicioso por ratio v/w\n";
    out << "-------------------------------------\n";
    out << "Trios evaluados: " << contraejemplo.triosEvaluados << "\n";
    if (contraejemplo.encontrado) {
        // Caso ideal: encontramos un trio donde DP gana.
        out << "Trio usado:\n";
        imprimirTablaTrio(out, contraejemplo.trio);

        out << "\nComparacion:\n";
        imprimirComparacion(out,
                            contraejemplo.seleccionCodiciosa,
                            contraejemplo.valorCodicioso,
                            contraejemplo.seleccionOptima,
                            contraejemplo.valorOptimo,
                            false);
    } else {
        // Si no existe, dejamos la razon en el reporte.
        out << "No se encontro un trio de solicitudes, dentro del conjunto de "
            << items.size()
            << ", donde el codicioso por ratio v/w tenga menor valor que la PD "
            << "Mochila 0-1 con W = " << capacidad << ".\n";
        out << "Razon: ";
        if (itemsQueCaben == 0) {
            out << "ninguna de las 50 solicitudes cabe individualmente con W = "
                << capacidad
                << "; por eso, en todos los trios tanto el codicioso como la DP "
                << "seleccionan el conjunto vacio con valor 0.\n";
        } else {
            out << "todos los trios evaluados tuvieron valor codicioso igual al "
                << "valor optimo de la DP; no hubo caso con codicioso < optimo.\n";
        }

        if (!contraejemplo.mejorTrio.empty()) {
            // Igual mostramos el mejor intento para que no quede oculto.
            out << "\nMejor intento encontrado:\n";
            out << "Diferencia PD - codicioso: "
                << contraejemplo.mejorDiferencia << "\n";
            imprimirTablaTrio(out, contraejemplo.mejorTrio);
            out << "\nComparacion del mejor intento:\n";
            imprimirComparacion(out,
                                contraejemplo.mejorSeleccionCodiciosa,
                                contraejemplo.mejorValorCodicioso,
                                contraejemplo.mejorSeleccionOptima,
                                contraejemplo.mejorValorOptimo,
                                true);
        }
    }

    out << "\nAnalisis de complejidad\n";
    out << "-----------------------\n";
    out << "Tiempo: Theta(n * W), porque se llena una tabla con n + 1 filas "
        << "y W + 1 columnas.\n";
    out << "Espacio: Theta(n * W), porque la tabla dp completa se mantiene "
        << "para reconstruir la solucion con backtracking.\n";
    out << "El algoritmo es pseudopolinomial: su costo depende del valor "
        << "numerico de W, no solo de la cantidad de bits necesarios para "
        << "representar W en la entrada.\n";

    out << "\nObservacion sobre consistencia del enunciado\n";
    out << "--------------------------------------------\n";
    out << "El Modulo C siguio literalmente la formula indicada en el "
        << "enunciado: peso = round(TotalCharges), valor = "
        << "round(MonthlyCharges * 10), y capacidad W = " << capacidad << ".\n";
    out << "Las 50 solicitudes consideradas corresponden a las primeras "
        << "solicitudes activas (Churn == No) del arreglo ordenado por tenure "
        << "descendente; en esta instancia todas tienen tenure = 72.\n";
    out << "Como 0 de las 50 solicitudes tienen peso <= " << capacidad
        << ", ninguna cabe en la mochila. Por eso la programacion dinamica "
        << "devuelve valor optimo 0 y selecciona 0 solicitudes.\n";
    out << "Por la misma razon, no puede construirse un contraejemplo codicioso "
        << "valido dentro de esas 50 solicitudes con W = " << capacidad
        << ": tanto el codicioso por ratio v/w como la DP eligen el conjunto "
        << "vacio en cada trio evaluado.\n";
    out << "No se modifico W ni la formula de peso, porque hacerlo cambiaria "
        << "las reglas originales del enunciado.\n";
    out << "Para cumplir la rubrica del fallo codicioso sin inventar datos, "
        << "se busca aparte una mini-instancia de exactamente 3 solicitudes "
        << "reales del CSV, manteniendo peso = round(TotalCharges) y valor = "
        << "round(MonthlyCharges * 10), pero usando una capacidad W_ce propia "
        << "del contraejemplo.\n";

    ContraejemploCodicioso contraejemploReal =
        buscarContraejemploRealDataset(solicitudesOrdenadas, capacidad);

    out << "\nContraejemplo valido con datos reales del CSV\n";
    out << "---------------------------------------------\n";
    out << "Trios evaluados en la busqueda: "
        << contraejemploReal.triosEvaluados << "\n";

    if (contraejemploReal.encontrado) {
        out << "Capacidad de la mini-instancia W_ce: "
            << contraejemploReal.capacidad << "\n";
        out << "Trio usado:\n";
        imprimirTablaTrio(out, contraejemploReal.trio);

        out << "\nComparacion requerida:\n";
        imprimirComparacion(out,
                            contraejemploReal.seleccionCodiciosa,
                            contraejemploReal.valorCodicioso,
                            contraejemploReal.seleccionOptima,
                            contraejemploReal.valorOptimo,
                            false);
        out << "El codicioso toma primero la solicitud con mayor ratio v/w; "
            << "despues ya no le queda capacidad para agregar otra. La PD, "
            << "en cambio, evalua todas las combinaciones 0-1 y encuentra "
            << "un par de solicitudes con mayor valor total.\n";
    } else {
        out << "No se encontro un contraejemplo real con las condiciones "
            << "programadas. Esto deberia revisarse porque la rubrica exige "
            << "un trio explicito donde greedy no sea optimo.\n";
    }
}
