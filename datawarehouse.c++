#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>

using namespace std;

// ==========================================
// 1. FONTES DE DADOS BRUTOS (Sistemas Origem)
// ==========================================
struct VendaBruta {
    string idTransacao;
    string nomeProduto;
    string categoriaProduto;
    int quantidade;
    double precoUnitario;
    string dataTexto; // Ex: "2026-06-19"
};

// ==========================================
// 2. ESTRUTURA DO DATA WAREHOUSE (Star Schema)
// ==========================================

// Dimensão Produto
struct DimProduto {
    int skProduto; // Surrogate Key (Chave Substituta)
    string nome;
    string categoria;
};

// Dimensão Tempo
struct DimTempo {
    int skTempo;
    string dataCompleta;
    int ano;
    int mes;
    int dia;
};

// Tabela de Fatos (Fato Vendas)
struct FatoVendas {
    int skProduto;    // Chave estrangeira para DimProduto
    int skTempo;      // Chave estrangeira para DimTempo
    int quantidadeVendida;
    double valorTotal; // Métrica/Fato
};

// O Data Warehouse propriamente dito
class DataWarehouse {
private:
    map<int, DimProduto> tabelaDimProduto;
    map<int, DimTempo> tabelaDimTempo;
    vector<FatoVendas> tabelaFatoVendas;

    int geradorSkProduto = 1;
    int geradorSkTempo = 1;

    // Funções auxiliares para evitar duplicidade nas dimensões (Lookup)
    int obterOuInserirProduto(const string& nome, const string& categoria) {
        for (const auto& [sk, prod] : tabelaDimProduto) {
            if (prod.nome == nome) return sk;
        }
        int novaSk = geradorSkProduto++;
        tabelaDimProduto[novaSk] = {novaSk, nome, categoria};
        return novaSk;
    }

    int obterOuInserirTempo(const string& data) {
        for (const auto& [sk, tempo] : tabelaDimTempo) {
            if (tempo.dataCompleta == data) return sk;
        }
        // Transformação simples de string para inteiros (Simulando ETL)
        int ano = stoi(data.substr(0, 4));
        int mes = stoi(data.substr(5, 2));
        int dia = stoi(data.substr(8, 2));

        int novaSk = geradorSkTempo++;
        tabelaDimTempo[novaSk] = {novaSk, data, ano, mes, dia};
        return novaSk;
    }

public:
    // ==========================================
    // 3. PROCESSO ETL (Extract, Transform, Load)
    // ==========================================
    void executarETL(const vector<VendaBruta>& dadosOrigem) {
        cout << "[ETL] Iniciando processo de Extração, Transformação e Carga...\n";
        
        for (const auto& venda : dadosOrigem) {
            // 1. TRANSFORMAÇÃO & LOOKUP: Tratamento de dados e geração de chaves substitutas
            int skProd = obterOuInserirProduto(venda.nomeProduto, venda.categoriaProduto);
            int skTemp = obterOuInserirTempo(venda.dataTexto);

            // Cálculo da métrica derivada
            double valorTotalCalculado = venda.quantidade * venda.precoUnitario;

            // 2. CARGA: Inserção na Tabela de Fatos
            FatoVendas fato {
                skProd,
                skTemp,
                venda.quantidade,
                valorTotalCalculado
            };
            tabelaFatoVendas.push_back(fato);
        }
        cout << "[ETL] Processo concluído com sucesso! Dados carregados no DW.\n\n";
    }

    // ==========================================
    // 4. CONSULTAS OLAP (Business Intelligence)
    // ==========================================
    void relatorioVendasPorCategoria() {
        cout << "=== RELATÓRIO OLAP: VENDAS POR CATEGORIA DE PRODUTO ===\n";
        map<string, double> faturamentoPorCategoria;

        // Query simulada cruzando Fato com Dimensão
        for (const auto& fato : tabelaFatoVendas) {
            string categoria = tabelaDimProduto[fato.skProduto].categoria;
            faturamentoPorCategoria[categoria] += fato.valorTotal;
        }

        cout << left << setw(20) << "Categoria" << " | " << "Faturamento Total" << "\n";
        cout << "-------------------------------------------\n";
        for (const auto& [categoria, total] : faturamentoPorCategoria) {
            cout << left << setw(20) << categoria << " | R$ " << fixed << setprecision(2) << total << "\n";
        }
        cout << "\n";
    }

    void relatorioVendasMensais() {
        cout << "=== RELATÓRIO OLAP: EVOLUÇÃO MENSAL DE VENDAS ===\n";
        map<string, double> faturamentoMensal;

        for (const auto& fato : tabelaFatoVendas) {
            DimTempo tempo = tabelaDimTempo[fato.skTempo];
            string chaveMes = to_string(tempo.ano) + "/" + (tempo.mes < 10 ? "0" : "") + to_string(tempo.mes);
            faturamentoMensal[chaveMes] += fato.valorTotal;
        }

        cout << left << setw(10) << "Mês/Ano" << " | " << "Total Vendido" << "\n";
        cout << "-----------------------------------\n";
        for (const auto& [mes, total] : faturamentoMensal) {
            cout << left << setw(10) << mes << " | R$ " << fixed << setprecision(2) << total << "\n";
        }
        cout << "\n";
    }
};

// ==========================================
// EXECUÇÃO DO SISTEMA
// ==========================================
int main() {
    // Simulando os dados brutos que viriam de um sistema de caixa/E-commerce (OLTP)
    vector<VendaBruta> sistemaOLTP = {
        {"TX001", "Notebook", "Eletrônicos", 1, 4500.00, "2026-05-10"},
        {"TX002", "Mouse Gamer", "Eletrônicos", 2, 150.00, "2026-05-12"},
        {"TX003", "Cadeira Ergonômica", "Móveis", 1, 1200.00, "2026-05-15"},
        {"TX004", "Notebook", "Eletrônicos", 1, 4500.00, "2026-06-01"},
        {"TX005", "Mesa de Escritório", "Móveis", 1, 800.00, "2026-06-18"}
    };

    // Criando a instância do Data Warehouse
    DataWarehouse dw;

    // Executando o pipeline de dados
    dw.executarETL(sistemaOLTP);

    // Gerando os relatórios de tomada de decisão (BI)
    dw.relatorioVendasPorCategoria();
    dw.relatorioVendasMensais();

    return 0;
}