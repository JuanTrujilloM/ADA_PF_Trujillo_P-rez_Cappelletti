#pragma once
#include "parser.hpp"
#include <string>
#include <vector>

using namespace std;

struct ItemBW {
    string customerID;      // id del cliente
    int tenure;             // antiguedad del cliente
    double monthlyCharges;  // cargo mensual original
    double totalCharges;    // cargo total original
    int peso;               // peso usado en la mochila
    int valor;              // valor usado en la mochila
};

struct ResultadoMochila {
    int valorOptimo;              // mejor valor que encontro la DP
    vector<ItemBW> seleccionados; // items que quedaron elegidos
    vector<vector<int>> dp;       // tabla completa para poder volver atras
};

struct ContraejemploCodicioso {
    bool encontrado;                       // true si hay fallo del codicioso
    long long triosEvaluados;              // cuantos trios se probaron
    vector<ItemBW> trio;                   // trio donde falla, si existe
    vector<ItemBW> seleccionCodiciosa;     // lo que toma el codicioso
    int valorCodicioso;                    // valor del codicioso
    vector<ItemBW> seleccionOptima;        // lo que toma la DP
    int valorOptimo;                       // valor de la DP
    vector<ItemBW> mejorTrio;              // mejor trio visto aunque no falle
    vector<ItemBW> mejorSeleccionCodiciosa;// codicioso en el mejor intento
    int mejorValorCodicioso;               // valor codicioso del mejor intento
    vector<ItemBW> mejorSeleccionOptima;   // DP en el mejor intento
    int mejorValorOptimo;                  // valor DP del mejor intento
    int mejorDiferencia;                   // diferencia DP - codicioso
};

vector<ItemBW> construirItemsActivos(const vector<Solicitud>& solicitudesOrdenadas,
                                     int maxItems = 50);

ResultadoMochila resolverMochila01(const vector<ItemBW>& items, int capacidad);

ContraejemploCodicioso buscarContraejemploCodicioso(const vector<ItemBW>& items,
                                                    int capacidad);

void generarReporteAsignacionBW(const vector<Solicitud>& solicitudesOrdenadas,
                                 const string& outputPath = "results/asignacion_bw.txt",
                                 int capacidad = 500);
