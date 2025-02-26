#include <iostream>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <string>
#include <iomanip>

using namespace std;

struct Conexao {
    string proximo;
    int duracao;
};

unordered_map<string, vector<Conexao>> mapa_espacial;

void carregar_dados(const string &nome_arquivo)
{
    ifstream entrada(nome_arquivo);

    if (!entrada.is_open())
    {
        cerr << "Erro: nao foi possivel abrir o arquivo: " << nome_arquivo << endl;
        return;
    }

    string linha;

    cout << "\nCarregando informacoes sobre conexoes orbitais...\n";

    auto inicio = chrono::high_resolution_clock::now();

    while (getline(entrada, linha))
    {
        stringstream dados(linha);
        string origem, destino, duracao_str;

        getline(dados, origem, ',');

        while (dados.good())
        {
            getline(dados, destino, ',');
            if (destino.empty()) continue;

            getline(dados, duracao_str, ',');
            if (duracao_str.empty()) continue;

            int duracao = stoi(duracao_str);
            mapa_espacial[origem].push_back({destino, duracao});
        }
    }

    entrada.close();

    auto fim = chrono::high_resolution_clock::now();
    cout << "Dados carregados em " << chrono::duration<double>(fim - inicio).count() << " segundos.\n";
}

void buscar_rotas(const string &atual, const string &alvo,
                  vector<string> &caminho_temporario, vector<int> &custos_temporarios,
                  vector<vector<string>> &todos_caminhos, vector<vector<int>> &todos_custos,
                  set<string> &visitados, int limite_profundidade = 10)
{
    if (caminho_temporario.size() > limite_profundidade) return;

    caminho_temporario.push_back(atual);

    if (atual == alvo)
    {
        todos_caminhos.push_back(caminho_temporario);
        todos_custos.push_back(custos_temporarios);
    }
    else
    {
        visitados.insert(atual);

        for (const auto &rota : mapa_espacial[atual])
            {
            if (visitados.find(rota.proximo) == visitados.end())
            {
                custos_temporarios.push_back(rota.duracao);
                buscar_rotas(rota.proximo, alvo, caminho_temporario, custos_temporarios,
                             todos_caminhos, todos_custos, visitados, limite_profundidade);
                custos_temporarios.pop_back();
            }
        }

        visitados.erase(atual);
    }

    caminho_temporario.pop_back();
}

void identificar_redundancias(const string &inicio, const string &fim)
{
    vector<vector<string>> todos_caminhos;
    vector<vector<int>> todos_custos;
    vector<string> caminho_atual;
    vector<int> custo_atual;
    set<string> ja_visitados;

    buscar_rotas(inicio, fim, caminho_atual, custo_atual, todos_caminhos, todos_custos, ja_visitados);

    unordered_map<string, int> redundancias;

    for (const auto &caminho : todos_caminhos)
    {
        for (const auto &estacao : caminho)
        {
            redundancias[estacao]++;
        }
    }

    cout << "\n----- REDUNDANCIAS NA REDE -----\n\n";

    for (const auto &par : redundancias)
    {
        if (par.second > 1) { // Estacoes que podem ser alcancadas por multiplas rotas
            cout << "Estacao (" << par.first << ") pode ser alcancada por (" << par.second << ") rotas diferentes.\n";
        }
    }
    cout << "\n----------------------------------------------------------------------\n";
}

void exibir_rotas(const string &inicio, const string &fim)
{
    vector<vector<string>> todos_caminhos;
    vector<vector<int>> todos_custos;
    vector<string> caminho_atual;
    vector<int> custo_atual;
    set<string> ja_visitados;

    buscar_rotas(inicio, fim, caminho_atual, custo_atual, todos_caminhos, todos_custos, ja_visitados);

    cout << "\nRotas encontradas entre " << inicio << " e " << fim << ":\n\n";

    vector<pair<int, int>> rotas_ordenadas;
    for (size_t i = 0; i < todos_caminhos.size(); ++i)
    {
        int custo_total = 0;
        for (int custo : todos_custos[i]) custo_total += custo;
        rotas_ordenadas.push_back({custo_total, i});
    }

    sort(rotas_ordenadas.begin(), rotas_ordenadas.end());

    for (size_t idx = 0; idx < rotas_ordenadas.size(); ++idx)
    {
        int i = rotas_ordenadas[idx].second;
        int custo_total = rotas_ordenadas[idx].first;

        cout << "--- ROTA (" << (idx + 1) << "):\n";
        for (size_t j = 0; j < todos_caminhos[i].size(); ++j)
        {
            cout << todos_caminhos[i][j];
            if (j < todos_caminhos[i].size() - 1)
            {
                cout << " -> (" << todos_custos[i][j] << ") ";
            }
        }
        cout << "\nCusto total: " << custo_total << "\n\n";
    }

    if (!rotas_ordenadas.empty())
    {
        int melhor_indice = rotas_ordenadas[0].second;
        int melhor_custo = rotas_ordenadas[0].first;

        cout << "\n----- MELHOR ROTA ENCONTRADA / CAMINHO MAIS CURTO -----\n\n";
        cout << "--- ROTA (1):\n";

        for (size_t j = 0; j < todos_caminhos[melhor_indice].size(); ++j)
        {
            cout << todos_caminhos[melhor_indice][j];
            if (j < todos_caminhos[melhor_indice].size() - 1)
            {
                cout << " -> (" << todos_custos[melhor_indice][j] << ") ";
            }
        }

        cout << "\n- CUSTO TOTAL DA ROTA: [*" << melhor_custo << "*]\n";
        cout << "\n----------------------------------------------------------------------\n";
    }
    else
    {
        cout << "Nenhuma rota encontrada entre " << inicio << " e " << fim << ".\n";
    }

    identificar_redundancias(inicio, fim);
}

int main()
{
    string arquivo = "estacoeserotas.csv";

    carregar_dados(arquivo);

    cout << "\nIniciando busca entre Terra e Centauri...\n";
    auto inicio_busca = chrono::high_resolution_clock::now();
    exibir_rotas("Terra", "Centauri");
    auto fim_busca = chrono::high_resolution_clock::now();

    cout << "\nTempo total da operacao: " << chrono::duration<double>(fim_busca - inicio_busca).count() << " segundos.\n";

    return 0;
}
