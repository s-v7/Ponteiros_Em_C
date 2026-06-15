# grafos_municipios_br

Roteamento sobre o **grafo dos municípios do Brasil** (~5.570 nós) em C puro, do zero — projeto independente, não compartilha código com o repositório `Ponteiros_Em_C`.

Cada município é um nó (ponto representativo via PostGIS `ST_PointOnSurface`); as arestas são **fronteiras compartilhadas** (`ST_Intersects`). O peso de cada aresta é a distância **haversine**, calculada em C com a mesma função que gera a heurística do A\* — garantindo admissibilidade por construção.

## Estrutura

```
grafos_municipios_br/
├── pipeline/
│   ├── carrega_geobr.py     # geobr -> PostGIS (municipios_geo_br) + indice GiST
│   ├── nos_br.sql           # export nós   -> data/nos_br.csv
│   └── arestas_br.sql       # export arestas (ST_Intersects) -> data/arestas_br.csv
├── src/
│   ├── heap.h / heap.c      # min-heap binário (chave double, decrease-key)
│   ├── grafo.h / grafo.c    # loader CSV + lista de adjacência + haversine + lookups
│   └── astar_br.c           # ENTREGA 1: A* ponto-a-ponto
├── data/                    # nos_br.csv / arestas_br.csv (sample sintético incluído)
├── Makefile
└── README.md
```

## Pipeline de dados

```bash
pip install geobr geopandas sqlalchemy psycopg2-binary
python3 pipeline/carrega_geobr.py            # baixa todos os municípios -> PostGIS
psql -h localhost -U postgres -d postgres -f pipeline/nos_br.sql
psql -h localhost -U postgres -d postgres -f pipeline/arestas_br.sql
```

Fonte: pacote **geobr** (IPEA), dados oficiais do IBGE (Malha Municipal, escala 1:250.000, SIRGAS 2000 / CRS 4674).

## Build e uso

```bash
make
./astar_br 2211001 2201838            # por code_muni (Teresina -> Bom Jesus/PI)
./astar_br "Teresina/PI" "Bom Jesus/PI"   # por Nome/UF
```

Saída: rota mínima em km, caminho município a município, e a comparação de expansão de nós A\* × Dijkstra.

## Três correções de escala (vs. a versão Piauí)

1. **Sem teto fixo** — arrays do heap/grafo dimensionados em runtime (`malloc`), suportam os ~5.570 municípios sem `MAXN`.
2. **Chave canônica = `code_muni`** — nomes de município se repetem entre estados (ex.: "Bom Jesus" em PI e RS). A busca por nome só resolve se for único; senão, exige `Nome/UF` e lista os homônimos.
3. **Conectividade honesta** — o grafo de fronteira **não** é totalmente conexo: ilhas como **Fernando de Noronha** não têm fronteira terrestre e ficam isoladas; rotas até elas retornam "não existe caminho".

## Roadmap (motores que reaproveitam `grafo`/`heap`)

1. **A\* ponto-a-ponto** — entregue (`astar_br.c`).
2. Otimizador de rota nacional (TSP, nearest-neighbor + 2-opt).
3. Dijkstra 1→todos.
4. Floyd / all-pairs (com a ressalva de O(V³) e ~248 MB em escala nacional — provavelmente entregue como "Dijkstra de todas as fontes").

## Nota técnica

Os pesos são haversine entre sedes municipais (distância geográfica de saltos território-a-território), não quilometragem de estrada. Trocar a malha por OSM/DNIT transforma o peso em km de rodovia sem alterar os algoritmos.

## License

MIT — Silas Vasconcelos Cruz ([s-v7](https://github.com/s-v7))
