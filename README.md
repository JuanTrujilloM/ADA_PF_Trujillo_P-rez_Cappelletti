# ADA_PF_Trujillo_Pérez_Cappelletti

Proyecto final de Análisis y Diseño de Algoritmos 2. Aplica algoritmos de ordenamiento, búsqueda, grafos y programación dinámica sobre el dataset de churn de clientes de telecomunicaciones (Telco Customer Churn).

---

## Algoritmos implementados

| Módulo | Algoritmo | Propósito |
|--------|-----------|-----------|
| A | MergeSort | Ordenar registros por `tenure` |
| A | Búsqueda binaria | Consultar clientes por `tenure` |
| B | Kruskal (MST) | Red mínima de conectividad entre grupos |
| C | Knapsack 0-1 (DP) | Asignación óptima de ancho de banda |

---

## Requisitos

- Compilador `g++` con soporte para C++17
- Dataset: `data/WA_Fn-UseC_-Telco-Customer-Churn.csv` (incluido en el repositorio)

### Verificar versión de g++

```bash
g++ --version
```
---

## Compilación

Desde la raíz del proyecto:

```bash
g++ -std=c++17 -O2 -o ada_pf src/*.cpp
```

Esto compila todos los archivos fuente en `src/` y genera el ejecutable `ada_pf` en el directorio actual.

---

## Ejecución

```bash
./ada_pf data/WA_Fn-UseC_-Telco-Customer-Churn.csv
```

El programa recibe como único argumento la ruta al dataset CSV.

### Salida esperada en consola

```
MODULE A
Records loaded: 7043
Records with null TotalCharges: 11
File generated: results/solicitudes_ordenadas.csv

Binary searches:
Q_A01 (k=72): <customerID>
Q_A02 (k=60): <customerID>
Q_A03 (k=45): <customerID>
Q_A04 (k=30): <customerID>
Q_A05 (k=12): <customerID>
File generated: results/busquedas_A.txt

MergeSort timing:
n           time (ms)      
---------------------------
1000        ...            
3500        ...            
7043        ...            

MODULE B
Graph: 20 nodes, <n> edges
MST total weight: <peso>
File generated: results/mst_red.txt

MODULE C
File generated: results/asignacion_bw.txt
```

---

## Archivos generados

Todos los resultados se escriben en la carpeta `results/`:

| Archivo | Contenido |
|---------|-----------|
| `results/solicitudes_ordenadas.csv` | Registros ordenados por `tenure` (MergeSort) |
| `results/busquedas_A.txt` | Resultados de las 5 búsquedas binarias |
| `results/mst_red.txt` | Árbol de expansión mínima (Kruskal) con pesos y grupos |
| `results/asignacion_bw.txt` | Asignación óptima de ancho de banda (Knapsack DP) y contraejemplo greedy |

---

## Estructura del proyecto

```
.
├── data/
│   └── WA_Fn-UseC_-Telco-Customer-Churn.csv   # Dataset de entrada
├── results/                                     # Resultados generados
│   ├── solicitudes_ordenadas.csv
│   ├── busquedas_A.txt
│   ├── mst_red.txt
│   └── asignacion_bw.txt
├── src/
│   ├── main.cpp          # Punto de entrada
│   ├── parser.cpp/.hpp   # Lectura del CSV
│   ├── mergesort.cpp/.hpp
│   ├── binary_search.cpp/.hpp
│   ├── graph.cpp/.hpp    # Construcción del grafo
│   ├── kruskal.cpp/.hpp  # MST con Union-Find
│   └── knapsack.cpp/.hpp # Knapsack 0-1 DP
├── report/
│   └── Informe Proyecto Final.pdf
└── README.md
```