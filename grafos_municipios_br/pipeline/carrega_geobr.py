"""
carrega_geobr.py --- baixa todos os municípios do Brasil via geobr e grava no 
PostGIS (tabela municípios_geo_br), criando o indice espacial GisT.

Dependências: pip install geobr geopandas sqlalchemy psycopg2-binary
Uso:          python3 carregar_geo_br.py
Ajuste a string de conexão e o 'ANO' conforme suas necessedades...
"""
import geobr
from sqlalchemy import create_engine, text

ANOS_VALIDOS = {
    1872, 1900, 1911, 1920, 1933, 1940, 1950, 1960, 1970, 1980,
    1991, 2000, 2001, 2005, 2007, 2010, 2013, 2014, 2015, 2016,
    2017, 2018, 2019, 2020, 2021, 2022, 2023, 2024
}

DB_GEO_BR = "postgresql+psycopg2://postgres:silasvc07@localhost:5432/postgres"
TAB_GEO_BR = "municipios_geo_br"


def pedir_ano():
    valor = input("Ano que deseja? [2024]: ").strip()

    if valor == "":
        return 2024

    ano = int(valor)

    if ano not in ANOS_VALIDOS:
        print(f"Ano {ano} não disponível no geobr instalado. Usando 2024.")
        return 2024

    return ano


def main():
    ano = pedir_ano()

    print(f"Baixando municípios do Brasil ({ano}) via geobr...")
    mun = geobr.read_municipality(
        code_muni="all",
        year=ano,
        simplified=False
    )

    print(f"{len(mun)} municípios | CRS {mun.crs}")

    eng = create_engine(DB_GEO_BR)

    print(f"Gravando em {TAB_GEO_BR}...")

    mun.to_postgis(
        name=TAB_GEO_BR,
        con=eng,
        if_exists="replace",
        index=False
    )

    with eng.begin() as con:
        con.execute(text("CREATE EXTENSION IF NOT EXISTS postgis;"))

        con.execute(text(
            f"CREATE INDEX IF NOT EXISTS idx_{TAB_GEO_BR}_geom "
            f"ON {TAB_GEO_BR} USING gist (geometry);"
        ))

        con.execute(text(f"ANALYZE {TAB_GEO_BR};"))

    print("OK: tabela carregada e índice GiST criado.")
    print("Próximo: psql -f pipeline/nos_br.sql && psql -f pipeline/arestas_br.sql")


if __name__ == "__main__":
    main()
